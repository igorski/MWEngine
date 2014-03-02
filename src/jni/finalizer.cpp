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
#include "finalizer.h"

// constructor

/**
 * Finalizer should be seen as the "final" processor in a typical setting, it is essentially
 * a limiter making sure no clipping occurs, as well as a preventive measure to omit
 * nasty pops and clicks caused by DC offsets
 *
 * @param attackMs   {float} attack time in milliseconds
 * @param releaseMs  {float} attack decay time in milliseconds
 * @param sampleRate {int} the current samplerate
 * @param amountOfChannels {int} the amount of output channels
 */
Finalizer::Finalizer( float attackMs, float releaseMs, int sampleRate, int amountOfChannels )
{
    init( attackMs, releaseMs, sampleRate, amountOfChannels );

    lastSamples = new float[ amountOfChannels ];

    for ( int i = 0; i < amountOfChannels; ++i )
        lastSamples[ i ] = 0.0;
}

Finalizer::~Finalizer()
{
    delete[] lastSamples;
}

/* public methods */

/**
 * used by the AudioBouncer, which doesn't need shorts!
 * @param sampleBuffer {AudioBuffer*} buffer containing samples to limit
 */
void Finalizer::process( AudioBuffer* sampleBuffer, bool isMonoSource )
{
    Limiter::process( sampleBuffer, isMonoSource );

    int bufferSize = sampleBuffer->bufferSize;

    for ( int i = 0, l = sampleBuffer->amountOfChannels; i < l; ++i )
    {
        float* channelBuffer = sampleBuffer->getBufferForChannel( i );

        for ( int j = 0; j < bufferSize; ++j )
        {
            float lastSample = lastSamples[ i ];
            float theSample  = 0.996f * ( lastSample + channelBuffer[ j ] - lastSample );

            // extreme limiting (still above the thresholds?)
            if ( theSample < -1.0f )
                theSample = -1.0f;

            else if ( theSample > +1.0f )
                theSample = +1.0f;

            lastSamples[ i ]   = theSample;
            channelBuffer[ j ] = theSample;
        }
    }
}
