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
#include <android/log.h>
#include <time.h>
#include <string.h>
#include <cstdlib>

/* convenience methods */

/**
 * @method scale
 * scales a value against a scale different to the one "value" stems from
 * i.e. a UI slider with a value 0 - 100 ( percent ) to match against a
 * scale with a maximum value of 255
 *
 * @param value           {float} value to get scaled to
 * @param maxValue        {float} the maximum value we are likely to expect for param value
 * @param maxCompareValue {float} the maximum value in the scale we're matching against
 *
 * @return float
 */
float scale( float value, float maxValue, float maxCompareValue )
{
    float ratio = maxCompareValue / maxValue;
    return value * ratio;
}

//
// Generate a random number between 0 and 1
// return a uniform number in [0,1].
float randomFloat()
{
    return rand() / float( RAND_MAX );
}

float now_ms()
{
    struct timespec res;
    clock_gettime( CLOCK_REALTIME, &res );
    return 1000.0 * res.tv_sec + ( float ) res.tv_nsec / 1e6;
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

/* buffer util */

namespace BufferUtil
{
    float* generateSilentBuffer( int aBufferSize )
    {
        float* out = new float[ aBufferSize ];

        for ( int i = 0; i < aBufferSize; ++i )
            out[ i ] = 0.0;

        return out;
    }

    /**
     * calculate the amount of samples a single cycle of a waveform
     * will hold at a give rate in Hz, at the current sample rate
     *
     * @param aMinRate {float} length in Hz for the waveform
     */
    int calculateBufferLength( float aMinRate )
    {
        float phaseStep = aMinRate / audio_engine::SAMPLE_RATE;
        return ceil( 1.0 / phaseStep );
    }

    /**
     * calculate the amount of samples necessary for
     * writing the given amount in milliseconds
     */
    int calculateBufferLength( int milliSeconds )
    {
        return milliSeconds * ( audio_engine::SAMPLE_RATE * .001 );
    }
}

/* logging */

namespace DebugTool
{
    /**
     * send a debug message to the Android logcat
     * @param aMessage {char} message string
     */
    void log( char const* aMessage )
    {
        __android_log_print( ANDROID_LOG_VERBOSE, APPNAME, "%s", aMessage );
    }

    /**
     * same as above, but traces contents of an char const*
     * @param aValue {char const*} optional numerical value
     *
     * to trace char values pass "%s" in aMessage to show aValue
     */
    void log( char const* aMessage, char const* aValue )
    {
        __android_log_print( ANDROID_LOG_VERBOSE, APPNAME, aMessage, aValue );
    }
    /**
     * same as above, but traces contents of an int
     * @param aValue {int} optional numerical value
     *
     * to trace numerical values pass "%d" in aMessage to show aValue
     */
    void log( char const* aMessage, int aValue )
    {
        __android_log_print( ANDROID_LOG_VERBOSE, APPNAME, aMessage, aValue );
    }

    /**
     * same as above, but traces contents of a float value
     * @param aValue {float} optional numerical value
     *
     * to trace float values pass "%f" in aMessage to show aValue
     */
    void log( char const* aMessage, float aValue )
    {
        __android_log_print( ANDROID_LOG_VERBOSE, APPNAME, aMessage, aValue );
    }
}
