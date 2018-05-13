/**
 * The MIT License (MIT)
 *
 * adapted from musicdsp.org-post by mistert@inwind.it
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
#include "../global.h"
#include <math.h>

// constructor

LPFHPFilter::LPFHPFilter( float aLPCutoff, float aHPCutoff, int amountOfChannels )
{
    setLPF( aLPCutoff, AudioEngineProps::SAMPLE_RATE );
    setHPF( aHPCutoff, AudioEngineProps::SAMPLE_RATE );

    outSamples = new SAMPLE_TYPE[ amountOfChannels ];
    inSamples  = new SAMPLE_TYPE[ amountOfChannels ];

    for ( int i = 0; i < amountOfChannels; ++i )
    {
        outSamples[ i ] = 0.0;
        inSamples[ i ]  = 0.0;
    }
}

LPFHPFilter::~LPFHPFilter()
{
    delete[] outSamples;
    delete[] inSamples;
}

/* public methods */

void LPFHPFilter::setLPF( float aCutOffFrequency, int aSampleRate )
{
    SAMPLE_TYPE w = 2.0 * aSampleRate;
    SAMPLE_TYPE Norm;

    aCutOffFrequency *= TWO_PI;
    Norm              = 1.0 / ( aCutOffFrequency + w );
    b1                = ( w - aCutOffFrequency ) * Norm;
    a0                = a1 = aCutOffFrequency * Norm;
}

void LPFHPFilter::setHPF( float aCutOffFrequency, int aSampleRate )
{
    SAMPLE_TYPE w = 2.0 * aSampleRate;
    SAMPLE_TYPE Norm;

    aCutOffFrequency *= TWO_PI;
    Norm              = 1.0 / ( aCutOffFrequency + w );
    a0                = w * Norm;
    a1                = -a0;
    b1                = ( w - aCutOffFrequency ) * Norm;
}

void LPFHPFilter::process( AudioBuffer* sampleBuffer, bool isMonoSource )
{
    int bufferSize = sampleBuffer->bufferSize;

    for ( int c = 0, ca = sampleBuffer->amountOfChannels; c < ca; ++c )
    {
        SAMPLE_TYPE* channelBuffer = sampleBuffer->getBufferForChannel( c );

        for ( int i = 0; i < bufferSize; ++i )
        {
            SAMPLE_TYPE sample = channelBuffer[ i ];

            channelBuffer[ i ] = sample * a0 + inSamples[ c ] * a1 + outSamples[ c ] * b1;
            inSamples[ c ]     = sample; // store this unprocessed sample for next iteration
            outSamples[ c ]    = channelBuffer[ i ]; // and the processed sample too
        }

        // save CPU cycles when source is mono
        if ( isMonoSource )
        {
            sampleBuffer->applyMonoSource();
            break;
        }
    }
}
