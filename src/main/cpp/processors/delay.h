/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2018 Igor Zinken - http://www.igorski.nl
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
#ifndef __DELAY_H_INCLUDED__
#define __DELAY_H_INCLUDED__

#include "baseprocessor.h"

class Delay : public BaseProcessor
{
    public:
        Delay( int aDelayTime, int aMaxDelayTime, float aMix, float aFeedback, int amountOfChannels );
        ~Delay();

        int getDelayTime();
        void setDelayTime( int aValue );
        float getMix();
        void setMix( float aValue );
        float getFeedback();
        void setFeedback( float aValue );
        void process( AudioBuffer* sampleBuffer, bool isMonoSource );
        void reset();

    protected:
        AudioBuffer* _delayBuffer;
        int* _delayIndices;
        int _time;
        int _maxTime;
        float _mix;
        float _feedback;
        int _amountOfChannels;
};

#endif
