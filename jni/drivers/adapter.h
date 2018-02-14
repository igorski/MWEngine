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
#ifndef __DRIVER_H_INCLUDED__
#define __DRIVER_H_INCLUDED__

#include "../global.h"

/**
 * DriverAdapter acts as a proxy for all the available driver types
 * within MWEngine (OpenSL, AAudio or mocked OpenSL)
 *
 * DriverAdapter will maintain its own references to the driver instances
 * AudioEngine will operate via the DriverAdapter
 */
namespace DriverAdapter {

    bool create();
    void destroy();

    // start the render loop
    void render();

    /* internal methods */

    // these are invoked during the render cycle and should not be called directly

    // write the contents of given outputBuffer into the drivers output
    // so we can hear sound. outputBuffer contains interleaved samples
    void writeOutput( float *outputBuffer, int amountOfSamples );

    // get the input buffer from the driver (when recording)
    // and write it into given recordBuffer
    // returns integer value of amount of recorded samples
    int getInput( float* recordBuffer );
}

// whether to include the OpenSL, AAudio or mocked (unit test mode) driver for audio output

#if DRIVER == 0

// OpenSL

#ifdef MOCK_ENGINE
// mocking requested, e.g. unit test mode
#include "../tests/helpers/mock_opensl_io.h"

#else
// production build for OpenSL
#include "opensl_io.h"

#endif

extern OPENSL_STREAM* driver_openSL;

#elif DRIVER == 1

// production build for AAudio
#include "aaudio_io.h"
extern AAudio_IO* driver_aAudio;

#endif

#endif
