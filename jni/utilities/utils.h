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
#ifndef __UTILS_H_INCLUDED__
#define __UTILS_H_INCLUDED__

#include <math.h>
#include <sstream>
#include "global.h"

/* convenience methods */

float scale( float value, float maxValue, float maxCompareValue );
float randomFloat();
float now_ms();

/* volume util */

namespace VolumeUtil
{
    extern float FACTOR1;
    extern float FACTOR2;

    extern float lin2log( float dLinear );
    extern float log2lin( float dLogarithmic );
}

/* convenience methods */

// numbers to string
#define SSTR( x ) dynamic_cast< std::ostringstream & >( \
 ( std::ostringstream() << std::dec << x ) ).str()

/* convenience log methods */

#define LOGTAG "MWENGINE"

namespace DebugTool
{
    extern void log( char const* aMessage );
    extern void log( char const* aMessage, char const* aValue );
    extern void log( char const* aMessage, int aValue );
    extern void log( char const* aMessage, unsigned int aValue );
    extern void log( char const* aMessage, float aValue );
    extern void log( char const* aMessage, double aValue );
}

#endif
