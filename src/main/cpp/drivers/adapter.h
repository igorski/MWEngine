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
#ifndef __MWENGINE__DRIVER_ADAPTER_H_INCLUDED__
#define __MWENGINE__DRIVER_ADAPTER_H_INCLUDED__

#include "../global.h"
#include <definitions/drivers.h>
#include "aaudio_io.h"
#include "opensl_io.h"

#ifdef MOCK_ENGINE
#include "mock_io.h"
#endif

/**
 * DriverAdapter acts as a proxy for all the available driver types
 * within MWEngine (OpenSL, AAudio or mocked)
 *
 * DriverAdapter will maintain its own references to the driver instances
 * AudioEngine will operate via the DriverAdapter
 */
namespace MWEngine {
namespace DriverAdapter {

    bool create( Drivers::types driver );
    void destroy();

    bool isAAudio(); // TODO: no actor in the engine should care about this.
    bool isMocked(); // TODO: no actor in the engine should care about this.

    /* internal methods */

    // these are invoked during the engines render cycle and should not be called directly

    void render();

    // writes the contents of given outputBuffer into the drivers output
    // so we can hear sound. outputBuffer contains interleaved samples

    void writeOutput( float* outputBuffer, int amountOfSamples );

    // gets the input buffer from the driver (when recording)
    // and writes it into given recordBuffer
    // returns integer value of amount of recorded samples

    int getInput( float* recordBuffer, int amountOfSamples );


    /* internal variables */

    extern Drivers::types _driver;
    extern OPENSL_STREAM* driver_openSL;
    extern AAudio_IO*     driver_aAudio;

#ifdef MOCK_ENGINE
    extern Mock_IO*       driver_mocked;
#endif
}
} // E.O namespace MWEngine


#endif
