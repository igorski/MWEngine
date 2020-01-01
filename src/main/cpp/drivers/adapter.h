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

    // prototypes for the callback mechanisms

    typedef void( *MWEngine_renderCallback )();
    typedef void( *MWEngine_writeCallback )( float* outputBuffer, int amountOfSamples );
    typedef int ( *MWEngine_readCallback ) ( float* recordBuffer, int amountOfSamples );

    bool create( Drivers::types driver );
    void destroy();

    bool isAAudio(); // TODO: no actor in the engine should care about this.
    bool isMocked(); // TODO: no actor in the engine should care about this.

    // start the render loop
    extern MWEngine_renderCallback render;

    /* internal methods */

    // these are invoked during the render cycle and should not be called directly

    // write the contents of given outputBuffer into the drivers output
    // so we can hear sound. outputBuffer contains interleaved samples
    extern MWEngine_writeCallback writeOutput;

    // get the input buffer from the driver (when recording)
    // and write it into given recordBuffer
    // returns integer value of amount of recorded samples
    extern MWEngine_readCallback getInput;

    extern Drivers::types _driver;
    extern OPENSL_STREAM* driver_openSL;
    extern AAudio_IO*     driver_aAudio;

#ifdef MOCK_ENGINE
    extern Mock_IO*       driver_mocked;
#endif
}
} // E.O namespace MWEngine


#endif
