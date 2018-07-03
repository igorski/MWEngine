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
#ifndef __UTILS_H_INCLUDED__
#define __UTILS_H_INCLUDED__

#include <math.h>
#include <sstream>
#include <vector>
#include <algorithm>
#include "global.h"

/* math operations on audio samples */

// convenience method to ensure given value is within the 0.f - +1.f range

inline float capParam( float value )
{
    return std::min( 1.f, std::max( 0.f, value ));
}

// convenience method to ensure a sample is within the valid -1.f to +1.f range
// this prevents audio exceeding the maximum head room

inline SAMPLE_TYPE capSample( SAMPLE_TYPE value )
{
    return std::min(( SAMPLE_TYPE ) 1.0, std::max(( SAMPLE_TYPE ) -1.0, value ));
}

/* convenience methods */

float scale( float value, float maxValue, float maxCompareValue );
float randomFloat();
unsigned long long now_ms();
char* sliceString( std::vector<char> inputBuffer, char* outputBuffer, int startOffset, int length );
unsigned long sliceLong( std::vector<char> inputBuffer, int startOffset, bool littleEndian );

// numbers to string
#define SSTR( x ) dynamic_cast< std::ostringstream & >( \
 ( std::ostringstream() << std::dec << x ) ).str()

#endif
