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
#include "../global.h"

namespace MWEngine {

// constructors / destructor

Limiter::Limiter()
{
    init( 0.15, 0.50, 0.60 );
}

Limiter::Limiter( float attackMs, float releaseMs, float thresholdDb )
{
    init( attackMs, releaseMs, thresholdDb );
}

Limiter::~Limiter()
{
    // nowt...
}

/* public methods */

float Limiter::getAttack()
{
    return ( float ) pAttack;
}

void Limiter::setAttack( float attackMs )
{
    pAttack = ( SAMPLE_TYPE ) attackMs;
    recalculate();
}

float Limiter::getRelease()
{
    return ( float ) pRelease;
}

void Limiter::setRelease( float releaseMs )
{
    pRelease = ( SAMPLE_TYPE ) releaseMs;
    recalculate();
}

float Limiter::getThreshold()
{
    return ( float ) pTresh;
}

void Limiter::setThreshold( float thresholdDb )
{
    pTresh = ( SAMPLE_TYPE ) thresholdDb;
    recalculate();
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
        
    if ( pKnee > 0.5 )
    {
        // soft knee
        
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

void Limiter::init( float attackMs, float releaseMs, float thresholdDb )
{
    pAttack  = ( SAMPLE_TYPE ) attackMs;
    pRelease = ( SAMPLE_TYPE ) releaseMs;
    pTresh   = ( SAMPLE_TYPE ) thresholdDb;
    pTrim    = ( SAMPLE_TYPE ) 0.60;
    pKnee    = ( SAMPLE_TYPE ) 0.40;

    gain = 1.0;

    recalculate();
}

void Limiter::recalculate()
{
    if ( pKnee > 0.5 ) {
        // soft knee
        thresh = ( SAMPLE_TYPE ) pow( 10.0, 1.0 - ( 2.0 * pTresh ));
    }
    else {
        // hard knee
        thresh = ( SAMPLE_TYPE ) pow( 10.0, ( 2.0 * pTresh ) - 2.0 );
    }
    trim = ( SAMPLE_TYPE )( pow( 10.0, ( 2.0 * pTrim) - 1.0 ));
    att  = ( SAMPLE_TYPE )  pow( 10.0, -2.0 * pAttack );
    rel  = ( SAMPLE_TYPE )  pow( 10.0, -2.0 - ( 3.0 * pRelease ));
}

} // E.O namespace MWEngine
