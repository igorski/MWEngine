/**
 * Ported from mdaLimiterProcessor.cpp
 * Created by Arne Scheffler on 6/14/08.
 *
 * mda VST Plug-ins
 *
 * Copyright (c) 2008 Paul Kellett
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
#include <utilities/utils.h>
#include "../global.h"

namespace MWEngine {

// constructors / destructor

Limiter::Limiter()
{
    init( 0.8f, 1.0f, 0.55f, true );
}

Limiter::Limiter( float attackNormalized, float releaseNormalized, float thresholdNormalized )
{
    init( attackNormalized, releaseNormalized, thresholdNormalized, false );
}

Limiter::Limiter( float attackInMicroseconds, float releaseInMilliseconds, float thresholdNormalized, bool softKnee )
{
    init( 0.f, 0.f, thresholdNormalized, softKnee );
    setAttackMicroseconds( attackInMicroseconds );
    setReleaseMilliseconds( releaseInMilliseconds );
}

Limiter::~Limiter()
{
    // nowt...
}

/* public methods */

float Limiter::getAttack()
{
    return inversePow(( float ) _attack, 10.f ) / -2.0f;
}

void Limiter::setAttack( float attackNormalized )
{
    _attack = pow( 10.0, -2.0 * attackNormalized );
}

float Limiter::getAttackMicroseconds()
{
    return -301030.1f / (( float ) AudioEngineProps::SAMPLE_RATE * log10( 1.0f - ( float ) _attack ));
}

void Limiter::setAttackMicroseconds( float attackInMicroseconds )
{
    _attack = 1.0 - inverseLog( 1.f / ( attackInMicroseconds / -301030.1f ) / ( float ) AudioEngineProps::SAMPLE_RATE, 10 );
}

float Limiter::getRelease()
{
    return ( inversePow(( float ) _release, 10.f ) + 2.f ) / -3.f;
}

void Limiter::setRelease( float releaseNormalized )
{
    _release = pow( 10.0, -2.0 - ( 3.0 * releaseNormalized ));
}

float Limiter::getReleaseMilliseconds()
{
    return -301.0301f / (( float ) AudioEngineProps::SAMPLE_RATE * log10( 1.0f - ( float ) _release ));
}

void Limiter::setReleaseMilliseconds( float releaseInMilliseconds )
{
    _release = 1.0 - inverseLog( 1.f / ( releaseInMilliseconds / -301.0301f ) / ( float ) AudioEngineProps::SAMPLE_RATE, 10 );
}

float Limiter::getThreshold()
{
    return ( float ) _threshold;
}

void Limiter::setThreshold( float thresholdNormalized )
{
    _threshold = ( SAMPLE_TYPE ) thresholdNormalized;
    cacheValues();
}

bool Limiter::getSoftKnee()
{
    return _softKnee;
}

void Limiter::setSoftKnee( bool softKnee )
{
    _softKnee = softKnee;
    cacheValues();
}

float Limiter::getLinearGR()
{
    return ( _gain > 1.0f ) ? 1.0f / ( float ) _gain : 1.0f;
}

void Limiter::process( AudioBuffer* sampleBuffer, bool isMonoSource )
{
    if ( _gain > 0.9999f && sampleBuffer->isSilent())
    {
        // don't process if input is silent
        return;
    }

    SAMPLE_TYPE gain, level, leftSample, rightSample;

    gain = _gain;

    int bufferSize = sampleBuffer->bufferSize;

    SAMPLE_TYPE* leftBuffer  = sampleBuffer->getBufferForChannel( 0 );
    SAMPLE_TYPE* rightBuffer = !isMonoSource ? sampleBuffer->getBufferForChannel( 1 ) : nullptr;
        
    if ( _softKnee )
    {
        for ( int i = 0; i < bufferSize; ++i ) {

            leftSample  = leftBuffer[ i ];
            rightSample = !isMonoSource ? rightBuffer[ i ] : 0;

            level = ( SAMPLE_TYPE ) ( 1.0 / ( 1.0 + pThreshold * fabs( leftSample + rightSample )));

            if ( gain > level ) {
                gain = gain - _attack * ( gain - level );
            }
            else {
                gain = gain + _release * ( level - gain );
            }

            leftBuffer[ i ] = ( leftSample * _trim * gain );

            if ( !isMonoSource ) {
                rightBuffer[ i ] = ( rightSample * _trim * gain );
            }
        }
    }
    else
    {
        for ( int i = 0; i < bufferSize; ++i ) {

            leftSample  = leftBuffer[ i ];
            rightSample = !isMonoSource ? rightBuffer[ i ] : 0;

            level = ( SAMPLE_TYPE ) ( 0.5 * gain * fabs( leftSample + rightSample ));

            if ( level > pThreshold ) {
                gain = gain - ( _attack * ( level - pThreshold ));
            }
            else {
                // below threshold
                gain = gain + ( SAMPLE_TYPE )( _release * ( 1.0 - gain ));
            }

            leftBuffer[ i ] = ( leftSample * _trim * gain );

            if ( !isMonoSource ) {
                rightBuffer[ i ] = ( rightSample * _trim * gain );
            }
        }
    }
    _gain = gain;
}

bool Limiter::isCacheable()
{
    return true;
}

/* protected methods */

void Limiter::init( float attackNormalized, float releaseNormalized, float thresholdNormalized, bool softKnee )
{
    _threshold = ( SAMPLE_TYPE ) thresholdNormalized;

    SAMPLE_TYPE trim = 0.60;

    _gain = 1.0;
    _trim = pow( 10.0, ( 2.0 * trim ) - 1.0 );

    setSoftKnee( softKnee );
    setAttack( attackNormalized );
    setRelease( releaseNormalized );
}

void Limiter::cacheValues()
{
    if ( _softKnee ) {
        pThreshold = pow( 10.0, 1.0 - ( 2.0 * _threshold ));
    } else {
        pThreshold = pow( 10.0, ( 2.0 * _threshold ) - 2.0 );
    }
}

} // E.O namespace MWEngine
