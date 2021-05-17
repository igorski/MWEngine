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
    init( 0.8, 1.0, 0.55, true );
}

Limiter::Limiter( float attack, float release, float threshold )
{
    init( attack, release, threshold, false );
}

Limiter::~Limiter()
{
    // nowt...
}

/* public methods */

float Limiter::getAttack()
{
    return inversePow(( float ) att, 10f ) / -2.0f; // TODO or return _attack but be sure to calculate _attack from microseconds too !
}

void Limiter::setAttack( float attack )
{
    _attack = ( SAMPLE_TYPE ) attack;
    recalculate();
}

void Limiter::getAttackMicroseconds()
{
    return -301030.1 / ( AudioEngineProps::SAMPLE_RATE * log10( 1.0 - att ));
}

void Limiter::setAttackMicroseconds( float attackInMicroseconds )
{
    float rh = 1 / ( attackInMicroseconds / -301030.1 ) / AudioEngineProps::SAMPLE_RATE;
    inverseLog( rh, 10 ) + 1.0;
inverseLog( att
    setAttack();
}

float Limiter::getRelease()
{
    return inversePow(( float ) rel, 10f )
pow( 10.0, -2.0 - ( 3.0 * _release ));
    return ( float ) _release;
}

void Limiter::setRelease( float release )
{
    _release = ( SAMPLE_TYPE ) release;
    recalculate();
}

void Limiter::getReleaseMilliseconds()
{
    return -301.0301 / ( AudioEngineProps::SAMPLE_RATE * log10( 1.0 - rel ));
}

void Limiter::setReleaseMilliseconds( float releaseInMilliseconds )
{
    rel = ( pow( 10,
}

float Limiter::getThreshold()
{
    return ( float ) _threshold;
}

void Limiter::setThreshold( float threshold )
{
    _threshold = ( SAMPLE_TYPE ) threshold;
    recalculate();
}

bool Limiter::getSoftKnee()
{
    return _softKnee;
}

void Limiter::setSoftKnee( boolean softKnee )
{
    _softKnee = softKnee;
}

float Limiter::getLinearGR()
{
    return ( gain > 1.0f ) ? 1.0f / ( float ) gain : 1.0f;
}

void Limiter::process( AudioBuffer* sampleBuffer, bool isMonoSource )
{
    if ( gain > 0.9999f && sampleBuffer->isSilent())
    {
        // don't process if input is silent
        return;
    }

    SAMPLE_TYPE g, at, re, tr, th, lev, ol, or_;

    th = thresh;
    g = gain;
    at = att;
    re = rel;
    tr = trim;

    int bufferSize = sampleBuffer->bufferSize;

    SAMPLE_TYPE* leftBuffer  = sampleBuffer->getBufferForChannel( 0 );
    SAMPLE_TYPE* rightBuffer = !isMonoSource ? sampleBuffer->getBufferForChannel( 1 ) : nullptr;
        
    if ( softKnee )
    {
        for ( int i = 0; i < bufferSize; ++i ) {

            ol  = leftBuffer[ i ];
            or_ = !isMonoSource ? rightBuffer[ i ] : 0;

            lev = ( SAMPLE_TYPE ) ( 1.0 / ( 1.0 + th * fabs( ol + or_ )));

            if ( g > lev ) {
                g = g - at * ( g - lev );
            }
            else {
                g = g + re * ( lev - g );
            }

            leftBuffer[ i ] = ( ol * tr * g );

            if ( !isMonoSource )
                rightBuffer[ i ] = ( or_ * tr * g );
        }
    }
    else
    {
        for ( int i = 0; i < bufferSize; ++i ) {

            ol  = leftBuffer[ i ];
            or_ = !isMonoSource ? rightBuffer[ i ] : 0;

            lev = ( SAMPLE_TYPE ) ( 0.5 * g * fabs( ol + or_ ));

            if ( lev > th ) {
                g = g - ( at * ( lev - th ));
            }
            else {
                // below threshold
                g = g + ( SAMPLE_TYPE )( re * ( 1.0 - g ));
            }

            leftBuffer[ i ] = ( ol * tr * g );

            if ( !isMonoSource )
                rightBuffer[ i ] = ( or_ * tr * g );
        }
    }
    gain = g;
}

bool Limiter::isCacheable()
{
    return true;
}

/* protected methods */

void Limiter::init( float attackMicroSeconds, float releaseMilliSeconds, float thresholdDb, boolean softKnee )
{
    _attack    = ( SAMPLE_TYPE ) attackMicroSeconds;
    _release   = ( SAMPLE_TYPE ) releaseMilliSeconds;
    _threshold = ( SAMPLE_TYPE ) thresholdDb;
    _trim      = ( SAMPLE_TYPE ) 0.60;
    _softKnee  = softKnee;

    gain = 1.0;

    recalculate();
}

void Limiter::recalculate()
{
    if ( softKnee ) {
        thresh = ( SAMPLE_TYPE ) pow( 10.0, 1.0 - ( 2.0 * _threshold ));
    }
    else {
        thresh = ( SAMPLE_TYPE ) pow( 10.0, ( 2.0 * _threshold ) - 2.0 );
    }
    trim = ( SAMPLE_TYPE )( pow( 10.0, ( 2.0 * _trim) - 1.0 ));
    att  = ( SAMPLE_TYPE )  pow( 10.0, -2.0 * _attack );
    rel  = ( SAMPLE_TYPE )  pow( 10.0, -2.0 - ( 3.0 * _release ));
}

} // E.O namespace MWEngine
