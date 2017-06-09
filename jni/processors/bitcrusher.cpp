/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2017 Igor Zinken - http://www.igorski.nl
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
#include "bitcrusher.h"
#include "../global.h"
#include "../utilities/utils.h"
#include <limits.h>

/* constructor */

BitCrusher::BitCrusher( float amount, float inputMix, float outputMix )
{
    setAmount   ( amount );
    setInputMix ( inputMix );
    setOutputMix( outputMix );
}

/* public methods */

void BitCrusher::process( AudioBuffer* sampleBuffer, bool isMonoSource )
{
    int bufferSize = sampleBuffer->bufferSize;
    int bits = _bits + 1;

    for ( int i = 0, l = sampleBuffer->amountOfChannels; i < l; ++i )
    {
        SAMPLE_TYPE* channelBuffer = sampleBuffer->getBufferForChannel( i );

        for ( int j = 0; j < bufferSize; ++j )
        {
            short input = ( short ) (( channelBuffer[ j ] * _inputMix ) * SHRT_MAX );
            short prevent_offset = ( short )( -1 >> bits );
            input &= ( -1 << ( 16 - _bits ));
            channelBuffer[ j ] = (( input + prevent_offset ) * _outputMix ) / SHRT_MAX;
        }

        // omit unnecessary cycles by copying the mono content
        if ( isMonoSource )
        {
            sampleBuffer->applyMonoSource();
            break;
        }
    }
}

bool BitCrusher::isCacheable()
{
    return true;
}

/* getters / setters */

float BitCrusher::getAmount()
{
    return _amount;
}

void BitCrusher::setAmount( float value )
{
    _amount = value;

    // scale float to 1 - 16 bit range
    _bits = ( int ) floor( scale( value, 1, 15 )) + 1;
}

float BitCrusher::getInputMix()
{
    return _inputMix;
}

void BitCrusher::setInputMix( float value )
{
    _inputMix = value;
}

float BitCrusher::getOutputMix()
{
    return _outputMix;
}

void BitCrusher::setOutputMix( float value )
{
    _outputMix = value;
}
