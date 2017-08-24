/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2017 Igor Zinken - http://www.igorski.nl
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
#ifndef __PHASER_H_INCLUDED__
#define __PHASER_H_INCLUDED__

#include "baseprocessor.h"
#include <vector>

class AllPassDelay
{
    public:
        AllPassDelay();
        void delay( float aDelay );
        float update( float aSample );

    private:
        float _a1;
        float _zm1;
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
        int _channels;
        float _dmin;
        float _dmax;
        float _fb;
        float _depth;
        float _zm1;
        float _lfoPhase;
        float _lfoInc;
        float _rate;

        std::vector<std::vector<AllPassDelay*>>* _alps;

        void init( float aRate, float aFeedback, float aDepth, float aMinFreq, float aMaxFreq, int amountOfChannels );
};

#endif
