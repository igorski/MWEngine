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
float randomfloat()
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
     * duplicate the contents of an existing buffer
     *
     * @param aInputBuffer {float*} the existing buffer to copy
     * @param aBufferSize {int} the length of the buffer
     */
    float* copyBuffer( float* aInputBuffer, int aBufferSize )
    {
        // TODO memory copy ?
        float* out = new float[ aBufferSize ];

        for ( int i = 0; i < aBufferSize; ++i )
            out[ i ] = aInputBuffer[ i ];

        return out;
    }

    /**
     * merges the content of aBufferToMerge with the content
     * of aInputBuffer (adds values) note that the buffer size
     * must exist for both buffers
     *
     * @param aOutputBuffer {float*} the buffer to merge into
     * @param aBufferToMerge {float*} the buffer to merge from
     * @param aBufferSize {int} length of the buffer we're duplicating into the aOutputBuffer
     * @param aOffset {int} the offset in aInputBuffer where the merged buffer will be written to
     *                      this can be 0 for equal length buffers / merging at the beginning of aOutputBuffer
     */
    void mergeBuffer( float* aOutputBuffer, float* aBufferToMerge, int aBufferSize, int aOffset )
    {
        int l = aOffset + aBufferSize;
        int j = 0;

        for ( int i = aOffset; i < l; ++i, ++j ) {
            aOutputBuffer[ i ] += aBufferToMerge[ j ];
        }
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
