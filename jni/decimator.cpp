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
#include "decimator.h"
#include <math.h>

// constructor

/**
 * @param bits {int} 1 - 32
 * @param rate {float} sample rate 0 - 1 ( 1 being equal to the original sample rate )
 */
Decimator::Decimator( int bits, float rate )
{
    setBits( bits );
    setRate( rate );

    _count = 0.0;
}

/* public methods */

void Decimator::process( AudioBuffer* sampleBuffer, bool isMonoSource )
{
    int bufferSize = sampleBuffer->bufferSize;

    for ( int c = 0, ca = sampleBuffer->amountOfChannels; c < ca; ++c )
    {
        SAMPLE_TYPE* channelBuffer = sampleBuffer->getBufferForChannel( c );

        for ( int i = 0; i < bufferSize; ++i )
        {
            float sample = channelBuffer[ i ];

            _count += _rate;

            if ( _count >= 1 )
            {
                _count -= 1;
                //sample  = ( long ) ( sample * _m ) / _m;
                sample = _m * floor( sample / _m + 0.5 );
            }
            channelBuffer[ i ] = sample;
        }
        // save CPU cycles when source is mono
        if ( isMonoSource )
        {
            sampleBuffer->applyMonoSource();
            break;
        }
    }
}

/* getters / setters */

int Decimator::getBits()
{
    return _bits;
}

void Decimator::setBits( int value )
{
    _bits = value;
    _m    = 1 << ( _bits - 1 );
}

float Decimator::getRate()
{
    return _rate;
}

void Decimator::setRate( float value )
{
    _rate = value;
}
