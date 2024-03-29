/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2017-2022 Igor Zinken - https://www.igorski.nl
 *
 * AAudio driver implementation adapted from the Android Open Source Project
 * Input and output synchronisation (full duplex streams) adapted from the Oboe project
 *
 * Licensed under the Apache License, Version 2.0 (the "License" );
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
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
#include "aaudio_io.h"
#include "../global.h"
#include "../audioengine.h"
#include <assert.h>
#include <inttypes.h>
#include <utilities/debug.h>
#include <utilities/perfutility.h>

namespace MWEngine {

#ifndef INCLUDE_AAUDIO_LIBRARY
    LibraryLoader* AAudio_IO::libraryLoader = nullptr;
#endif

/**
 * Every time the playback stream requires data this method will be called.
 *
 * @param stream the audio stream which is requesting data, this is the _outputStream object
 * @param userData the context in which the function is being called (AAudio_IO instance)
 * @param audioData an empty buffer into which we can write our audio data
 * @param numFrames the number of audio frames which are required
 * @return Either AAUDIO_CALLBACK_RESULT_CONTINUE if the stream should continue requesting data
 *         or AAUDIO_CALLBACK_RESULT_STOP if the stream should stop.
 */
aaudio_data_callback_result_t dataCallback( AAudioStream* stream, void* userData, void* audioData, int32_t numFrames ) {
    assert(userData && audioData);
    AAudio_IO* instance = reinterpret_cast<AAudio_IO*>( userData );
    return instance->dataCallback( stream, audioData, numFrames );
}

/**
 * If there is an error with a stream this function will be called. A common example of an error
 * is when an audio device (such as headphones) is disconnected. In this case you should not
 * restart the stream within the callback, instead use a separate thread to perform the stream
 * recreation and restart.
 *
 * @param stream the stream with the error
 * @param userData the context in which the function is being called (AAudio_IO instance)
 * @param error the error which occured, a human readable string can be obtained using
 * AAudio_convertResultToText(error);
 */
void errorCallback( AAudioStream* stream, void *userData, aaudio_result_t error ) {
    assert( userData );
    AAudio_IO* instance = reinterpret_cast<AAudio_IO*>( userData );
    instance->errorCallback( stream, error );
}

AAudio_IO::AAudio_IO( int amountOfInputChannels, int amountOfOutputChannels ) {

    assert( isSupported());

    _inputChannelCount  = ( int16_t ) amountOfInputChannels;
    _outputChannelCount = ( int16_t ) amountOfOutputChannels;

    // MWEngine operates internally using floating point resolution
    // if floating point is supported by the hardware, we'd like to use it so we
    // can omit converting samples when reading and writing from the streams
    // sampleFormat can be updated during stream creation, if so, we will convert sample
    // formats as "AAudio might perform sample conversion on its own" <- nicely vague Google !

    _sampleFormat = AAUDIO_FORMAT_PCM_FLOAT;

    createAllStreams();
}

AAudio_IO::~AAudio_IO() {
    closeAllStreams();

    if ( _enqueuedOutputBuffer != nullptr ) {
        delete _enqueuedOutputBuffer;
        _enqueuedOutputBuffer = nullptr;
    }

    if ( _recordBuffer != nullptr ) {
        delete _recordBuffer;
        _recordBuffer = nullptr;
    }

    if ( _recordBufferI != nullptr ) {
        delete _recordBufferI;
        _recordBufferI = nullptr;
    }
}

bool AAudio_IO::isSupported() {
#ifdef INCLUDE_AAUDIO_LIBRARY
    return true;
#else
    libraryLoader = LibraryLoader::getInstance();
    return libraryLoader->loadAAudioLibrary();
#endif
}

/**
 * Set the audio device which should be used for playback. Can be set to AAUDIO_UNSPECIFIED if
 * you want to use the default playback device (which is usually the built-in speaker if
 * no other audio devices, such as headphones, are attached).
 *
 * @param deviceId the audio device id, can be obtained through an {@link AudioDeviceInfo} object
 * using Java/JNI.
 */
void AAudio_IO::setDeviceId( int32_t deviceId ) {

    _outputDeviceId = deviceId;

    // If this is a different device from the one currently in use then restart the stream
    int32_t currentDeviceId = AAudioStream_getDeviceId( _outputStream );
    if ( _outputDeviceId != currentDeviceId ) {
        restartStreams();
    }
}

void AAudio_IO::setRecordingDeviceId( int32_t deviceId ) {

    _inputDeviceId = deviceId;
    
    // If this is a different device from the one currently in use then restart the stream
    int32_t currentDeviceId = AAudioStream_getDeviceId( _inputStream );
    if ( _inputDeviceId != currentDeviceId ) {
        restartStreams();
    }
}

/**
 * Creates a stream builder which can be used to construct AAudioStreams
 */
AAudioStreamBuilder* AAudio_IO::createStreamBuilder() {
    AAudioStreamBuilder* builder = nullptr;
    aaudio_result_t result = static_cast<aaudio_result_t>(AAudio_createStreamBuilder( &builder ));
    if ( result != AAUDIO_OK && !builder ) {
        Debug::log( "AAudio_IO::Error creating stream builder: %s", AAudio_convertResultToText( result ));
    }
    return builder;
}

void AAudio_IO::createAllStreams() {

    // Create the output stream
    // This will also create the appropriate read and write buffers

    createOutputStream();

    // Create the recording stream
    // Note: The order of stream creation is important. We create the playback stream first,
    // then use properties from the playback stream (e.g. sample rate) to create the
    // recording stream. By matching the properties we should get the lowest latency path

    if ( _inputChannelCount > 0 ) {
        resetStreamStabilization();
        createInputStream();
    }

    // Now start the recording stream first so that we can read from it during the playback
    // stream's dataCallback - which is delegated to the driver adapter using getInput() -

    if ( _inputStream != nullptr ) {
        startStream( _inputStream );
    }
    if ( _outputStream != nullptr ) {
        startStream( _outputStream );
    }
}

void AAudio_IO::createInputStream() {

    AAudioStreamBuilder* builder = createStreamBuilder();

    if ( builder == nullptr ) {
        Debug::log( "AAudio_IO::Unable to obtain an AAudioStreamBuilder object" );
        return;
    }
    setupInputStream( builder );

    // Now that the parameters are set up we can open the stream
    aaudio_result_t result = static_cast<aaudio_result_t>(AAudioStreamBuilder_openStream( builder, &_inputStream ));
    if ( result == AAUDIO_OK && _inputStream != nullptr ) {
        if ( AAudioStream_getPerformanceMode( _inputStream ) != AAUDIO_PERFORMANCE_MODE_LOW_LATENCY ){
            Debug::log( "AAudio_IO::Input stream is NOT low latency. Check your requested format, sample rate and channel count" );
        }
    } else {
        Debug::log( "Failed to create recording stream. Error: %s", AAudio_convertResultToText( result ));
    }
    AAudioStreamBuilder_delete( builder );
}

void AAudio_IO::createOutputStream() {

    AAudioStreamBuilder* builder = createStreamBuilder();

    if ( builder == nullptr ) {
        Debug::log( "AAudio_IO::Unable to obtain an AAudioStreamBuilder object" );
        return;
    }

    setupOutputStream( builder );

    aaudio_result_t result = static_cast<aaudio_result_t>(AAudioStreamBuilder_openStream( builder, &_outputStream ));

    if ( result == AAUDIO_OK && _outputStream != nullptr ) {

        if ( AAudioStream_getPerformanceMode( _outputStream ) != AAUDIO_PERFORMANCE_MODE_LOW_LATENCY ) {
            Debug::log( "AAudio_IO::Output stream is NOT low latency. Check your requested format, sample rate and channel count" );
        }

        // verify requested format and update in case hardware does not support it
        // ideally we work in floating point across the engine to omit the need to convert samples

        if ( _sampleFormat != AAudioStream_getFormat( _outputStream )) {
            Debug::log( "AAudio_IO::Sample format does not match requested format %d", _sampleFormat );
            _sampleFormat = AAudioStream_getFormat( _outputStream );
        }

        _sampleRate     = AAudioStream_getSampleRate( _outputStream );
        _framesPerBurst = AAudioStream_getFramesPerBurst( _outputStream );

        AudioEngineProps::SAMPLE_RATE = _sampleRate;
        AudioEngine::handleTempoUpdate( AudioEngine::tempo, false ); // force sync to sample rate

        Debug::log( "AAudio_IO::Sample rate of AAudio stream is set to %d", _sampleRate );

        // Set the buffer size to the burst size - this will give us the minimum possible latency
        // This will also create the temporary read and write buffers

        updateBufferSizeInFrames( AAudioStream_setBufferSizeInFrames( _outputStream, _framesPerBurst ));

//          PrintAudioStreamInfo( _outputStream );

        // Store the underrun count so we can tune the latency in the dataCallback
        _underrunCountOutputStream = AAudioStream_getXRunCount( _outputStream );

    } else {
        Debug::log( "AAudio_IO::Failed to create stream. Error: %s", AAudio_convertResultToText( result ));
    }
    AAudioStreamBuilder_delete( builder );
}

/**
 * Sets the stream parameters which are specific to playback, including device id and the
 * dataCallback function, which must be set for low latency playback.
 */
void AAudio_IO::setupOutputStream( AAudioStreamBuilder* builder ) {
    AAudioStreamBuilder_setDeviceId    ( builder, _outputDeviceId );
    AAudioStreamBuilder_setFormat      ( builder, _sampleFormat );
    AAudioStreamBuilder_setChannelCount( builder, _outputChannelCount );

    // We request EXCLUSIVE mode since this will give us the lowest possible latency.
    // If EXCLUSIVE mode isn't available the builder will fall back to SHARED mode.
    
    AAudioStreamBuilder_setSharingMode    ( builder, AAUDIO_SHARING_MODE_EXCLUSIVE );
    AAudioStreamBuilder_setPerformanceMode( builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY );
    AAudioStreamBuilder_setDirection      ( builder, AAUDIO_DIRECTION_OUTPUT );
    AAudioStreamBuilder_setDataCallback   ( builder, ::dataCallback, this );
    AAudioStreamBuilder_setErrorCallback  ( builder, ::errorCallback, this );
}

/**
 * Sets the stream parameters which are specific to recording, including the sample rate which
 * is determined from the playback stream.
 */
void AAudio_IO::setupInputStream( AAudioStreamBuilder* builder ) {
    AAudioStreamBuilder_setDeviceId    ( builder, _inputDeviceId );
    AAudioStreamBuilder_setSampleRate  ( builder, _sampleRate );
    AAudioStreamBuilder_setChannelCount( builder, _inputChannelCount );
    AAudioStreamBuilder_setFormat      ( builder, _sampleFormat );
    
    // We request EXCLUSIVE mode since this will give us the lowest possible latency.
    // If EXCLUSIVE mode isn't available the builder will fall back to SHARED mode.
    
    AAudioStreamBuilder_setSharingMode    ( builder, AAUDIO_SHARING_MODE_EXCLUSIVE );
    AAudioStreamBuilder_setPerformanceMode( builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY );
    AAudioStreamBuilder_setDirection      ( builder, AAUDIO_DIRECTION_INPUT );
    AAudioStreamBuilder_setErrorCallback  ( builder, ::errorCallback, this );
}

void AAudio_IO::startStream( AAudioStream* stream ) {
    aaudio_result_t result = static_cast<aaudio_result_t>(AAudioStream_requestStart( stream ));
    if ( result != AAUDIO_OK ) {
        Debug::log( "AAudio_IO::Error starting stream. %s", AAudio_convertResultToText( result ));
    }
}

void AAudio_IO::stopStream( AAudioStream* stream ) {
    if ( stream == nullptr ) {
        return;
    }
    aaudio_result_t result = static_cast<aaudio_result_t>(AAudioStream_requestStop( stream ));
    if ( result != AAUDIO_OK ) {
        Debug::log( "AAudio_IO::Error stopping stream. %s", AAudio_convertResultToText( result ));
    }
}

void AAudio_IO::closeStream( AAudioStream* stream ) {
    if ( stream == nullptr ) {
        return;
    }
    stopStream( stream );
    aaudio_result_t result = static_cast<aaudio_result_t>(AAudioStream_close( stream ));
    if ( result != AAUDIO_OK ) {
        Debug::log( "AAudio_IO::Error closing stream. %s", AAudio_convertResultToText( result ));
    }
}

/**
 * Invoked whenever the AAudio drivers frame buffer size has updated
 * through AAudioStream_setBufferSizeInFrames (see dataCallback())
 *
 * This allows us to synchronize the changes across the engine and ensures we have the
 * appropriate size for our temporary read/write buffers
 */
void AAudio_IO::updateBufferSizeInFrames( int bufferSize ) {
    bool update = _bufferSizeInFrames != bufferSize || _enqueuedOutputBuffer == nullptr;

    if ( !update ) {
        return;
    }

    Debug::log( "AAudio_IO::Setting buffer size to %d", bufferSize );

    _bufferSizeInFrames = bufferSize;

    // sync across engine, be sure to call this when AudioEngine::render() isn't running
    AudioEngineProps::BUFFER_SIZE = _bufferSizeInFrames;
    AudioEngine::createOutputBuffer();

    // update temporary buffers as their size is now known (this operation should always happen
    // before or after a read and write ensuring no data loss / null pointer)

    // create the temporary buffers used to write data from and to the AudioEngine during playback and recording
    delete _enqueuedOutputBuffer;
    _enqueuedOutputBuffer = new float[ _bufferSizeInFrames * _outputChannelCount ]{ 0 };

    if ( _inputChannelCount > 0 ) {
        if ( _sampleFormat == AAUDIO_FORMAT_PCM_I16 ) {
            delete _recordBufferI;
            _recordBufferI = new int16_t[ _bufferSizeInFrames * _inputChannelCount ]{ 0 };
        } else {
            delete _recordBuffer;
            _recordBuffer = new float[ _bufferSizeInFrames * _inputChannelCount ]{ 0 };
        }
    }
}

aaudio_data_callback_result_t AAudio_IO::dataCallback( AAudioStream* stream, void *audioData, int32_t numFrames ) {
    assert( stream == _outputStream );

    int32_t underrunCount          = AAudioStream_getXRunCount( stream );
    int32_t bufferSize             = AAudioStream_getBufferSizeInFrames( stream );
    bool hasUnderrunCountIncreased = false;
    bool shouldChangeBufferSize    = false;

    if ( underrunCount > _underrunCountOutputStream ) {
        _underrunCountOutputStream = underrunCount;
        hasUnderrunCountIncreased  = true;
    }

    if ( hasUnderrunCountIncreased && _bufferSizeSelection == BUFFER_SIZE_AUTOMATIC ) {

        // This is a buffer size tuning algorithm. If the number of underruns (i.e. instances where
        // we were unable to supply sufficient data to the stream) has increased since the last callback
        // we will try to increase the buffer size by the burst size, which will give us more protection
        // against underruns in the future, at the cost of additional latency.

        bufferSize += _framesPerBurst; // Increase buffer size by one burst
        shouldChangeBufferSize = true;
    }
    else if ( _bufferSizeSelection > 0 && ( _bufferSizeSelection * _framesPerBurst ) != bufferSize )
    {
        // If the buffer size selection has changed then update it here
        bufferSize = _bufferSizeSelection * _framesPerBurst;
        shouldChangeBufferSize = true;
    }

    // Debug::log( "AAudio_IO::numFrames %d, Underruns %d, buffer size %d", numFrames, underrunCount, bufferSize );

    if ( shouldChangeBufferSize ) {
        bufferSize = AAudioStream_setBufferSizeInFrames( stream, bufferSize );
        if ( bufferSize > 0 ) {
            updateBufferSizeInFrames( bufferSize );
            if ( _inputStream != nullptr ) {
                AAudioStream_setBufferSizeInFrames( _inputStream, bufferSize );
            }
        } else {
            Debug::log( "AAudio_IO::Error setting buffer size: %s", AAudio_convertResultToText( bufferSize ));
        }
    }

    // AudioEngine's render thread active ? write output

    if ( AudioEngineProps::isRendering.load() ) {

        // is there an input stream ?

        if ( _inputStream != nullptr ) {

            // 1. stabilize input and output streams (only once)

            if ( !_stabilizedStreams && !stabilizeStreams( numFrames )) {
                return AAUDIO_CALLBACK_RESULT_CONTINUE;
            }

            // 2. when recording is active, read the stream contents

            if ( AudioEngine::recordingState.recordDeviceInput || AudioEngine::recordingState.inputToFile ) {

                // drain existing buffer contents on first write to make sure no lingering data is present

                if ( _flushInputOnCallback ) {
                    flushInputStream( audioData, numFrames );
                    _flushInputOnCallback = false;
                }

                _readInputFrames = static_cast<aaudio_result_t>( AAudioStream_read(
                    _inputStream,
                    _sampleFormat == AAUDIO_FORMAT_PCM_I16 ? ( void* ) _recordBufferI : ( void* ) _recordBuffer,
                    numFrames,
                    static_cast<int32_t>( 0 )
                ));

                if ( _readInputFrames != numFrames ) {
                    Debug::log( "AAudio_IO::AAudioStream_read() returns %d read frames instead of %d", _readInputFrames, numFrames );
                    _readInputFrames = 0;
                }
            }
        }

        // invoke the render() method of the engine to collect audio into the enqueued buffer
        // if it returns false, we can stop this stream (render thread has stopped)

        if ( !AudioEngine::render( numFrames )) {
            return AAUDIO_CALLBACK_RESULT_STOP;
        }

        // write enqueued buffer into the output buffer (both contain interleaved samples)

        int samplesToWrite = numFrames * _outputChannelCount;

        if ( _sampleFormat == AAUDIO_FORMAT_PCM_I16 ) {

            // ideally the hardware supports floating point samples, in case it is running
            // as 16-bit PCM, convert the samples provided by the engine

            auto outputBuffer = static_cast<int16_t*>( audioData );
            for ( int i = 0; i < samplesToWrite; ++i ) {
                outputBuffer[ i ] = ( int16_t ) ( _enqueuedOutputBuffer[ i ] * CONV16BIT );
            }
        } else {

            // hardware supports floating point operation, copy the buffer contents directly

            memcpy( static_cast<float*>( audioData ), _enqueuedOutputBuffer, samplesToWrite * sizeof( float ));
        }
    }

    calculateCurrentOutputLatencyMillis( stream, &currentOutputLatencyMillis_ );

    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

/**
 * enqueue a buffer (of interleaved samples) for rendering
 * this is invoked by AudioEngine::render() upon request of the dataCallback method
 */
void AAudio_IO::enqueueOutputBuffer( const float* sourceBuffer, int amountOfSamples ) {
    memcpy( _enqueuedOutputBuffer, sourceBuffer, amountOfSamples * sizeof( float ));
}

/**
 * retrieve the recorded input buffer populated by the dataCallback method
 * this is invoked by AudioEngine::render()
 */
int AAudio_IO::getEnqueuedInputBuffer( float* destinationBuffer ) {
    if ( _sampleFormat == AAUDIO_FORMAT_PCM_I16 ) {

        // ideally the hardware supports floating point samples, in case it is running
        // as 16-bit PCM, convert the samples into floating point for use in the engine

        for ( int i = 0; i < _readInputFrames; ++i ) {
            destinationBuffer[ i ] = ( float ) _recordBufferI[ i ] * ( float ) CONVMYFLT;
        }
    } else {
        memcpy( destinationBuffer, _recordBuffer, _readInputFrames * sizeof( float ));
    }
    return _readInputFrames;
}

/**
 * Calculate the current latency between writing a frame to the output stream and
 * the same frame being presented to the audio hardware.
 *
 * Here's how the calculation works:
 *
 * 1) Get the time a particular frame was presented to the audio hardware
 * @see AAudioStream_getTimestamp
 * 2) From this extrapolate the time which the *next* audio frame written to the stream
 * will be presented
 * 3) Assume that the next audio frame is written at the current time
 * 4) currentLatency = nextFramePresentationTime - nextFrameWriteTime
 *
 * @param stream The stream being written to
 * @param latencyMillis pointer to a variable to receive the latency in milliseconds between
 * writing a frame to the stream and that frame being presented to the audio hardware.
 * @return AAUDIO_OK or a negative error. It is normal to receive an error soon after a stream
 * has started because the timestamps are not yet available.
 */
aaudio_result_t AAudio_IO::calculateCurrentOutputLatencyMillis( AAudioStream* stream, double *latencyMillis ) {

    // Get the time that a known audio frame was presented for playing
    int64_t existingFrameIndex;
    int64_t existingFramePresentationTime;
    aaudio_result_t result = static_cast<aaudio_result_t>(
        AAudioStream_getTimestamp( stream,
        CLOCK_MONOTONIC,
        &existingFrameIndex,
        &existingFramePresentationTime )
    );

    if ( result == AAUDIO_OK ) {
        // Get the write index for the next audio frame
        int64_t writeIndex = AAudioStream_getFramesWritten( stream );

        // Calculate the number of frames between our known frame and the write index
        int64_t frameIndexDelta = writeIndex - existingFrameIndex;

        // Calculate the time which the next frame will be presented
        int64_t frameTimeDelta = ( frameIndexDelta * NANOS_PER_SECOND ) / _sampleRate;
        int64_t nextFramePresentationTime = existingFramePresentationTime + frameTimeDelta;

        // Assume that the next frame will be written at the current time
        int64_t nextFrameWriteTime = PerfUtility::now( CLOCK_MONOTONIC );

        // Calculate the latency
        *latencyMillis = ( double )( nextFramePresentationTime - nextFrameWriteTime ) / NANOS_PER_MILLISECOND;
    } else {
        Debug::log( "AAudio_IO::Error calculating latency: %s", AAudio_convertResultToText( result ));
    }
    return result;
}

void AAudio_IO::errorCallback( AAudioStream* stream, aaudio_result_t error ) {

    assert( stream == _outputStream || stream == _inputStream );
    Debug::log( "AAudio_IO::errorCallback result: %s", AAudio_convertResultToText( error ));

    auto streamState = static_cast<aaudio_stream_state_t>( AAudioStream_getState( _outputStream ));
    if ( streamState == AAUDIO_STREAM_STATE_DISCONNECTED ) {
        // Handle stream restart on a separate thread
        std::function<void( void )> restartStreams = std::bind( &AAudio_IO::restartStreams, this );
        _streamRestartThread = new std::thread( restartStreams );
    }
}

void AAudio_IO::closeAllStreams() {
    if ( _outputStream != nullptr ) {
        closeStream( _outputStream );
        _outputStream = nullptr;
    }
    if ( _inputStream != nullptr ) {
        closeStream( _inputStream );
        _inputStream = nullptr;
        resetStreamStabilization();
    }
}

/**
 * Drain the recording stream of any existing data by reading from it until it's empty. This is
 * usually run to clear out any stale data before performing an actual read operation, thereby
 * obtaining the most recently recorded data and the best possible recording latency.
 *
 * @param audioData A buffer which the existing data can be read into
 * @param numFrames The number of frames to read in a single read operation, this is typically the
 * size of `audioData`.
 */
void AAudio_IO::flushInputStream( void *audioData, int32_t numFrames ) {
    int32_t clearedFrames = 0;
    do {
        clearedFrames = AAudioStream_read( _inputStream, audioData, numFrames, 0 );
    } while ( clearedFrames > 0 );
}


void AAudio_IO::restartStreams() {

  Debug::log( "AAudio_IO::Restarting streams" );

    if ( _restartingLock.try_lock() ) {
        closeAllStreams();
        createAllStreams();
        _restartingLock.unlock();
    } else {
        Debug::log( "AAudio_IO::Restart stream operation already in progress - ignoring this request" );
        // We were unable to obtain the restarting lock which means the restart operation is currently
        // active. This is probably because we received successive "stream disconnected" events.
        // Internal issue b/63087953
    }
}

/**
 * Adapted from Oboe's FullDuplexStream to synchronize input and output streams.
 *
 * "Input and output have different startup times. The input side may have to charge up the microphone circuit.
 * Also the initial timing for the output callback may be bursty as it fills the buffer up.
 * So when the output stream makes its first callback, the input buffer may be overflowing or empty or partially full.
 *
 * In order to get into sync we go through a few phases.
 *
 * In Phase 1 we always drain the input buffer as much as possible, more than the output callback asks for.
 * When we have done this for a while, we move to phase 2.
 * In Phase 2 we optionally skip reading the input once to allow it to fill up with one burst.
 * This makes it less likely to underflow on future reads.
 * In Phase 3 we should be in a stable situation where the output is nearly full and the input is nearly empty.
 * You should be able to run for hours like this with no glitches."
 */
bool AAudio_IO::stabilizeStreams( int32_t numFrames ) {
    int32_t actualFramesRead = 0;

    if ( _amountOfInputCallbacksToFlush > 0 ) {
        // Debug::log("AAudio_IO::Flushing %d input callbacks", _amountOfInputCallbacksToFlush );

        int32_t totalFramesRead = 0;
        do {
            int32_t result = static_cast<aaudio_result_t>( AAudioStream_read(
                _inputStream,
                _sampleFormat == AAUDIO_FORMAT_PCM_I16 ? ( void* ) _recordBufferI : ( void* ) _recordBuffer,
                numFrames,
                static_cast<int32_t>( 0 )
            ));
            if ( !result ) {
                break; // input stream may not yet be started
            }
            actualFramesRead = result;
            totalFramesRead += actualFramesRead;
        } while ( actualFramesRead > 0 );

        // when we got actual data, treat one callback as flushed
        if ( totalFramesRead > 0 ) {
            --_amountOfInputCallbacksToFlush;
        }

    } else if ( _amountOfInputBurstsToPad > 0 ) {
        // Debug::log( "AAudio_IO::input bursts to pad: %d", _amountOfInputBurstsToPad );
        --_amountOfInputBurstsToPad;

    } else if ( _amountOfInputCallbacksToIgnore > 0 ) {
        // Debug::log("AAudio_IO::callbacks to ignore %d", _amountOfInputCallbacksToIgnore );

        int32_t result = static_cast<aaudio_result_t>( AAudioStream_read(
            _inputStream,
            _sampleFormat == AAUDIO_FORMAT_PCM_I16 ? ( void* ) _recordBufferI : ( void* ) _recordBuffer,
            numFrames,
            static_cast<int32_t>( 0 )
        ));
        if ( !result ) {
            // closeAllStreams();
            return false;
        }
        if ( --_amountOfInputCallbacksToIgnore == 0 ) {
            Debug::log( "AAudio_IO::Stabilized input and output streams" );
            _stabilizedStreams = true;
        }
    }
    return _stabilizedStreams;
}

void AAudio_IO::resetStreamStabilization() {
    _stabilizedStreams              = false;
    _amountOfInputCallbacksToFlush  = 20;
    _amountOfInputCallbacksToIgnore = 30;
    _amountOfInputBurstsToPad       = _inputBurstsToPad;
}

int AAudio_IO::getOutputLatency() {
    return ( int ) ( currentOutputLatencyMillis_ * (( double ) AudioEngineProps::SAMPLE_RATE / 1000 ));
}

void AAudio_IO::setBufferSizeInBursts( int32_t numBursts ) {
    AAudio_IO::_bufferSizeSelection = numBursts;
}

} // E.O namespace MWEngine
