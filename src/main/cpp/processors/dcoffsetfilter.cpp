/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2014 Igor Zinken - http://www.igorski.nl
 *
 * DCOffsetFilter is based on work by Julius O. Smith III (jos@ccrma.stanford.edu)
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

/* constructor / destructor */

DCOffsetFilter::DCOffsetFilter( int amountOfChannels )
{
    _lastInSamples  = new SAMPLE_TYPE[ amountOfChannels ];
    _lastOutSamples = new SAMPLE_TYPE[ amountOfChannels ];

    for ( int i = 0; i < amountOfChannels; ++i )
    {
        _lastInSamples [ i ] = 0.0;
        _lastOutSamples[ i ] = 0.0;
    }
    SAMPLE_TYPE baseFrequency = 65.41; // is a C2 note
    R = MAX_PHASE - ( TWO_PI * baseFrequency / AudioEngineProps::SAMPLE_RATE );
}

DCOffsetFilter::~DCOffsetFilter()
{
    delete[] _lastInSamples;
    delete[] _lastOutSamples;
}

/* public methods */

void DCOffsetFilter::process( AudioBuffer* sampleBuffer, bool isMonoSource )
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
    int bufferSize = sampleBuffer->bufferSize;

    for ( int c = 0, ca = sampleBuffer->amountOfChannels; c < ca; ++c )
    {
        SAMPLE_TYPE* channelBuffer = sampleBuffer->getBufferForChannel( c );
        SAMPLE_TYPE lastInSample   = _lastInSamples [ c ];
        SAMPLE_TYPE lastOutSample  = _lastOutSamples[ c ];

        for ( int i = 0; i < bufferSize; ++i )
        {
            SAMPLE_TYPE outSample = channelBuffer[ i ] - lastInSample + R * lastOutSample;
            lastInSample          = channelBuffer[ i ]; // cache last input sample
            lastOutSample         =                     // cache last output sample
            channelBuffer[ i ]    = outSample;          // write filtered sample into output buffer
        }

        _lastInSamples [ c ] = lastInSample;
        _lastOutSamples[ c ] = lastOutSample;

        // save CPU cycles when source is mono
        if ( isMonoSource )
        {
            sampleBuffer->applyMonoSource();
            break;
        }
    }
}
