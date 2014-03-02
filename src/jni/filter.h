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
#ifndef __FILTER_H_INCLUDED__
#define __FILTER_H_INCLUDED__

#include "baseprocessor.h"
#include "lfo.h"

class Filter : public BaseProcessor
{
    public:
        Filter( float aCutoffFrequency, float aResonance, float aMinFreq, float aMaxFreq, float aLfoRate, int numChannels );
        ~Filter();

        void setCutoff( float frequency );
        float getCutoff();
        void setResonance( float resonance );
        float getResonance();
        bool hasLFO();
        void hasLFO( bool value );
        float getLFO();
        void setLFO( LFO *lfo );
        void setLFORate( float rate );
        void process( AudioBuffer* sampleBuffer, bool isMonoSource );
        bool isCacheable();

    protected:
        float _cutoff;
        float _resonance;
        float _tempCutoff; // used for reading when automating via LFO

        // LFO related

        LFO *_lfo;
        float minFreq;
        float maxFreq;
        float lfoRange;

        float fs;

        // used internally

        float a1;
        float a2;
        float a3;
        float b1;
        float b2;
        float c;
        float output;

        float* in1;
        float* in2;
        float* out1;
        float* out2;

    private:
        bool _hasLFO;
        void calculateParameters();
};

#endif
