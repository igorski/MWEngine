/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Igor Zinken - http://www.igorski.nl
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
#include "levelutility.h"
#include <math.h>

/* public methods */

SAMPLE_TYPE LevelUtility::RMS( AudioChannel* audioChannel, int channelNum )
{
    AudioBuffer* audioBuffer = audioChannel->getOutputBuffer();
    SAMPLE_TYPE out          = 0.0;
    SAMPLE_TYPE* buffer      = audioBuffer->getBufferForChannel( channelNum );
    SAMPLE_TYPE sample;

    for ( int i = 0, l = audioBuffer->bufferSize; i < l; ++i )
    {
        sample = buffer[ i ];
        out   += sample * sample;
    }
    out = out / ( SAMPLE_TYPE ) audioBuffer->bufferSize;
    return sqrt( out );
}

SAMPLE_TYPE LevelUtility::dBSPL( AudioChannel* audioChannel, int channelNum )
{
    SAMPLE_TYPE value = pow( linear( audioChannel, channelNum ), 0.5 );
    value = value / audioChannel->getOutputBuffer()->bufferSize;

    return 20.0 * log10( value );
}

SAMPLE_TYPE LevelUtility::linear( AudioChannel* audioChannel, int channelNum )
{
    AudioBuffer* audioBuffer = audioChannel->getOutputBuffer();
    SAMPLE_TYPE out          = 0.0;
    SAMPLE_TYPE* buffer      = audioBuffer->getBufferForChannel( channelNum );
    SAMPLE_TYPE sample;

    for ( int i = 0, l = audioBuffer->bufferSize; i < l; ++i )
    {
        sample = buffer[ i ];
        out   += sample * sample;
    }
    return out;
}
