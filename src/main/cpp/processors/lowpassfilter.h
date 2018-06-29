/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2016-2018 Igor Zinken - http://www.igorski.nl
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
#ifndef __LOWPASSFILTER_H_INCLUDED__
#define __LOWPASSFILTER_H_INCLUDED__

#include "baseprocessor.h"

/**
 * a simple two pole low-pass filter
 */
class LowPassFilter : public BaseProcessor
{
    public:
        LowPassFilter( float cutoff );
        ~LowPassFilter();

        float getCutoff();
        void setCutoff( float value);

        void process( AudioBuffer* sampleBuffer, bool isMonoSource );

        inline SAMPLE_TYPE processSingle( SAMPLE_TYPE sample )
        {
            SAMPLE_TYPE sampleOut = (b0/a0) * sample + (b1/a0) * x1 + (b2/a0) * x2 - (a1/a0) * y1 - (a2/a0) * y2;

            x2 = x1;
            x1 = sample;
            y2 = y1;
            y1 = sampleOut;

            return sampleOut;
        }

        // store/restore the processor properties
        // this ensures that multi channel processing for a
        // single buffer uses all properties across all channels
        // store() before processing channel 0, restore() every
        // channel afterwards

        void store();
        void restore();

    protected:
        SAMPLE_TYPE x1, x2, y1, y2;
        SAMPLE_TYPE orgx1, orgx2, orgy1, orgy2;
        SAMPLE_TYPE a0, a1, a2, b0, b1, b2, w0, alpha;

        float _cutoff;
};

#endif
