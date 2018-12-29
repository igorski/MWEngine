/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2016-2018 Igor Zinken - http://www.igorski.nl
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
#include "lowpassfilter.h"
#include "../global.h"

namespace MWEngine {

/* constructor / destructor */

LowPassFilter::LowPassFilter( float cutoff )
{
    setCutoff( cutoff );
}

LowPassFilter::~LowPassFilter()
{

}

/* public methods */

float LowPassFilter::getCutoff()
{
    return _cutoff;
}

void LowPassFilter::setCutoff( float value )
{
    _cutoff = value;

    SAMPLE_TYPE Q = 1.1f;
    w0 = TWO_PI * _cutoff / ( SAMPLE_TYPE ) AudioEngineProps::SAMPLE_RATE;
    alpha = sin(w0) / (2.0 * Q);
    b0 =  (1.0 - cos(w0))/2;
    b1 =   1.0 - cos(w0);
    b2 =  (1.0 - cos(w0))/2;
    a0 =   1.0 + alpha;
    a1 =  -2.0 * cos(w0);
    a2 = 1.0 - alpha;
    x1 = x2 = y1 = y2 = 0;
}

void LowPassFilter::process( AudioBuffer* sampleBuffer, bool isMonoSource )
{
    int bufferSize = sampleBuffer->bufferSize;

    for ( int i = 0, l = sampleBuffer->amountOfChannels; i < l; ++i )
    {
        SAMPLE_TYPE* channelBuffer = sampleBuffer->getBufferForChannel( i );

        // store current values for this channel
        SAMPLE_TYPE orgx1 = x1,
                    orgx2 = x2,
                    orgy1 = y1,
                    orgy2 = y2;

        for ( int j = 0; j < bufferSize; ++j )
            channelBuffer[ j ] = processSingle( channelBuffer[ j ] );

        // omit unnecessary cycles by copying the mono content
        if ( isMonoSource )
        {
            sampleBuffer->applyMonoSource();
            break;
        }

        // restore values for next channel

        x1 = orgx1;
        x2 = orgx2;
        y1 = orgy1;
        y2 = orgy2;
    }
}

void LowPassFilter::store()
{
    orgx1 = x1;
    orgx2 = x2;
    orgy1 = y1;
    orgy2 = y2;
}

void LowPassFilter::restore()
{
    x1 = orgx1;
    x2 = orgx2;
    y1 = orgy1;
    y2 = orgy2;
}

} // E.O namespace MWEngine
