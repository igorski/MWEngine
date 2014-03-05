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
#include "lpfhpfilter.h"
#include "global.h"
#include <math.h>

// constructor

LPFHPFilter::LPFHPFilter( float aLPCutoff, float aHPCutoff, int amountOfChannels )
{
    setLPF( aLPCutoff, audio_engine::SAMPLE_RATE );
    setHPF( aHPCutoff, audio_engine::SAMPLE_RATE );

    lastSamples            = new float[ amountOfChannels ];
    lastUnprocessedSamples = new float[ amountOfChannels ];

    for ( int i = 0; i < amountOfChannels; ++i )
    {
        lastSamples[ i ]            = 0.0;
        lastUnprocessedSamples[ i ] = 0.0;
    }
}

LPFHPFilter::~LPFHPFilter()
{
    delete[] lastSamples;
    delete[] lastUnprocessedSamples;
}

/* public methods */

void LPFHPFilter::setLPF( float aCutOffFrequency, int aSampleRate )
{
    float w = 2.0 * aSampleRate;
    float Norm;

    aCutOffFrequency *= 2.0 * ( atan( 1 ) * 4 );
    Norm              = 1.0 / ( aCutOffFrequency + w );
    b1                = ( w - aCutOffFrequency ) * Norm;
    a0                = a1 = aCutOffFrequency * Norm;
}

void LPFHPFilter::setHPF( float aCutOffFrequency, int aSampleRate )
{
    float w = 2.0 * aSampleRate;
    float Norm;

    aCutOffFrequency *= 2.0 *( atan( 1 ) * 4 );
    Norm              = 1.0 / ( aCutOffFrequency + w );
    a0                = w * Norm;
    a1                = -a0;
    b1                = ( w - aCutOffFrequency ) * Norm;
}

void LPFHPFilter::process( AudioBuffer* sampleBuffer, bool isMonoSource )
{
    int bufferSize = sampleBuffer->bufferSize;

    for ( int i = 0, l = sampleBuffer->amountOfChannels; i < l; ++i )
    {
        SAMPLE_TYPE* channelBuffer = sampleBuffer->getBufferForChannel( i );

        for ( int j = 0; j < bufferSize; ++j )
        {
            float curUnprocessedSample = channelBuffer[ j ];

            // keep an eye on the iterator names ;)
            if ( j > 0 )
                lastSamples[ i ] = channelBuffer[ j - 1 ];

            channelBuffer[ j ]     = curUnprocessedSample * a0 + lastUnprocessedSamples[ i ] * a1 + lastSamples[ i ] * b1;
            lastUnprocessedSamples[ i ] = curUnprocessedSample;

    //            out[n] = in[n]*a0 + in[n-1]*a1 + out[n-1]*b1;
        }

        // save CPU cycles when source is mono
        if ( isMonoSource )
        {
            sampleBuffer->applyMonoSource();
            break;
        }
    }
}
