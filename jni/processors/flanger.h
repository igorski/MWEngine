/**
 * The MIT License (MIT)
 *
 * based on public source code by Dennis Cronin, adapted
 * by Igor Zinken - http://www.igorski.nl
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
#ifndef __FLANGER_H_INCLUDED__
#define __FLANGER_H_INCLUDED__

#include "baseprocessor.h"
#include "lowpassfilter.h"

// Adaptation of modf() by Dennis Cronin
// this macro breaks a double into integer and fractional components i and f respectively.
//
// n - input number, a double
// i - integer portion, an integer (the input number integer portion should fit)
// f - fractional portion, a double

#define MODF(n,i,f) ((i) = (int)(n), (f) = (n) - (double)(i))

/**
 * a mono/stereo Flanger effect (more channels currently not supported)
 */
class Flanger : public BaseProcessor
{
    public:

        // all arguments are in the 0 - 1 range
        Flanger( float rate, float width, float feedback, float delay, float mix );
        ~Flanger();

        float getRate();
        void setRate( float value );
        float getWidth();
        void setWidth( float value );
        float getFeedback();
        void setFeedback( float value );
        float getDelay();
        void setDelay( float value );
        float getMix();
        void setMix( float value );

        void process( AudioBuffer* sampleBuffer, bool isMonoSource );

    protected:

        float _rate;
        float _width;
        float _feedback;
        float _delay;
        float _mix;
        SAMPLE_TYPE _feedbackPhase;     // -1.0 to 1.0
        SAMPLE_TYPE _sweepSamples;      // sweep width in # of samples
        SAMPLE_TYPE _maxSweepSamples;   // upper bound of sweep in samples
        int _writePointer;
        SAMPLE_TYPE _step;
        SAMPLE_TYPE _sweep;
        SAMPLE_TYPE* _buf1;
        SAMPLE_TYPE* _buf2;

        LowPassFilter* _delayFilter;
        LowPassFilter* _mixFilter;

        SAMPLE_TYPE _lastSampleLeft;
        SAMPLE_TYPE _lastSampleRight;
        SAMPLE_TYPE _mixLeftWet;
        SAMPLE_TYPE _mixLeftDry;
        SAMPLE_TYPE _mixRightWet;
        SAMPLE_TYPE _mixRightDry;
        SAMPLE_TYPE _sweepRate;

        int FLANGER_BUFFER_SIZE;
        SAMPLE_TYPE SAMPLE_MULTIPLIER;

        void setSweep();
};

#endif
