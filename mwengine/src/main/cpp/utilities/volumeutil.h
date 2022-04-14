/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2018-2022 Igor Zinken - https://www.igorski.nl
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
#ifndef __MWENGINE__VOLUME_UTIL_H_INCLUDED__
#define __MWENGINE__VOLUME_UTIL_H_INCLUDED__

#include "../global.h"
#include <math.h>

namespace MWEngine {

// a convenient macro which is used to mix in a sample
// into an existing audio buffer preventing overflowing
// the range (defined by 1.0) which in turn will lead to clipping
// 1.0 defines the maximum value for the sample (+1.f for a float based range)
// sampleBase = your base audio sample
// sampleToAdd = sample to add to base

#define SUM_SAMPLES( sampleBase, sampleToAdd ) ((( MAX_VOLUME - (sampleBase)) * (sampleToAdd)) / MAX_VOLUME ) + (sampleBase)

namespace VolumeUtil
{
    /**
     * You can scale the power up to 3f, 4f+ for different curves
     * see https://ux.stackexchange.com/a/116300
     */
    static float CURVE = 2.f;

    /**
     * convenience method to scale a value in the 0 - 1 range
     * to a logarithmic scale. you can treat the input value
     * as a percentile while the resulting value can be
     * fed to actors inside the engine
     */
    inline float toLog( float value )
    {
        return pow( value, CURVE );
    }

    /**
     * convenience method to change a value from a
     * logarithmic scale back to a percentile value in the 0 - 1 range
     */
    inline float toLinear( float value )
    {
        return pow( value, 1.0f / CURVE );
    }
}

} // E.O namespace MWEngine

#endif
