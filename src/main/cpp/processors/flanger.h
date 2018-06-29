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
// n - input number, a sample (e.g. float or double)
// i - integer portion, an integer (the input number integer portion should fit)
// f - fractional portion, a sample (e.g. float or double)

#define MODF(n,i,f) ((i) = (int)(n), (f) = (n) - ( SAMPLE_TYPE )(i))

/**
 * a multichannel Flanger effect
 */
class Flanger : public BaseProcessor
{
    public:

        // all arguments are in the 0 - 1 range
        Flanger( float rate, float width, float feedback, float delay, float mix );
        Flanger();
        ~Flanger();

        float getRate();
        void setRate( float value );
        float getWidth();
        void setWidth( float value );
        float getFeedback();
        void setFeedback( float value );
        float getDelay();
        void setDelay( float value );

        // get/set the wet/dry mix of the effect as a whole

        float getMix();
        void setMix( float value );

        // set the wet/dry mix of individual output channels

        void setChannelMix( int channel, float wet );

        void process( AudioBuffer* sampleBuffer, bool isMonoSource );

    protected:

        float _rate;
        float _width;
        float _feedback;
        float _delay;
        float _mix;
        SAMPLE_TYPE _feedbackPhase;
        SAMPLE_TYPE _sweepSamples;
        SAMPLE_TYPE _maxSweepSamples;
        int _writePointer;
        SAMPLE_TYPE _step;
        SAMPLE_TYPE _sweep;
        std::vector<SAMPLE_TYPE*> _buffers;

        LowPassFilter* _delayFilter;
        LowPassFilter* _mixFilter;

        struct ChannelCache {
            SAMPLE_TYPE lastSample;
            SAMPLE_TYPE mixDry;
            SAMPLE_TYPE mixWet;

            ChannelCache() {
                lastSample = 0.f;
                mixDry     = 1.f;
                mixWet     = 1.f;
            }
        };

        std::vector<ChannelCache*> _caches;
        SAMPLE_TYPE _sweepRate;

        int FLANGER_BUFFER_SIZE;
        SAMPLE_TYPE SAMPLE_MULTIPLIER;

        void setSweep();
        void init( float rate, float width, float feedback, float delay, float mix );
};

#endif
