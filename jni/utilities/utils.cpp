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
#include "utils.h"
#include "global.h"
#include <time.h>
#include <ctime>
#include <string.h>
#include <cstdlib>

/* convenience methods */

/**
 * @method scale
 * scales a value against a scale different to the one "value" stems from
 * i.e. a UI slider with a value 0 - 100 ( percent ) to match against an
 * 8-bit scale with a maximum value of 255
 *
 * @param value           {float} value to scale
 * @param maxValue        {float} the maximum value we are likely to expect for param value
 * @param maxCompareValue {float} the maximum value in the scale we're matching against
 *
 * @return float
 */
float scale( float value, float maxValue, float maxCompareValue )
{
    double ratio = maxCompareValue / maxValue;
    return ( float ) ( value * ratio );
}

/**
 * keep given value within the accepted
 * phase range this prevents clipped output
 */
SAMPLE_TYPE cap( SAMPLE_TYPE value )
{
    if ( value < -MAX_PHASE )
        return -MAX_PHASE;
    else if ( value > MAX_PHASE )
        return MAX_PHASE;

    return value;
}

//
// Generate a random number between 0 and 1
// return a uniform number in [0,1].
float randomFloat()
{
    return rand() / float( RAND_MAX );
}

/**
 * get the current time (since the epoch) in
 * milliseconds, can be used for benchmarking purposes
 */
unsigned long long now_ms()
{
    struct timeval tv;

    gettimeofday( &tv, NULL );

    unsigned long long ret = tv.tv_usec;

    ret /= 1000;                 // micro seconds to milliseconds
    ret += ( tv.tv_sec * 1000 ); // add the seconds after millisecond conversion

    return ret;
}

/* volume util */

namespace VolumeUtil
{
    float FACTOR1 = 20.0 / log( 10.0 );
    float FACTOR2 = 1 / 20.0;

    float lin2log( float dLinear )
    {
        return VolumeUtil::FACTOR1 * log( dLinear );
    }
    float log2lin( float dLogarithmic )
    {
        return pow( 10.0, dLogarithmic * VolumeUtil::FACTOR2 );
    }
}
