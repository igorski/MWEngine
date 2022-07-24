/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 Igor Zinken - http://www.igorski.nl
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
#include "gain.h"
#include <utilities/utils.h>
#include <cmath>

namespace MWEngine {

/* constructor / destructor */

Gain::Gain()
{
    setAmount( MAX_VOLUME );
}

Gain::Gain( float amount )
{
    setAmount( amount );
}

Gain::~Gain()
{
    // nowt...
}

/* public methods */

void Gain::process( AudioBuffer* sampleBuffer, bool isMonoSource )
{
    // no gain specified ? do nothing
    if ( _amount == MAX_VOLUME )
        return;

    int bufferSize = sampleBuffer->bufferSize;

    for ( int i = 0, l = sampleBuffer->amountOfChannels; i < l; ++i )
    {
        SAMPLE_TYPE* channelBuffer = sampleBuffer->getBufferForChannel( i );

        for ( int j = 0; j < bufferSize; ++j )
        {
            channelBuffer[ j ] = capSampleSafe( channelBuffer[ j ] * _amount );
        }

        // omit unnecessary cycles by copying the mono content
        if ( isMonoSource )
        {
            sampleBuffer->applyMonoSource();
            break;
        }
    }
}

bool Gain::isCacheable()
{
    return true;
}

/* getters / setters */

float Gain::getAmount()
{
    return ( float ) _amount;
}

void Gain::setAmount( float value )
{
    _amount = std::max( MIN_GAIN, std::min( MAX_GAIN, ( SAMPLE_TYPE ) value ));
}

} // E.O namespace MWEngine
