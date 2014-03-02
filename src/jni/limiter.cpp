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
#include "limiter.h"
#include "global.h"

// constructor / destructor

Limiter::Limiter()
{
    init( 10, 500, audio_engine::SAMPLE_RATE, audio_engine::OUTPUT_CHANNELS );
}

Limiter::~Limiter()
{
    delete _follower;
    delete _output;
}

/**
 * creates a limiter, attack 10 ms works well with decay 500 ms for average use
 *
 * @param attackMs   {float} attack time in milliseconds
 * @param releaseMs  {float} attack decay time in milliseconds
 * @param sampleRate {int} the current samplerate
 * @param amountOfChannels {int} amount of output channels
 */
Limiter::Limiter( float attackMs, float releaseMs, int sampleRate, int amountOfChannels )
{
    init( attackMs, releaseMs, sampleRate, amountOfChannels );
}

/* public methods */

float Limiter::getLinearGR()
{
    return _follower->envelope > 1. ? 1 / _follower->envelope : 1.;
}

void Limiter::process( AudioBuffer* sampleBuffer, bool isMonoSource )
{
    for ( int i = 0, l = sampleBuffer->amountOfChannels; i < l; ++i )
    {
        float* channelBuffer = sampleBuffer->getBufferForChannel( i );
        int j                 = sampleBuffer->bufferSize;

        while ( j-- > 0 )
        {
            float dest = channelBuffer[ j ];

            _follower->process( dest, skip );

            if ( _follower->envelope > maxGain )
                dest = dest / _follower->envelope;

            channelBuffer[ j ] = ( dest + skip );
        }
    }
}

bool Limiter::isCacheable()
{
    return true;
}

/* protected methods */

void Limiter::init( float attackMs, float releaseMs, int sampleRate, int amountOfChannels )
{
    maxGain   = .85;
    _follower = new EnvelopeFollower( maxGain, attackMs, releaseMs, sampleRate );
    _output   = new short[ audio_engine::BUFFER_SIZE ];
    skip      = amountOfChannels - 1;
}
