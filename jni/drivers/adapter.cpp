/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Igor Zinken - http://www.igorski.nl
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

#if DRIVER == 0
OPENSL_STREAM* driver_openSL = 0; // OpenSL
#elif DRIVER == 1
// AAudio
#endif

namespace DriverAdapter {

    bool create() {

#if DRIVER == 0

        // OpenSL
        driver_openSL = android_OpenAudioDevice(
            AudioEngineProps::SAMPLE_RATE, AudioEngineProps::INPUT_CHANNELS,
            AudioEngineProps::OUTPUT_CHANNELS, AudioEngineProps::BUFFER_SIZE
        );
        return ( driver_openSL != NULL );

#endif
    }

    void destroy() {

#if DRIVER == 0
        // OpenSL
        android_CloseAudioDevice( driver_openSL );
        delete driver_openSL;
        driver_openSL = 0;
#endif

    }

    void writeOutput( float* outputBuffer, int amountOfSamples ) {

#if DRIVER == 0
        // OpenSL
        android_AudioOut( driver_openSL, outputBuffer, amountOfSamples );
#endif

    }

    int getInput( float* recordBuffer ) {

#if DRIVER == 0
        // OpenSL
        return android_AudioIn( driver_openSL, recordBuffer, AudioEngineProps::BUFFER_SIZE );
#endif
        // TODO: no AAudio recording yet
    }
}
