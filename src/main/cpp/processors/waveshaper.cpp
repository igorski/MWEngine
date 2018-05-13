/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2015 Igor Zinken - http://www.igorski.nl
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
#include "waveshaper.h"
#include <cmath>

// constructor

WaveShaper::WaveShaper( float amount, float level )
{
    setAmount( amount );
    setLevel ( level );
}

/* public methods */

void WaveShaper::process( AudioBuffer* sampleBuffer, bool isMonoSource )
{
    int bufferSize = sampleBuffer->bufferSize;

    for ( int i = 0, l = sampleBuffer->amountOfChannels; i < l; ++i )
    {
        SAMPLE_TYPE* channelBuffer = sampleBuffer->getBufferForChannel( i );

        for ( int j = 0; j < bufferSize; ++j )
        {
            SAMPLE_TYPE input = channelBuffer[ j ];
            channelBuffer[ j ] = (( MAX_PHASE + _multiplier ) * input / ( MAX_PHASE + _multiplier * std::abs( input ))) * _level;
        }

        // omit unnecessary cycles by copying the mono content
        if ( isMonoSource )
        {
            sampleBuffer->applyMonoSource();
            break;
        }
    }
}

/* getters / setters */

float WaveShaper::getAmount()
{
    return _amount;
}

void WaveShaper::setAmount( float value )
{
    // keep within range

    if ( value <= -MAX_PHASE )
        value = -( MAX_PHASE - .1f );

    else if ( value >= MAX_PHASE )
        value = MAX_PHASE - .1f;

    _amount = value;

    _multiplier = ( MAX_PHASE * 2.0 ) * _amount / ( MAX_PHASE - _amount );
}

float WaveShaper::getLevel()
{
    return _level;
}

void WaveShaper::setLevel( float value )
{
    _level = value;
}
