/**
 * Ported from mdaLimiterProcessor.h
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
#ifndef __MWENGINE__LIMITER_H_INCLUDED__
#define __MWENGINE__LIMITER_H_INCLUDED__

#include "baseprocessor.h"
#include "../audiobuffer.h"
#include <vector>

namespace MWEngine {
class Limiter : public BaseProcessor
{
    public:
        Limiter();
        /**
         * Unit specific constructor
         * @param attackInMicroseconds up to 1563.90 microseconds
         * @param releaseInMilliseconds up to 1571/755 milliseconds
         * @param thresholdNormalized 0 - 1 range where 0 == -20 dB and 1 == +20 dB
         * @param softKnee
         */
        Limiter( float attackInMicroseconds, float releaseInMilliseconds, float thresholdNormalized, bool softKnee );
        /**
         * legacy constructor using normalized (0 - 1 range) values (TO BE DEPRECATED?)
         * @param attackNormalized (1 == 1563.89 microseconds)
         * @param releaseNormalized (1 == 1571.755 milliseconds)
         * @param thresholdNormalized (0 == -20 dB and 1 == +20 dB)
         */
        Limiter( float attackNormalized, float releaseNormalized, float thresholdNormalized );
        ~Limiter();

        std::string getType() const {
            return std::string( "Limiter" );
        }

        // getter/setter in 0 - 1 range where 1 == 1563.89 microseconds (1.56 milliseconds)
        float getAttack();
        void setAttack( float attackNormalized );
        // getter/setter using microseconds
        float getAttackMicroseconds();
        void setAttackMicroseconds( float attackInMicroseconds );

        // getter/setter in 0 - 1 range where 1 == 1571.755 milliseconds
        float getRelease();
        void setRelease( float releaseNormalized );
        // getter/setter using milliseconds
        float getReleaseMilliseconds();
        void setReleaseMilliseconds( float releaseInMilliseconds );

        // 0 - 1 range where 0 == -20 dB and 1 == +20 dB
        float getThreshold();
        void setThreshold( float thresholdNormalized );

        bool getSoftKnee();
        void setSoftKnee( bool softKnee );

        float getLinearGR();

#ifndef SWIG
        // internal to the engine
        void process( AudioBuffer* sampleBuffer, bool isMonoSource );
        bool isCacheable();
#endif

    protected:
        void init( float attackNormalized, float releaseNormalized, float thresholdNormalized, bool softKnee );
        void cacheValues();

        // instance variables

        SAMPLE_TYPE _threshold;
        SAMPLE_TYPE _trim;
        SAMPLE_TYPE _attack;
        SAMPLE_TYPE _release;
        SAMPLE_TYPE _gain;
        bool        _softKnee;
        SAMPLE_TYPE pThreshold; // cached process value of threshold for given knee type
    };
} // E.O namespace MWEngine

#endif
