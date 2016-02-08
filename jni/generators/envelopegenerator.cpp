/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2016 Igor Zinken - http://www.igorski.nl
 *
 * envelope generation adapted from sources by Christian Schoenebeck
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
#include "envelopegenerator.h"
#include "../global.h"
#include <definitions/waveforms.h>
#include <algorithm>
#include <math.h>

namespace EnvelopeGenerator
{
    SAMPLE_TYPE* generateLinear( int tableLength, SAMPLE_TYPE startAmplitude, SAMPLE_TYPE endAmplitude )
    {
        SAMPLE_TYPE* out = new SAMPLE_TYPE[ tableLength ];

        // we can't use a 0.0 value as we would get Infinity values, sanitize to a small, positive number instead

        startAmplitude = std::max( 0.001, startAmplitude );
        endAmplitude   = std::max( 0.001, endAmplitude );

        SAMPLE_TYPE coeff  = MAX_PHASE + ( log( endAmplitude ) - log( startAmplitude )) / tableLength;
        SAMPLE_TYPE sample = startAmplitude;

        for ( int i = 0; i < tableLength; ++i )
        {
            sample  *= coeff;
            out[ i ] = sample;
        }
        return out;
    }

    SAMPLE_TYPE* generateExponential( int tableLength )
    {
        SAMPLE_TYPE* out = new SAMPLE_TYPE[ tableLength ];

        // ensure the first index is 0.0

        out[ 0 ] = 0.0;

        for ( int i = 1; i < tableLength; i++ ) {
            out[ i ] = pow( 10.0f, (( SAMPLE_TYPE ) ( tableLength - 1 ) - i ) / -200.0f );
         }
        return out;
    }
}
