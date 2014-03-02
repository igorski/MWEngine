#ifndef __UTILS_H_INCLUDED__
#define __UTILS_H_INCLUDED__

#include <math.h>
#include <sstream>

/* convenience methods */

float scale( float value, float maxValue, float maxCompareValue );
float randomfloat();
float now_ms();

/* volume util */

namespace VolumeUtil
{
    extern float FACTOR1;
    extern float FACTOR2;

    extern float lin2log( float dLinear );
    extern float log2lin( float dLogarithmic );
}

/* buffer util */

namespace BufferUtil
{
    extern float* generateSilentBuffer( int aBufferSize );
    extern int calculateBufferLength( float aMinRate );
    extern float* copyBuffer( float* aInputBuffer, int aBufferSize );
    extern void mergeBuffer( float* aOutputBuffer, float* aBufferToMerge, int aBufferSize, int aOffset );
}

/* convenience methods */

// note this will not work on pointers (yes, that means arrays passed as arguments too ;-)
#define ARRAY_SIZE( array ) ( sizeof(( array )) / sizeof(( array[ 0 ])))

// numbers to string
#define SSTR( x ) dynamic_cast< std::ostringstream & >( \
 ( std::ostringstream() << std::dec << x ) ).str()

/* logging */

#define APPNAME "SYNTH"

namespace DebugTool
{
    extern void log( char const* aMessage );
    extern void log( char const* aMessage, int aValue );
    extern void log( char const* aMessage, float aValue );
}

#endif
