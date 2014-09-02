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

// constructors / destructor

Limiter::Limiter()
{

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

Limiter::~Limiter()
{
    while ( !_followers->empty())
    {
        delete _followers->back(), _followers->pop_back();
    }
}

/* public methods */

float Limiter::getLinearGR()
{
    // TODO : currently mono / left signal only
    return _followers->at( 0 )->envelope > 1. ? 1 / _followers->at( 0 )->envelope : 1.;
}

void Limiter::process( AudioBuffer* sampleBuffer, bool isMonoSource )
{
    for ( int i = 0, l = sampleBuffer->amountOfChannels; i < l; ++i )
    {
        SAMPLE_TYPE* channelBuffer = sampleBuffer->getBufferForChannel( i );
        EnvelopeFollower* follower = _followers->at( i );
        int j                      = sampleBuffer->bufferSize;

        while ( j-- > 0 )
        {
            SAMPLE_TYPE dest = channelBuffer[ j ];

            follower->process( dest );

            if ( follower->envelope > maxGain )
                dest = dest / follower->envelope;

            channelBuffer[ j ] = ( dest );
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
    maxGain    = .85;
    _followers = new std::vector<EnvelopeFollower*>( amountOfChannels );

    for ( int i = 0; i < amountOfChannels; ++i )
        _followers->at( i ) = new EnvelopeFollower( maxGain, attackMs, releaseMs, sampleRate );
}

