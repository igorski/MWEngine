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
#include <modules/envelopefollower.h>
#include <cmath>

// constructor

EnvelopeFollower::EnvelopeFollower( float maxGain, float attackMs, float releaseMs, int sampleRate )
{
    envelope = 0.0;

    _attack  = pow( 0.01, maxGain / ( attackMs  * sampleRate / 1000 ));
    _release = pow( 0.01, maxGain / ( releaseMs * sampleRate / 1000 ));
}

/* public methods */

void EnvelopeFollower::process( SAMPLE_TYPE src )
{
    SAMPLE_TYPE v = std::abs( src );

    if ( v > envelope )
        envelope = _attack * ( envelope - v ) + v;
    else
        envelope = _release * ( envelope - v ) + v;
}
