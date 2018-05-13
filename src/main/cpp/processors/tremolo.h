/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2016 Igor Zinken - http://www.igorski.nl
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
#ifndef __TREMOLO_H_INCLUDED__
#define __TREMOLO_H_INCLUDED__

#include "baseprocessor.h"
#include "../audiobuffer.h"
#include <events/sampleevent.h>
#include <vector>

class Tremolo : public BaseProcessor
{
    public:

        // envelope types

        enum types {
            LINEAR,
            EXPONENTIAL
        };

        static const int ENVELOPE_PRECISION = 960; // 96 dB range

        // Tremolo can work with two distinct channels, if the left or right channel
        // have a different type or envelope length, the effect operates in stereo

        Tremolo( int aLeftType,  int aLeftAttack,  int aLeftDecay,
                 int aRightType, int aRightAttack, int aRightDecay );

        ~Tremolo();

        int getLeftAttack();
        void setLeftAttack ( int aAttack );
        int getRightAttack();
        void setRightAttack( int aAttack );
        int getLeftDecay();
        void setLeftDecay  ( int aDecay );
        int getRightDecay();
        void setRightDecay ( int aDecay );

        // aChannelNum 0 = left channel table, aChannelNum 1 = right channel table

        SAMPLE_TYPE* getTableForChannel( int aChannelNum );

        bool isStereo();
        void process( AudioBuffer* sampleBuffer, bool isMonoSource );

    protected:

        std::vector<SAMPLE_TYPE*>* _tables;

        SAMPLE_TYPE _leftTableIndex;
        SAMPLE_TYPE _rightTableIndex;

        // envelope state (0 = attack, 1 = decayy)
        int _leftState;
        int _rightState;

        int _leftType;
        int _rightType;
        int _leftAttack;
        int _rightAttack;
        int _leftDecay;
        int _rightDecay;

        SAMPLE_TYPE _leftAttackIncr;
        SAMPLE_TYPE _leftDecayIncr;
        SAMPLE_TYPE _rightAttackIncr;
        SAMPLE_TYPE _rightDecayIncr;
};

#endif
