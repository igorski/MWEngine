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
#include <vector>
#include "audioengine.h"
#include "global.h"
#include "audiochannel.h"
#include "baseaudioevent.h"
#include "diskwriter.h"
#include "processingchain.h"
#include "finalizer.h"
#include "lpfhpfilter.h"
#include "sequencer.h"
#include "opensl_io.h"
#include "observer.h"
#include "utils.h"

namespace AudioEngine
{
    int bytes_per_beat;
    int bytes_per_bar;
    int bytes_per_tick;

    int amount_of_bars      = 1;
    int beat_subdivision    = 4;
    int min_buffer_position = 0; // initially 0, but can differ when looping specific measures
    int max_buffer_position = 0; // calculated when sequencer API creates output
    int min_step_position   = 0;
    int max_step_position   = 16;

    bool playing          = false;
    bool recordOutput     = false;
    bool haltRecording    = false;
    bool bouncing         = false;
    bool recordFromDevice = false;
    bool monitorRecording = false; // might introduce feedback on microphone ;)
    int recordingFileId = 0;

    /* buffer read/write pointers */

    int bufferPosition = 0;
    int stepPosition   = 0;

    /* tempo related */

    float tempo                    = 90.0;
    float queuedTempo              = 120.0;
    int time_sig_beat_amount       = 4;
    int time_sig_beat_unit         = 4;
    int queuedTime_sig_beat_amount = time_sig_beat_amount;
    int queuedTime_sig_beat_unit   = time_sig_beat_unit;

    /* output related */

    float volume = .85;

    static int thread;

    /**
     * starts the render thread
     * NOTE: the render thread is always active, even when the
     * sequencer is paused
     */
    void start()
    {
        OPENSL_STREAM *p;

        p = android_OpenAudioDevice( AudioEngineProps::SAMPLE_RATE,     AudioEngineProps::INPUT_CHANNELS,
                                     AudioEngineProps::OUTPUT_CHANNELS, AudioEngineProps::BUFFER_SIZE );

        // hardware unavailable ? halt thread, trigger JNI callback for error handler
        if ( p == NULL )
        {
            Observer::handleHardwareUnavailable();
            return;
        }
        // audio hardware available, start render thread

        int buffer_size, i, c, ci;
        buffer_size        = AudioEngineProps::BUFFER_SIZE;
        int outputChannels = AudioEngineProps::OUTPUT_CHANNELS;
        bool isMono        = outputChannels == 1;
        std::vector<AudioChannel*> channels;
        std::vector<AudioChannel*> channels2; // used when loop starts for gathering events at the start range

        bool loopStarted = false;   // whether the current buffer will exceed the end offset of the loop (read remaining samples from the start)
        int loopOffset = 0;         // the offset within the current buffer where we start reading from the current loops start offset
        int loopAmount = 0;         // amount of samples we must read from the current loops start offset

        float recbufferIn   [ buffer_size ];                  // used for recording from device input
        float outbuffer     [ buffer_size * outputChannels ]; // the output buffer rendered by the hardware

        // generate buffers for temporary channel buffer writes
        AudioBuffer* channelBuffer = new AudioBuffer( outputChannels, buffer_size );
        AudioBuffer* inbuffer      = new AudioBuffer( outputChannels, buffer_size ); // accumulates all channels ("master strip")
        AudioBuffer* recbuffer     = new AudioBuffer( AudioEngineProps::INPUT_CHANNELS, buffer_size );

        thread = 1;

        // signal processors
        Finalizer* limiter = new Finalizer  ( 2, 500,  AudioEngineProps::SAMPLE_RATE, outputChannels );
        LPFHPFilter* hpf   = new LPFHPFilter(( float ) AudioEngineProps::SAMPLE_RATE, 55, outputChannels );

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
                    DiskWriter::writeBufferToFile( AudioEngineProps::SAMPLE_RATE, AudioEngineProps::OUTPUT_CHANNELS, false );

                    // broadcast update via JNI, pass buffer identifier name to identify last recording
                    Observer::handleBounceComplete( 1 );
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
            if ( recordFromDevice && AudioEngineProps::INPUT_CHANNELS > 0 )
            {
                int recSamps                  = android_AudioIn( p, recbufferIn, AudioEngineProps::BUFFER_SIZE );
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
                bool mustCache        = AudioEngineProps::USE_CACHING && channel->canCache() && !isCached; // whether to cache this channels output
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

                // write the channel buffer into the combined output buffer, apply channel volume
                // note live events are always audible as their volume is relative to the instrument
                if ( channel->hasLiveEvents && channelVolume == 0.0 ) channelVolume = 1.0f;
                inbuffer->mergeBuffers( channelBuffer, 0, 0, channelVolume );
            }

            // TODO: create bus processors for these ?

            // apply high pass filtering to prevent extreme low rumbling and nasty filter offsets
            hpf->process( inbuffer, buffer_size );

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
                android_AudioOut( p, outbuffer, buffer_size * AudioEngineProps::OUTPUT_CHANNELS );

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
                    int amountOfChannels = recordFromDevice ? AudioEngineProps::INPUT_CHANNELS : outputChannels;
                    DiskWriter::writeBufferToFile( AudioEngineProps::SAMPLE_RATE, amountOfChannels, true );

                    if ( !haltRecording )
                    {
                        DiskWriter::generateOutputBuffer(); // allocate new buffer for next iteration
                        ++recordingFileId;
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

    void stop()
    {
        thread = 0;
        DebugTool::log( "MWENGINE :: STOPPED OpenSL engine" );
    }

    void reset()
    {
        DebugTool::log( "MWENGINE :: RESET" );

        // nothing much... references are currently maintained by Java, causing SWIG to destruct referenced Objects

        sequencer::clearEvents();

        bufferPosition   = 0;
        stepPosition     = 0;
        recordOutput     = false;
        recordFromDevice = false;
        bouncing         = false;
    }

    /* internal methods */

    void handleTempoUpdate( float aQueuedTempo, bool broadcastUpdate )
    {
        if ( broadcastUpdate )
            tempo = aQueuedTempo;

        time_sig_beat_amount = queuedTime_sig_beat_amount;
        time_sig_beat_unit   = queuedTime_sig_beat_unit;

        float oldPosition     = ( float ) bufferPosition / ( float ) max_buffer_position;  // pct of loop offset
        float tempBytesPerBar = ((( float ) AudioEngineProps::SAMPLE_RATE * 60 ) / tempo ) * 4; // a full bar at 4 beats per measure

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
            Observer::broadcastTempoUpdate();
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
        Observer::broadcastStepPosition();
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
}

/**
 * this is only in use if javajni.h is included in the SWIG .i-definitions file
 * it provides a proxied hook into the methods of the AudioEngine that allow
 * us to grab a reference to the VM and Java object, so we can send messages
 * to it via the Java Bridge
 */
#ifdef USE_JNI

#include <jni.h>
#include "javabridge.h"

/**
 * registers the calling Object and its environment
 * in the Java bridge (note: there should be only one
 * Java class talking to the engine)
 */
extern "C"
void init( JNIEnv* env, jobject jobj )
{
    JavaBridge::registerInterface( env, jobj );
}

/**
 * when starting the render thread of the AudioEngine, we
 * make sure the JavaBridge is registered to the right
 * env en jobj (as it is likely that this is invoked from
 * a different Java thread that loaded the MWEngine (which
 * can cause concurrency issues when broadcasting messages to the VM)
 */
extern "C"
void start( JNIEnv* env, jobject jobj )
{
    JavaBridge::registerInterface( env, jobj );
    AudioEngine::start();
}

extern "C"
void stop( JNIEnv* env, jobject jobj )
{
    AudioEngine::stop();
}

extern "C"
void reset( JNIEnv* env, jobject jobj )
{
    AudioEngine::reset();
}

#endif
