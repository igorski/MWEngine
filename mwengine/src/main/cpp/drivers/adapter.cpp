/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2017-2022 Igor Zinken - https://www.igorski.nl
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

    bool create( Drivers::types driver ) {

        destroy(); // destroy existing drivers

        if ( driver == Drivers::AAUDIO && !AAudio_IO::isSupported() ) {
            driver = Drivers::OPENSL; // fall back to OpenSL when AAudio isn't supported
        }

        _driver = driver;
        int bursts;

        switch ( _driver ) {

            default:
                return false;

            case Drivers::OPENSL:

                Debug::log( "DriverAdapter::initializing OpenSL driver" );

                driver_openSL = android_OpenAudioDevice(
                    AudioEngineProps::SAMPLE_RATE,     AudioEngineProps::INPUT_CHANNELS,
                    AudioEngineProps::OUTPUT_CHANNELS, AudioEngineProps::BUFFER_SIZE
                );
                return driver_openSL != nullptr;

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

                bursts = 0; // Google provides {0, 1, 2, 4, 8} as values

#if defined(__arm__)
    // Cortex-A53-derivatives (SC9863A|Cortex-A55) are NOT running 64-bit (__aarch64__) and struggle severely
    if ( AudioEngineProps::CPU_CORES.size() == 0 ) {
        Debug::log( "DriverAdapter::32-bit ARM without exclusive cores detected, increasing bursts for increased playback stability" );
        bursts = 2;
    }
#endif
                driver_aAudio->setBufferSizeInBursts( bursts );

                return true;

#ifdef MOCK_ENGINE

            case Drivers::MOCKED:

                Debug::log( "DriverAdapter::initializing mocked driver" );
                driver_mocked = new Mock_IO();

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
    }

    bool isAAudio() {
        return _driver == Drivers::AAUDIO;
    }

    bool isMocked() {
        return _driver == Drivers::MOCKED;
    }

    /* internal methods */

    void startRender() {
        switch ( _driver ) {
            default:
                return;
            case Drivers::OPENSL:
#ifdef MOCK_ENGINE
            case Drivers::MOCKED:
#endif
                // OpenSL maintains its own locking mechanism, as such we create a daemon
                // in which rendering is requested for as long as the thread is active

                while ( AudioEngineProps::isRendering.load() ) {
                    AudioEngine::render( AudioEngineProps::BUFFER_SIZE );
                }
                break;
            case Drivers::AAUDIO:
                // AAudio is callback based and will invoke AudioEngine::render() internally
                // for as long as isRendering is set to true
                break;
        }
    }

    void writeOutput( float* outputBuffer, int amountOfSamples )
    {
        switch ( _driver ) {
            default:
                return;
#ifdef MOCK_ENGINE
            case Drivers::MOCKED:
                driver_mocked->writeOutput( outputBuffer, amountOfSamples );
                break;
#endif
            case Drivers::OPENSL:
                android_AudioOut( driver_openSL, outputBuffer, amountOfSamples );
                break;
            case Drivers::AAUDIO:
                driver_aAudio->enqueueOutputBuffer( outputBuffer, amountOfSamples );
                break;
        }
    }

    int getInput( float* recordBuffer, int amountOfSamples )
    {
        switch ( _driver ) {
            default:
#ifdef MOCK_ENGINE
            case Drivers::MOCKED:
#endif
                return 0;
            case Drivers::OPENSL:
                return android_AudioIn( driver_openSL, recordBuffer, amountOfSamples );
            case Drivers::AAUDIO:
                return driver_aAudio->getEnqueuedInputBuffer( recordBuffer );
        }
    }

    int getLatency()
    {
        switch ( _driver ) {
            default:
#ifdef MOCK_ENGINE
            case Drivers::MOCKED:
#endif
                return 0;
            case Drivers::OPENSL:
                return 0; // TODO (if we care...) how to calculate latency using OpenSL driver
            case Drivers::AAUDIO:
                return driver_aAudio->getOutputLatency();
        }
    }
}

} // E.O namespace MWEngine
