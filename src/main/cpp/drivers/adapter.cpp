/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2017-2020 Igor Zinken - https://www.igorski.nl
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
#include "adapter.h"
#include "../audioengine.h"
#include <utilities/debug.h>

namespace MWEngine {
namespace DriverAdapter {

    Drivers::types _driver       = Drivers::types::OPENSL;

    AAudio_IO* driver_aAudio     = nullptr;
    OPENSL_STREAM* driver_openSL = nullptr;
#ifdef MOCK_ENGINE
    Mock_IO*       driver_mocked = nullptr;
#endif

    MWEngine_renderCallback render      = nullptr;
    MWEngine_writeCallback  writeOutput = nullptr;
    MWEngine_readCallback   getInput    = nullptr;

    bool create( Drivers::types driver ) {

        destroy(); // destroy existing drivers

        _driver = driver;

        switch ( _driver ) {
            default:
                return false;

            case Drivers::OPENSL:

                Debug::log( "DriverAdapter::initializing OpenSL driver" );

                driver_openSL = android_OpenAudioDevice(
                    AudioEngineProps::SAMPLE_RATE,     AudioEngineProps::INPUT_CHANNELS,
                    AudioEngineProps::OUTPUT_CHANNELS, AudioEngineProps::BUFFER_SIZE
                );
                if ( driver_openSL == nullptr ) {
                    return false;
                }
                render = []() {
                    // OpenSL maintains its own locking mechanism, we can invoke
                    // the render cycle directly from the audio engine thread loop
                    AudioEngine::render( AudioEngineProps::BUFFER_SIZE );
                };
                writeOutput = []( float* outputBuffer, int amountOfSamples ) {
                    android_AudioOut( driver_openSL, outputBuffer, amountOfSamples );
                };
                getInput = []( float* recordBuffer, int amountOfSamples ) {
                    return android_AudioIn( driver_openSL, recordBuffer, AudioEngineProps::BUFFER_SIZE );
                };
                return true;

            case Drivers::AAUDIO:

                Debug::log( "DriverAdapter::initializing AAudio driver");

                driver_aAudio = new AAudio_IO(
                    AudioEngineProps::INPUT_CHANNELS, AudioEngineProps::OUTPUT_CHANNELS
                );

                if ( driver_aAudio == nullptr ) {
                    return false;
                }
                // TODO: allow specifying these from the outside?
                // driver_aAudio->setDeviceId();
                // driver_aAudio->setRecordingDeviceId();
                driver_aAudio->setBufferSizeInBursts( 1 ); // Google provides {0, 1, 2, 4, 8} as values

                render = []() {
                    // AAudio triggers its callback internally when ready
                    // AAudio driver will request render() on its own
                    driver_aAudio->render = true;
                };
                writeOutput = []( float* outputBuffer, int amountOfSamples ) {
                    driver_aAudio->enqueueOutputBuffer( outputBuffer, amountOfSamples );
                };
                getInput = []( float* recordBuffer, int amountOfSamples ) {
                    return driver_aAudio->getEnqueuedInputBuffer( recordBuffer, amountOfSamples );
                };

                return true;

#ifdef MOCK_ENGINE

            case Drivers::MOCKED:

                Debug::log( "DriverAdapter::initializing mocked driver");

                driver_mocked = new Mock_IO();

                render = []() {
                    AudioEngine::render( AudioEngineProps::BUFFER_SIZE );
                };
                writeOutput = []( float* outputBuffer, int amountOfSamples ) {
                    driver_mocked->writeOutput( outputBuffer, amountOfSamples );
                };
                getInput = []( float* recordBuffer, int amountOfSamples ) {
                    return 0;
                };

                return true;
#endif
        }
    }

    void destroy() {

        if ( driver_openSL != nullptr ) {
            android_CloseAudioDevice( driver_openSL );
        }
        driver_openSL = nullptr;

        delete driver_aAudio;
        driver_aAudio = nullptr;

#ifdef MOCK_ENGINE
        delete driver_mocked;
        driver_mocked = nullptr;
#endif
        render      = nullptr;
        writeOutput = nullptr;
        getInput    = nullptr;
    }

    bool isAAudio() {
        return _driver == Drivers::types::AAUDIO;
    }

    bool isMocked() {
        return _driver == Drivers::types::MOCKED;
    }
}

} // E.O namespace MWEngine
