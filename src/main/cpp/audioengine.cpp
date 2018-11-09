/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2018 Igor Zinken - http://www.igorski.nl
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
#include "audioengine.h"
#include "global.h"
#include "audiochannel.h"
#include "processingchain.h"
#include "sequencer.h"
#include <drivers/adapter.h>
#include <definitions/notifications.h>
#include <messaging/notifier.h>
#include <events/baseaudioevent.h>
#include <utilities/debug.h>
#include <vector>

#ifdef RECORD_TO_DISK
#include <utilities/diskwriter.h>
#endif

// whether to include JNI classes to add the Java bridge

#ifdef USE_JNI

#include <jni.h>
#include <jni/javabridge.h>

#endif

namespace AudioEngine
{
    bool recordOutputToDisk = false;
    bool haltRecording      = false;
    bool bouncing           = false;
    bool recordInputToDisk  = false;
    int recordingFileId = 0;

    bool loopStarted = false;
    int loopOffset   = 0;
    int loopAmount   = 0;

    int outputChannels = AudioEngineProps::OUTPUT_CHANNELS;
    bool isMono        = ( outputChannels == 1 );
    float* outBuffer   = nullptr;

#ifdef RECORD_DEVICE_INPUT
    float* recbufferIn         = nullptr;
    AudioChannel* inputChannel = new AudioChannel( 1.0f );
    bool recordDeviceInput     = false;
#endif

    AudioBuffer* inBuffer                = nullptr;
    std::vector<AudioChannel*>* channels = nullptr;

    /* tempo / sequencer position related */

    int samples_per_beat;                // strictly speaking sequencer specific, but scoped onto the AudioEngine
    int samples_per_bar;                 // for rendering purposes, see SequencerController on how to read and
    float samples_per_step;              // adjust these values
    int amount_of_bars             = 1;
    int beat_subdivision           = 4;
    int min_buffer_position        = 0;  // initially 0, but can differ when looping specific measures
    int max_buffer_position        = 0;  // calculated when SequencerController is created
    int marked_buffer_position     = -1; // -1 means no marker has been set, no notifications will go out
    int min_step_position          = 0;
    int max_step_position          = 15; // default to sixteen step sequence (note min step starts at 0)
    float tempo                    = 90.0;
    float queuedTempo              = 120.0;
    int time_sig_beat_amount       = 4;
    int time_sig_beat_unit         = 4;
    int queuedTime_sig_beat_amount = time_sig_beat_amount;
    int queuedTime_sig_beat_unit   = time_sig_beat_unit;

    /* buffer read/write pointers */

    int bufferPosition = 0;
    int stepPosition   = 0;

    /* output related */

    float volume = 1.0f;
    ProcessingChain* masterBus = new ProcessingChain();

    static int thread;

    void setup( int bufferSize, int sampleRate, int amountOfChannels )
    {
        AudioEngineProps::BUFFER_SIZE     = bufferSize;
        AudioEngineProps::SAMPLE_RATE     = sampleRate;
        AudioEngineProps::OUTPUT_CHANNELS = amountOfChannels;
    }

    /**
     * starts the render thread
     * NOTE: the render thread is always active, even when the
     * sequencer is paused
     */
    void start()
    {
        if ( thread == 1 )
            return;

        Debug::log( "STARTING engine" );

        // create the output driver using the adapter. If creation failed
        // prevent thread start and trigger JNI callback for error handler

        if ( !DriverAdapter::create() )
        {
            Notifier::broadcast( Notifications::ERROR_HARDWARE_UNAVAILABLE );
            return;
        }

        Debug::log( "STARTED engine" );

        // audio hardware available, prepare environment

        channels       = new std::vector<AudioChannel*>();
        outputChannels = AudioEngineProps::OUTPUT_CHANNELS;
        isMono         = ( outputChannels == 1 );
        outBuffer      = new float[ AudioEngineProps::BUFFER_SIZE * outputChannels ]();

#ifdef RECORD_DEVICE_INPUT

        // generate the input buffer used for recording from the device's input
        // as well as the temporary buffer used to merge the input into

        recbufferIn  = new float[ AudioEngineProps::BUFFER_SIZE ]();
        inputChannel->createOutputBuffer();
#endif
        // accumulates all channels ("master strip")

        inBuffer = new AudioBuffer( outputChannels, AudioEngineProps::BUFFER_SIZE );

        // ensure all AudioChannel buffers have the correct properties (in case engine is
        // restarting after changing buffer size, for instance)

        std::vector<BaseInstrument*> instruments = Sequencer::instruments;

        for ( int i = 0; i < instruments.size(); ++i )
            instruments.at( i )->audioChannel->createOutputBuffer();

        // start thread and request first render (gets render loop going)

        thread = 1;

        while ( thread )
        {
            // will only be interrupted if thread is set to 0 in render() method
            // the audio drivers are responsible for calling render() at the
            // appropriate time during this threads execution

            DriverAdapter::render();
        }

        Debug::log( "STOPPED engine" );

        DriverAdapter::destroy();

        // clear heap memory allocated before thread loop
        delete channels;
        delete outBuffer;
        delete inBuffer;

        channels  = nullptr;
        outBuffer = nullptr;
        inBuffer  = nullptr;

#ifdef RECORD_DEVICE_INPUT
        delete recbufferIn;
        recBufferIn = nullptr;
#endif
    }

    void stop()
    {
        Debug::log( "STOPPING engine" );
        thread = 0;
    }

    void reset()
    {
        Debug::log( "RESET engine" );

        // nothing much... when used with USE_JNI references are maintained by Java, causing SWIG to
        // destruct referenced Objects when breaking references through Java
        // when USE_JNI is not defined, manual cleanup of allocated events must follow

        Sequencer::clearEvents();

        bufferPosition    = 0;
        stepPosition      = 0;
#ifdef RECORD_DEVICE_INPUT
        recordDeviceInput = false;
#endif
        recordOutputToDisk = false;
        recordInputToDisk  = false;
        bouncing           = false;
    }

    bool render( int amountOfSamples )
    {
        if ( thread == 0 )
            return false;

        int i, c, ci;
        float sample;

        // erase previous buffer contents
        inBuffer->silenceBuffers();

        // gather the audio events by the buffer range currently being processed
        loopStarted = Sequencer::getAudioEvents( channels, bufferPosition, amountOfSamples, true, true );

        // read pointer exceeds maximum allowed offset (max_buffer_position) ? => sequencer has started its loop
        // we must now also gather extra events at the start position (min_buffer_position)
        loopOffset = ( max_buffer_position - bufferPosition ) + 1; // buffer index where the loop occurs
        loopAmount = amountOfSamples - loopOffset; // loopOffset is equal to the amount of samples read prior to loop start

        // collect all audio events that are eligible for playback for this iteration
        Sequencer::getAudioEvents( channels, min_buffer_position, loopAmount, false, false );

#ifdef RECORD_DEVICE_INPUT
        // record audio from Android device ?
        if (( recordDeviceInput || recordInputToDisk ) && AudioEngineProps::INPUT_CHANNELS > 0 )
        {
            int recSamps                  = DriverAdapter::getInput( recbufferIn );
            SAMPLE_TYPE* recBufferChannel = inputChannel->getOutputBuffer()->getBufferForChannel( 0 );

            for ( int j = 0; j < recSamps; ++j )
                recBufferChannel[ j ] = recbufferIn[ j ];//static_cast<float>( recbufferIn[ j ] );

            // apply processing chain onto the input

            std::vector<BaseProcessor*> processors = inputChannel->processingChain->getActiveProcessors();
            for ( int k = 0; k < processors.size(); ++k )
                processors[ k ]->process( inputChannel->getOutputBuffer(), AudioEngineProps::INPUT_CHANNELS == 1 );

            // merge recording into current input buffer for instant monitoring

            if ( inputChannel->getVolume() > 0.f ) {
                inputChannel->mixBuffer( inBuffer, inputChannel->getVolume() );
            }
        }
#endif
        // channel loop
        int j = 0;
        int channelAmount = channels->size();

        for ( ; j < channelAmount; ++j )
        {
            AudioChannel* channel = channels->at( j );
            bool isCached         = channel->hasCache;                // whether this channel has a fully cached buffer
            bool mustCache        = AudioEngineProps::CHANNEL_CACHING && channel->canCache() && !isCached; // whether to cache this channels output
            int cacheReadPos      = 0;  // the offset we start ready from the channel buffer (when writing to cache)

            std::vector<BaseAudioEvent*> audioEvents = channel->audioEvents;
            int amount = audioEvents.size();

            // divide the channels volume by the amount of channels to provide extra headroom
            SAMPLE_TYPE channelVolume = ( SAMPLE_TYPE ) channel->getVolumeLogarithmic() / ( SAMPLE_TYPE ) channelAmount;

            // get channel output buffer and clear previous contents
            AudioBuffer* channelBuffer = channel->getOutputBuffer();
            channelBuffer->silenceBuffers();

            bool useChannelRange  = channel->maxBufferPosition != 0; // channel has its own buffer range (i.e. drummachine)
            int maxBufferPosition = useChannelRange ? channel->maxBufferPosition : max_buffer_position;

            // we make a copy of the current buffer position indicator
            int bufferPos = bufferPosition;

            // ...in case the AudioChannels maxBufferPosition differs from the sequencer loop range
            // note that these buffer positions are always a full measure in length (as we loop by measures)
            while ( bufferPos > maxBufferPosition )
                bufferPos -= samples_per_bar;

            // only render sequenced events when the sequencer isn't in the paused state
            // and the channel volume is actually at an audible level! ( > 0 )

            if ( Sequencer::playing && amount > 0 && channelVolume > 0.0 )
            {
                if ( !isCached )
                {
                    // write the audioEvent buffers into the main output buffer
                    for ( int k = 0; k < amount; ++k )
                    {
                        BaseAudioEvent* audioEvent = audioEvents[ k ];

                        if ( audioEvent != nullptr && !audioEvent->isLocked()) // make sure we're allowed to query the contents
                        {
                            audioEvent->mixBuffer( channelBuffer, bufferPos, min_buffer_position,
                                                   maxBufferPosition, loopStarted, loopOffset, useChannelRange );
                        }
                    }
                }
                else
                {
                    channel->readCachedBuffer( channelBuffer, bufferPos );
                }
            }

            // perform live rendering for this channels instrument
            if ( channel->hasLiveEvents )
            {
                int lAmount = channel->liveEvents.size();

                // the volume of the live events is divided by the channel mix as a live event
                // is played on the same instrument, but just as a different voice (note the
                // events can have their own mix level)

                float lAmp = channelVolume > 0.f ? 1.0 / channelVolume : 1.0;

                for ( int k = 0; k < lAmount; ++k )
                {
                    BaseAudioEvent* vo = channel->liveEvents[ k ];
                    channelBuffer->mergeBuffers( vo->synthesize( amountOfSamples ), 0, 0, lAmp );
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
            if ( channel->hasLiveEvents && channelVolume == 0.0 )
                channelVolume = 1.0;

            channel->mixBuffer( inBuffer, channelVolume );
        }

        // apply master bus processors (e.g. high pass filter, limiter, etc.)
        std::vector<BaseProcessor*> processors = masterBus->getActiveProcessors();

        for ( int k = 0; k < processors.size(); k++ )
            processors[ k ]->process( inBuffer, isMono );

        // write the accumulated buffers into the output buffer
        for ( i = 0, c = 0; i < amountOfSamples; i++, c += outputChannels )
        {
            for ( ci = 0; ci < outputChannels; ci++ )
            {
                // apply the master volume onto the output
                sample = ( float ) inBuffer->getBufferForChannel( ci )[ i ] * volume;

                // and perform a fail-safe check in case we're exceeding the headroom ceiling

                if ( sample < -1.0 )
                    sample = -1.0f;

                else if ( sample > +1.0 )
                    sample = +1.0f;

                // write output interleaved (e.g. a sample per output channel
                // before continuing writing the next sample for the next channel range)

                outBuffer[ c + ci ] = sample;
            }

            // update the buffer pointers and sequencer position
            if ( Sequencer::playing )
            {
                if ( bufferPosition % ( int ) samples_per_step == 0 )
                {
                    // for higher accuracy we must calculate using floating point precision, it
                    // is a more expensive calculation than using integer modulo though, so we check
                    // only when the integer modulo operation check has passed
                    // TODO : this attempted fmod calculation is inaccurate.
                    //if ( std::fmod(( float ) bufferPosition, samples_per_step ) == 0 )
                        handleSequencerPositionUpdate( i );
                }
                if ( marked_buffer_position > 0 && bufferPosition == marked_buffer_position )
                     Notifier::broadcast( Notifications::MARKER_POSITION_REACHED );

                bufferPosition++;

                if ( bufferPosition > max_buffer_position )
                    bufferPosition = min_buffer_position;
            }
        }

        // write the synthesized output into the audio driver

        if ( !bouncing )
            DriverAdapter::writeOutput( outBuffer, amountOfSamples * outputChannels );


#ifdef RECORD_TO_DISK
        // write the output to disk if a recording state is active
        if (( Sequencer::playing && recordOutputToDisk ) || recordInputToDisk )
        {
#ifdef RECORD_DEVICE_INPUT
            if ( recordInputToDisk ) // recording from device input ? > write the record buffer
                DiskWriter::appendBuffer( inputChannel->getOutputBuffer() );
            else                    // recording global output ? > write the combined buffer
#endif
                DiskWriter::appendBuffer( outBuffer, amountOfSamples, outputChannels );

            // are we bouncing the current sequencer range and have we played throughed the full range?

            if ( bouncing && ( loopStarted || bufferPosition == 0 ))
            {
                DiskWriter::writeBufferToFile( AudioEngineProps::SAMPLE_RATE, outputChannels, false );
                // broadcast update via JNI, pass buffer identifier name to identify last recording
                Notifier::broadcast( Notifications::BOUNCE_COMPLETE, 1 );
                stop(); // stops thread, halts rendering
            }

            // exceeded maximum recording buffer amount ? > write current recording
            if ( DiskWriter::bufferFull() || haltRecording )
            {
                int amountOfChannels = recordInputToDisk ? AudioEngineProps::INPUT_CHANNELS : outputChannels;
                DiskWriter::writeBufferToFile( AudioEngineProps::SAMPLE_RATE, amountOfChannels, true );

                if ( !haltRecording )
                {
                    DiskWriter::generateOutputBuffer( amountOfChannels ); // allocate new buffer for next iteration
                    ++recordingFileId;
                }
                else {
                    haltRecording = false;
                }
            }
        }
#endif
        // tempo update queued ?
        if ( queuedTempo != tempo )
            handleTempoUpdate( queuedTempo, true );

        // render the buffer in the audio hardware (unless we're bouncing as writing the output
        // makes it both unnecessarily audible and stalls this thread's execution)
        bool keepRendering = ( thread == 1 );

#if DRIVER == 1
        // bit fugly, during bounce on AAudio driver, keep render loop going until bounce completes
        if ( bouncing && keepRendering )
            render( amountOfSamples );
#endif
        return keepRendering;
    }

    /* internal methods */

    void handleTempoUpdate( float aQueuedTempo, bool broadcastUpdate )
    {
        float ratio = 1;

        if ( broadcastUpdate ) {
            ratio = tempo / aQueuedTempo;
            tempo = aQueuedTempo;
        };

        time_sig_beat_amount = queuedTime_sig_beat_amount;
        time_sig_beat_unit   = queuedTime_sig_beat_unit;

        float tempSamplesPerBar = ((( float ) AudioEngineProps::SAMPLE_RATE * 60 ) / tempo ) * time_sig_beat_amount;
        samples_per_beat        = ( int ) ( tempSamplesPerBar / ( float ) time_sig_beat_amount );

        // samples per step describes the smallest note size the sequencer acknowledges (i.e. 8ths, 16ths, 32nds, 64ths, etc.)
        samples_per_step = ( float ) samples_per_beat / ( float ) beat_subdivision;
        samples_per_bar  = ( int )( samples_per_step * beat_subdivision * time_sig_beat_amount ); // in case of non-equals amount vs. unit

        int loopLength = max_buffer_position - min_buffer_position;

        min_buffer_position = ( int )(( float ) min_buffer_position * ratio );
        max_buffer_position = min_buffer_position + ( int )(( float ) loopLength * ratio );

        // make sure relative positions remain in sync
        bufferPosition = ( int )(( float ) bufferPosition * ratio );
        if ( marked_buffer_position > 0 ) {
            marked_buffer_position = ( int )(( float ) marked_buffer_position * ratio );
        }

        // inform sequencer of the update
        Sequencer::updateEvents();

        // broadcast update (so the Sequencer can invoke a re-calculation
        // on all existing audio events to match the new tempo / time signature)

        if ( broadcastUpdate )
        {
#ifdef USE_JNI
            // when using the engine through JNI with Java, we don't broadcast using
            // the Notifier, but instantly invoke a callback directly on the bridge
            // as it allows us to update multiple parameters at once

            jmethodID native_method_id = JavaBridge::getJavaMethod( JavaAPIs::TEMPO_UPDATED );

            if ( native_method_id != 0 )
            {
                JNIEnv* env = JavaBridge::getEnvironment();

                if ( env != 0 )
                    env->CallStaticVoidMethod( JavaBridge::getJavaInterface(), native_method_id, AudioEngine::tempo );
            }
#else
            Notifier::broadcast( Notifications::SEQUENCER_TEMPO_UPDATED );
#endif
        }
    }

    void handleSequencerPositionUpdate( int bufferOffset )
    {
        stepPosition = ( int ) floor( bufferPosition / samples_per_step );

        if ( stepPosition > max_step_position )
            stepPosition = min_step_position;

        Notifier::broadcast( Notifications::SEQUENCER_POSITION_UPDATED, bufferOffset );
    }

    bool writeChannelCache( AudioChannel* channel, AudioBuffer* channelBuffer, int cacheReadPos )
    {
        // mustCache isn't the same as isCaching (likely sequencer is waiting for start offset ;))
        if ( channel->isCaching )
            channel->writeCache( channelBuffer, cacheReadPos );
        else
            return false;

        return true; // indicates we have written the buffer to the cache
    }

#ifdef MOCK_ENGINE

    /* unit test related */

    bool engine_started    = false;
    int test_program       = 0;
    bool test_successful   = false;
    int render_iterations  = 0;
    float mock_opensl_time = 0.0f;

#endif

}

/**
 * the remainder is only in use when USE_JNI is set to true to allow using
 * the engine from Java. These method provide a proxied hook into the
 * public methods of the AudioEngine, these shouldn't be called directly
 * but are proxied via nl.igorski.lib.audio.MWEngine
 *
 * they are exposed in javabridge_api.h
 */
#ifdef USE_JNI

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

extern "C"
void setup( int bufferSize, int sampleRate, int amountOfChannels )
{
    AudioEngine::setup( bufferSize, sampleRate, amountOfChannels );
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

extern "C"
ProcessingChain* getMasterBusProcessors( JNIEnv* env, jobject jobj )
{
    return AudioEngine::masterBus;
}

extern "C"
AudioChannel* getInputChannel( JNIEnv* env, jobject jobj )
{
#ifdef RECORD_DEVICE_INPUT
    return AudioEngine::inputChannel;
#else
    return nullptr;
#endif
}

extern "C"
void recordInput( bool record )
{
#ifdef RECORD_DEVICE_INPUT
    AudioEngine::recordDeviceInput = record;
#endif
}

#endif
