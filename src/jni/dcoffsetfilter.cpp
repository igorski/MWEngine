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
