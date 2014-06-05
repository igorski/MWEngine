/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2014 Igor Zinken - http://www.igorski.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/*
    Native Audio Engine
    Android audio engine using OpenSL for hardware output
*/
#include <string.h>
#include <assert.h>
#include <math.h>
#include <vector>
#include "native_audio_engine.h"
#include "global.h"
#include "audiochannel.h"
#include "baseaudioevent.h"
#include "diskwriter.h"
#include "processingchain.h"
#include "sequencer.h"
#include "java_bridge.h"
#include "opensl_io.h"

static int thread;

/* initializers */

jint JNI_OnLoad( JavaVM* vm, void* reserved )
{
    JNIEnv* env;

    if ( vm->GetEnv(( void** ) &env, JNI_VERSION_1_6 ) != JNI_OK )
        return -1;

    registerVM( vm );

    DebugTool::log( "JNI INITED OK" );
    return JNI_VERSION_1_6;
}

/* public methods */

/**
 * register the calling Object and its environment
 * in the Java bridge (there should only be one calling Java class)
 */
extern "C"
void init( JNIEnv* env, jobject jobj )
{
    registerInterface( env, jobj );
}

/**
 * start the render thread
 * NOTE: the render thread is always active, even when the
 * sequencer is paused
 */
extern "C"
void start( JNIEnv* env, jobject jobj )
{
    registerInterface( env, jobj ); // after the initial init all Java callbacks are executed on a threaded Java Object!

    OPENSL_STREAM *p;

    p = android_OpenAudioDevice( audio_engine::SAMPLE_RATE,     audio_engine::INPUT_CHANNELS,
                                 audio_engine::OUTPUT_CHANNELS, audio_engine::BUFFER_SIZE );

    // hardware unavailable ? halt thread, trigger JNI callback for error handler
    if ( p == NULL )
    {
        handleHardwareUnavailable();
        return;
    }
    // audio hardware available, start render thread

    int buffer_size, i, c, ci;
    buffer_size        = audio_engine::BUFFER_SIZE;
    int outputChannels = audio_engine::OUTPUT_CHANNELS;
    bool isMono        = outputChannels == 1;
    bool loopStarted = false;   // whether the current buffer will exceed the end offset of the loop (read remaining samples from the start)
    int loopOffset = 0;         // the offset within the current buffer where we start reading from the current loops start offset
    int loopAmount = 0;         // amount of samples we must read from the current loops start offset
    std::vector<AudioChannel*> channels;
    std::vector<AudioChannel*> channels2;       // used when loop starts for gathering events at the start range

    float recbufferIn   [ buffer_size ];                  // used for recording from device input
    float outbuffer     [ buffer_size * outputChannels ]; // the output buffer rendered by the hardware

    // generate buffers for temporary channel buffer writes
    AudioBuffer* channelBuffer = new AudioBuffer( outputChannels, buffer_size );
    AudioBuffer* inbuffer      = new AudioBuffer( outputChannels, buffer_size ); // accumulates all channels ("master strip")
    AudioBuffer* recbuffer     = new AudioBuffer( audio_engine::INPUT_CHANNELS, buffer_size );

    thread = 1;

    // signal processors
    Finalizer* limiter = new Finalizer  ( 2, 500, audio_engine::SAMPLE_RATE, outputChannels );
    LPFHPFilter* hpf   = new LPFHPFilter(( float ) audio_engine::SAMPLE_RATE, 45, outputChannels );

    while ( thread )
    {
        // erase previous buffer contents
        inbuffer->silenceBuffers();

        // gather the audio events by the buffer range currently being processed
        int endPosition = bufferPosition + buffer_size;
        channels        = sequencer::getAudioEvents( channels, bufferPosition, endPosition, true );

        // read pointer exceeds maximum allowed offset ? => sequencer has started its loop
        // we must now also gather extra events at the start position of the seq. range
        loopStarted = endPosition > max_buffer_position;
        loopOffset  = (( max_buffer_position + 1 ) - bufferPosition );
        loopAmount  = buffer_size - loopOffset;

        if ( loopStarted )
        {
            // were we bouncing the audio ? save file and stop rendering
            if ( bouncing )
            {
                DiskWriter::writeBufferToFile( audio_engine::SAMPLE_RATE, audio_engine::OUTPUT_CHANNELS, false );

                // broadcast update via JNI, pass buffer identifier name to identify last recording
                handleBounceComplete( 1 );
                thread = 0; // stop thread, halts rendering
                break;
            }
            else
            {
                endPosition -= max_buffer_position;
                channels2 = sequencer::getAudioEvents( channels2, min_buffer_position, min_buffer_position + buffer_size, false );

                // er? the channels are magically merged by above invocation..., performing the insert below adds the same events TWICE*POP*!?!?
                //channels.insert( channels.end(), channels2.begin(), channels2.end() ); // merge the channels into one

                channels2.clear();  // would clear on next "getAudioEvents"-query... but why wait ?
            }
        }

        // record audio from Android device ?
        if ( recordFromDevice && audio_engine::INPUT_CHANNELS > 0 )
        {
            int recSamps                  = android_AudioIn( p, recbufferIn, audio_engine::BUFFER_SIZE );
            SAMPLE_TYPE* recBufferChannel = recbuffer->getBufferForChannel( 0 );

            for ( int j = 0; j < recSamps; ++j )
            {
                recBufferChannel[ j ] = recbufferIn[ j ];//static_cast<float>( recbufferIn[ j ] );

                // merge recording into current input buffer for instant monitoring
                if ( monitorRecording )
                {
                    for ( int k = 0; k < outputChannels; ++k )
                        inbuffer->getBufferForChannel( k )[ j ] = recBufferChannel[ j ];
                }
            }
        }

        // channel loop
        int j = 0;
        int channelAmount = channels.size();

        for ( j; j < channelAmount; ++j )
        {
            AudioChannel* channel = channels[ j ];
            bool isCached         = channel->hasCache;                // whether this channel has a fully cached buffer
            bool mustCache        = audio_engine::USE_CACHING && channel->canCache() && !isCached; // whether to cache this channels output
            bool gotBuffer        = false;
            int cacheReadPos      = 0;  // the offset we start ready from the channel buffer (when writing to cache)

            SAMPLE_TYPE channelVolume                = ( SAMPLE_TYPE ) channel->mixVolume;
            std::vector<BaseAudioEvent*> audioEvents = channel->audioEvents;
            int amount                               = audioEvents.size();

            // clear previous channel buffer content
            channelBuffer->silenceBuffers();

            bool useChannelRange = ( channel->maxBufferPosition != 0 ); // channel has its own buffer range (i.e. drumloops)
            int maxBufferPosition = useChannelRange ? channel->maxBufferPosition : max_buffer_position;

            // we make a copy of the current buffer position indicator
            int bufferPos = bufferPosition;

            // ...in case the AudioChannels maxBufferPosition differs from the sequencer loop range
            // note that these buffer positions are always a full bar in length (as we loop measures)
            while ( bufferPos > maxBufferPosition )
                bufferPos -= bytes_per_bar;

            // only render sequenced events when the sequencer isn't in the paused state
            // and the channel volume is actually at an audible level! ( > 0 )

            if ( playing && amount > 0 && channelVolume > 0.0 )
            {
                if ( !isCached )
                {
                    // write the audioEvent buffers into the main output buffer
                    for ( int k = 0; k < amount; ++k )
                    {
                        BaseAudioEvent* vo = audioEvents[ k ];

                        if ( !vo->isLocked())   // make sure we are allowed to query the contents
                        {
                            vo->lock();         // prevent buffer mutations during this read cycle

                            // read from a pre-cached buffer for sequenced notes
                            // first we cache references to the AudioEvents properties
                            AudioBuffer* buffer = vo->getBuffer();
                            int startOffset     = vo->getSampleStart();
                            int endOffset       = vo->getSampleEnd();
                            int sampleLength    = vo->getSampleLength();

                            for ( i = 0; i < buffer_size; ++i )
                            {
                                int readPointer = i + bufferPos;

                                // over the max position ? read from the start ( sequence has started loop )
                                if ( readPointer >= maxBufferPosition )
                                {
                                    if ( useChannelRange )  // TODO: channels use a min buffer position too ? (currently drumloop only)
                                        readPointer -= maxBufferPosition;

                                    else if ( !loopStarted )
                                        readPointer -= ( maxBufferPosition - min_buffer_position );
                                }
                                if ( readPointer >= startOffset && readPointer <= endOffset )
                                {
                                    // mind the offset ! ( cached buffer starts at 0 while
                                    // the startOffset defines where the event is positioned in the sequencer )
                                    readPointer -= startOffset;

                                    for ( int c = 0, ca = buffer->amountOfChannels; c < ca; ++c )
                                    {
                                        SAMPLE_TYPE* srcBuffer = buffer->getBufferForChannel( c );
                                        SAMPLE_TYPE* tgtBuffer = channelBuffer->getBufferForChannel( c );

                                        tgtBuffer[ i ] += srcBuffer[ readPointer ];
                                    }
                                }
                                else
                                {
                                    if ( loopStarted )
                                    {
                                        if ( i >= loopOffset )
                                        {
                                            readPointer = min_buffer_position + ( i - loopOffset );

                                            if ( readPointer >= startOffset && readPointer <= endOffset )
                                            {
                                                readPointer -= startOffset;

                                                for ( int c = 0, ca = buffer->amountOfChannels; c < ca; ++c )
                                                {
                                                    SAMPLE_TYPE* srcBuffer = buffer->getBufferForChannel( c );
                                                    SAMPLE_TYPE* tgtBuffer = channelBuffer->getBufferForChannel( c );

                                                    tgtBuffer[ i ] += srcBuffer[ readPointer ];
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            vo->unlock();   // release lock
                        }
                    }
                }
                else
                {
                    channel->readCachedBuffer( channelBuffer, bufferPos );
                }
            }

            // perform live rendering for this instrument
            if ( channel->hasLiveEvents )
            {
                int lAmount = channel->liveEvents.size();

                // the volume of the live events is divided by the channel mix as a live event
                // is played on the same instrument, but just as a different voice (note the
                // events can have their own mix level)

                float lAmp = channel->mixVolume > 0.0 ? 1.0 / channel->mixVolume : 1.0;

                for ( int k = 0; k < lAmount; ++k )
                {
                    BaseAudioEvent* vo = channel->liveEvents[ k ];
                    channelBuffer->mergeBuffers( vo->synthesize( buffer_size ), 0, 0, lAmp );
                }
            }

            // apply the processing chains processors / modulators
            ProcessingChain* chain = channel->processingChain;
            std::vector<BaseProcessor*> processors = chain->getActiveProcessors();

            for ( int k = 0; k < processors.size(); k++ )
            {
                BaseProcessor* processor = processors[ k ];
                bool canCacheProcessor   = processor->isCacheable();

                // only apply processor when we're not caching or cannot cache its output
                if ( !isCached || !canCacheProcessor )
                {
                    // cannot cache this processor and we're caching ? write all contents
                    // of the channelBuffer into the channels cache
                    if ( mustCache && !canCacheProcessor )
                        mustCache = !writeChannelCache( channel, channelBuffer, cacheReadPos );

                    processors[ k ]->process( channelBuffer, channel->isMono );
                }
            }

            // write cache if it didn't happen yet ;) (bus processors are (currently) non-cacheable)
            if ( mustCache )
                mustCache = !writeChannelCache( channel, channelBuffer, cacheReadPos );

            // apply bus processors
            std::vector<BaseBusProcessor*> bus = chain->getActiveBusProcessors();

            for ( int k = 0; k < bus.size(); k++ )
            {
                bus[ k ]->apply( channelBuffer, buffer_size );
            }

            // write the channel buffer into the combined output buffer, apply channel volume
            // note live events are always audible as their volume is relative to the instrument
            if ( channel->hasLiveEvents && channelVolume == 0.0 ) channelVolume = 1.0f;
            inbuffer->mergeBuffers( channelBuffer, 0, 0, channelVolume );
        }

        // apply high pass filtering to prevent extreme low rumbling and nasty filter offsets
        hpf->process( inbuffer, buffer_size );

        // apply the busProcessors to the combined output
        /* // never used before...
        if ( busProcessors.size() > 0 )
        {
            for ( BaseBusProcessor p : busProcessors)
                p.apply( sample, buffer_size );
        }
        */
        // limit the audio to prevent clipping
        limiter->process( inbuffer, isMono );

        // write the accumulated buffers into the output buffer
        for ( i = 0, c = 0; i < buffer_size; i++, c += outputChannels )
        {
            for ( ci = 0; ci < outputChannels; ci++ )
            {
                float sample = ( float ) inbuffer->getBufferForChannel( ci )[ i ] * volume; // apply master volume

                // extreme limiting (still above the thresholds?)
                if ( sample < -MAX_PHASE )
                    sample = -MAX_PHASE;

                else if ( sample > +MAX_PHASE )
                    sample = +MAX_PHASE;

                outbuffer[ c + ci ] = sample;
            }

            // update the buffer pointers and sequencer position
            if ( playing )
            {
                if ( ++bufferPosition % bytes_per_tick == 0 )
                   handleSequencerPositionUpdate( android_GetTimestamp( p ));

                if ( bufferPosition > max_buffer_position )
                    bufferPosition = min_buffer_position;
           }
        }
        // render the buffer in the audio hardware (unless we're bouncing as writing the output
        // makes it both unnecessarily audible and stalls this thread's execution
        if ( !bouncing )
            android_AudioOut( p, outbuffer, buffer_size * audio_engine::OUTPUT_CHANNELS );

        // record the output if recording state is active
        if ( playing && ( recordOutput || recordFromDevice ))
        {
            if ( recordFromDevice ) // recording from device input ? > write the record buffer
                DiskWriter::appendBuffer( recbuffer );
            else                    // recording global output ? > write the combined buffer
                DiskWriter::appendBuffer( inbuffer );

            // exceeded maximum recording buffer amount ? > write current recording
            if ( DiskWriter::bufferFull() || haltRecording )
            {
                int amountOfChannels = recordFromDevice ? audio_engine::INPUT_CHANNELS : outputChannels;
                DiskWriter::writeBufferToFile( audio_engine::SAMPLE_RATE, amountOfChannels, true );

                if ( !haltRecording )
                {
                    DiskWriter::generateOutputBuffer(); // allocate new buffer for next iteration
                    ++recordingFileName;
                }
                else {
                    haltRecording = false;
                }
            }
        }

        // tempo update queued ?
        if ( queuedTempo != tempo )
            handleTempoUpdate( queuedTempo, true );
    }
    android_CloseAudioDevice( p );

    // clear heap memory allocated before thread loop
    delete inbuffer;
    delete channelBuffer;
    delete limiter;
    delete hpf;
}

extern "C"
void stop()
{
    thread = 0;
    DebugTool::log( "NATIVE AUDIO ENGINE :: STOPPED OpenSL engine" );
}

extern "C"
void reset()
{
    DebugTool::log( "NATIVE AUDIO ENGINE :: RESET" );

    // nothing much... references are currently maintained by Java, causing SWIG to destruct referenced Objects

    sequencer::clearEvents();

    bufferPosition   = 0;
    stepPosition     = 0;
    recordOutput     = false;
    recordFromDevice = false;
    bouncing         = false;
}

/* private methods */

void handleTempoUpdate( float aQueuedTempo, bool broadcastUpdate )
{
    if ( broadcastUpdate )
        tempo = aQueuedTempo;

    time_sig_beat_amount = queuedTime_sig_beat_amount;
    time_sig_beat_unit   = queuedTime_sig_beat_unit;

    float oldPosition     = ( float ) bufferPosition / ( float ) max_buffer_position;  // pct of loop offset
    float tempBytesPerBar = ((( float ) audio_engine::SAMPLE_RATE * 60 ) / tempo ) * 4; // a full bar at 4 beats per measure

    bytes_per_beat         = ( int ) ( tempBytesPerBar / ( float ) time_sig_beat_unit );

    // bytes per tick equals the smallest note size the sequencer acknowledges (i.e. 8ths, 16ths, 32nds, 64ths, etc.)
    bytes_per_tick        = bytes_per_beat / beat_subdivision;
    bytes_per_bar         = ( int ) ( tempBytesPerBar / ( float ) time_sig_beat_unit * ( float ) time_sig_beat_amount ); // in case of non-equals amount vs. unit

    max_buffer_position = ( bytes_per_bar * amount_of_bars ) - 1; // -1 as we use array lookups and start at 0 // TODO: single time sig for all bars!!

    // make sure relative positions remain in sync
    bufferPosition = ( int ) llround( max_buffer_position * oldPosition );

    // broadcast update (so the Sequencer can invoke a re-calculation
    // on all existing audio events to match the new tempo / time signature)
    if ( broadcastUpdate )
    {
        jmethodID native_method_id = getJavaMethod( JavaAPIs::TEMPO_UPDATED );

        if ( native_method_id != 0 )
        {
            JNIEnv* env = getEnvironment();

            if ( env != 0 )
            {
                env->CallStaticVoidMethod( getJavaInterface(), native_method_id,
                                           tempo, bytes_per_beat, bytes_per_tick, bytes_per_bar, time_sig_beat_amount, time_sig_beat_unit );
            }
        }
    }
}

void handleSequencerPositionUpdate( float streamTimeStamp )
{
    ++stepPosition;

    // larger-equal check as we can remove measures from the sequencer while running
    if ( stepPosition >= max_step_position )
        stepPosition = min_step_position;

    /* // the OBL way
    if ( _soundChannel != null )
        _latency = ( e.position * 2.267573696145e-02 ) - _soundChannel.position;

     sample latency = (latency in milliseconds / 1000) * SAMPLE_RATE

    _timelines[ i ].updatePosition( MathTool.roundPos(( _position + _latency ) / BYTES_PER_TICK ) - 1 );
    */
    broadcastStepPosition();
}

void broadcastStepPosition()
{
    jmethodID native_method_id = getJavaMethod( JavaAPIs::SEQUENCER_STEP_UPDATE );

    if ( native_method_id != 0 )
    {
        JNIEnv* env = getEnvironment();

        if ( env != 0 )
            env->CallStaticVoidMethod( getJavaInterface(), native_method_id, stepPosition );
    }
}

void handleHardwareUnavailable()
{
    jmethodID native_method_id = getJavaMethod( JavaAPIs::OPENSL_INITIALIZATION_ERROR );

    if ( native_method_id != 0 )
    {
        JNIEnv* env = getEnvironment();

        if ( env != 0 )
        env->CallStaticVoidMethod( getJavaInterface(), native_method_id );
    }
}

bool writeChannelCache( AudioChannel* channel, AudioBuffer* channelBuffer, int cacheReadPos )
{
    // must cache isn't the same as IS caching (likely sequencer is waiting for start offset ;))
    if ( channel->isCaching )
        channel->writeCache( channelBuffer, cacheReadPos );
    else
        return false;

    return true; // indicates we have written the buffer to the cache
}

void handleBounceComplete( int aIdentifier )
{
    jmethodID native_method_id = getJavaMethod( JavaAPIs::BOUNCE_COMPLETED );

    if ( native_method_id != 0 )
    {
        JNIEnv* env = getEnvironment();

        if ( env != 0 )
            env->CallStaticVoidMethod( getJavaInterface(), native_method_id, aIdentifier );
    }
}

/*

THE SEAMLESS LOOPING ISSUE
--------------------------

REAL LIFE SCENARIO:

buffer size is 512 samples
current loop range is 0 - 88199 (88200 samples total length)

one of the loops starts at offset 87928
this means that max pos ( 88199 + 1 ) - current position 87928 = 272 samples read from the end range, this amount is the LOOPOFFSET. From this offset the remaining BUFFER SIZE 512 - LOOPOFFSET 272 = 240 samples are read from the start of the loops range.

if ( LOOPING && ( i >= LOOPOFFSET ))

using calculation mbp 0 + ( i - LOOPOFFSET ) would give us:

0 + ( 272 - 272 ) = 0
0 + ( 273 - 272 ) = 1
0 + ( 274 - 272 ) = 2

…up until

0 + (( BUFFER SIZE - 1 ) 511 - 272 ) = 239 (array index of 240th value)

-------

EASY FOR HUMAN BRAIN SCENARIO:

buffer size is 5 samples
current loop range is 0 - 7 (8 samples total length), buffer looks like:

1 2 3 4
-------
0 1 2 3 array index

5 6 7 8
-------
4 5 6 7 array index

one of the loops starts at array offset 5, this means that max pos (7 + 1 ) - current position 5 = 3 samples read from the end range (at array slots 5,6,7). From this offset the remaining ( BUFFER SIZE 5 - LOOPOFFSET 3 = ) 2 samples read from the start of the range (at array slots 0,1 using current buffer indices 3,4)
*/
