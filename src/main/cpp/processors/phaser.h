/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2019 Igor Zinken - https://www.igorski.nl
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
#ifndef __MWENGINE__PHASER_H_INCLUDED__
#define __MWENGINE__PHASER_H_INCLUDED__

#include "baseprocessor.h"
#include <vector>

namespace MWEngine {
class AllPassDelay
{
    public:
        AllPassDelay();
        void delay( SAMPLE_TYPE aDelay );
        SAMPLE_TYPE update( SAMPLE_TYPE aSample );

    private:
        SAMPLE_TYPE _a1;
        SAMPLE_TYPE _zm1;
};

class Phaser : public BaseProcessor
{
    static const int STAGES = 6;

    public:
        Phaser( float aRate, float aFeedback, float aDepth, float aMinFreq, float aMaxFreq );
        Phaser( float aRate, float aFeedback, float aDepth, float aMinFreq, float aMaxFreq, int amountOfChannels );
        ~Phaser();

        void setDepth( float depth );
        float getDepth();
        void setFeedback( float fb );
        float getFeedback();
        void setRate( float aRate );
        float getRate();
        void setRange( float aMin, float aMax );
        void process( AudioBuffer* sampleBuffer, bool isMonoSource );

    private:
        int _amountOfChannels;
        SAMPLE_TYPE _dmin;
        SAMPLE_TYPE _dmax;
        SAMPLE_TYPE _fb;
        SAMPLE_TYPE _depth;
        SAMPLE_TYPE _zm1;
        SAMPLE_TYPE _lfoPhase;
        SAMPLE_TYPE _lfoInc;
        SAMPLE_TYPE _rate;

        std::vector<std::vector<AllPassDelay*>>* _alps;

        void init( float aRate, float aFeedback, float aDepth, float aMinFreq, float aMaxFreq, int amountOfChannels );
};
} // E.O namespace MWEngine

#endif
