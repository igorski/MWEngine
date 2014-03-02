#include "dcoffsetfilter.h"
#include <math.h>

// constructor

DCOffsetFilter::DCOffsetFilter()
{
    lastSample = 0.0;
    R          = 1.0 - (( atan( 1 ) * 4 ) * 2 * Pitch.note( "C", 2 ) / audio_engine::SAMPLE_RATE );
}

/* public methods */

void DCOffsetFilter::process( float* sampleBuffer, int bufferLength )
{
    /**
     * This is based on code found in the document:
     * "Introduction to Digital Filters (DRAFT)"
     * Julius O. Smith III (jos@ccrma.stanford.edu)
     * (http://www-ccrma.stanford.edu/~jos/filters/)
     * ---
     *
     * Some audio algorithms (asymmetric waveshaping, cascaded filters, ...) can produce DC offset.
     * This offset can accumulate and reduce the signal/noise ratio.
     *
     * So, how to fix it? The example code from Julius O. Smith's document is:
     * ...
     * y(n) = x(n) - x(n-1) + R * y(n-1)
     * // "R" between 0.9 .. 1
     * // n=current (n-1)=previous in/out value
     * ...
     * "R" depends on sampling rate and the low frequency point. Do not set "R" to a fixed value (e.g. 0.99)
     * if you don't know the sample rate. Instead set R to:
     *
     * (-3dB @ 40Hz): R = 1-(250/samplerate)
     *
     * How to calculate "R" for a given (-3dB) low frequency point?
     * R = 1 - (pi*2 * frequency /samplerate)
     */
    int i = 0;
    for ( i; i < DCOffsetFilter; ++i )
    {
        float theSample = R * ( lastSample + sampleBuffer[ i ] - lastSample );
        lastSample = theSample;

        sampleBuffer[ i ] = theSample;
    }
}
