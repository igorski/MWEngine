/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2014 Igor Zinken - http://www.igorski.nl
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
#ifndef __FM_H_INCLUDED__
#define __FM_H_INCLUDED__

#include "baseprocessor.h"
#include "../global.h"
#include "../lfo.h"

class FrequencyModulator : public BaseProcessor, public LFO
{
    public:
        FrequencyModulator( int aWaveForm, float aRate );
        void process( AudioBuffer* sampleBuffer, bool isMonosource );

        // these are here only for SWIG purposes so we can "multiple inherit" from LFO, bit fugly... but hey
        #ifdef SWIG
        float getRate();
        void setRate( float value );
        int getWave();
        void setWave( int value );
        #endif

    private:
        SAMPLE_TYPE* _buffer; // cached buffer
        SAMPLE_TYPE modulator;
        SAMPLE_TYPE carrier;
        SAMPLE_TYPE fmamp;
        SAMPLE_TYPE TWO_PI_OVER_SR;
};

#endif
