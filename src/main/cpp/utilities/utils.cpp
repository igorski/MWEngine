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
#include "utils.h"
#include "global.h"
#include <time.h>
#include <ctime>
#include <string.h>
#include <stdlib.h>

/* convenience methods */

/**
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
 * rounds given value to the nearest multiple of given multipleOf
 * should only be used on positive numbers
 */
int roundTo( int value, int multipleOf )
{
    if ( value < multipleOf )
        return multipleOf;

    return ( int )( multipleOf * round( value / multipleOf ));
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

/**
 * slice the contents of given inputBuffer in the range defined by startOffset to
 * startOffset + length and append it to given outputBuffer
 */
char* sliceString( std::vector<char> inputBuffer, char* outputBuffer, int startOffset, int length )
{
    for ( int i = startOffset, l = startOffset + length, w = 0; i < l; ++i, ++w )
        outputBuffer[ w ] = inputBuffer[ i ];

    return outputBuffer;
}

/**
 * note: unsigned long has size of 4 so we read the next 4 bytes (in reverse) from
 * given startOffset to calculate the corresponding unsigned long value
 * littleEndian defines in what order the bytes are arranged
 */
unsigned long sliceLong( std::vector<char> inputBuffer, int startOffset, bool littleEndian )
{
    if ( littleEndian ) {
        return inputBuffer[ startOffset + 0 ] |
               ( inputBuffer[ startOffset + 1 ] << 8  ) |
               ( inputBuffer[ startOffset + 2 ] << 16 ) |
               ( inputBuffer[ startOffset + 3 ] << 24 );
    }
    return inputBuffer[ startOffset + 3 ] << 24 ||
           inputBuffer[ startOffset + 2 ] << 16 ||
           inputBuffer[ startOffset + 1 ] <<  8 ||
           inputBuffer[ startOffset + 0 ];
}
