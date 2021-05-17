/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2021 Igor Zinken - https://www.igorski.nl
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
#ifndef __MWENGINE__UTILS_H_INCLUDED__
#define __MWENGINE__UTILS_H_INCLUDED__

#include <math.h>
#include <vector>
#include <algorithm>
#include "global.h"

namespace MWEngine {

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

// inverts a pow operation, allowing you to derive the exponent from the known value and base

inline float inversePow( float value, float base )
{
    return log( value ) / log( base );
}

// inverts a log operation, allowing you to derive the original number from the known value and base
// e.g. reverting log10( 0.5 ) is inverseLog( -0.3010299956639812, 10 ) == 0.5

inline float inverseLog( float value, float base )
{
    return pow( base, value );
}

/* convenience methods */

float scale( float value, float maxValue, float maxCompareValue );
int roundTo( int value, int multipleOf );
float randomFloat();
char* sliceString( const std::vector<char>& inputBuffer, char* outputBuffer, int startOffset, int length );
unsigned long sliceLong( const std::vector<char>& inputBuffer, int startOffset, bool littleEndian );

} // E.O namespace MWEngine

#endif
