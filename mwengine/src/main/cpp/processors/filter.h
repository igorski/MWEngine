/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2020 Igor Zinken - https://www.igorski.nl
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
#ifndef __MWENGINE__FILTER_H_INCLUDED__
#define __MWENGINE__FILTER_H_INCLUDED__

#include "baseprocessor.h"
#include <modules/lfo.h>

namespace MWEngine {
class Filter : public BaseProcessor
{
    public:
        Filter( float aCutoffFrequency, float aResonance, float aMinFreq, float aMaxFreq, int numChannels );
        Filter();
        ~Filter();

        std::string getType() const {
            return std::string( "Filter" );
        }

        void setCutoff( float frequency );
        float getCutoff();
        void setResonance( float resonance );
        float getResonance();
        bool hasLFO();
        LFO* getLFO();
        void setLFO( LFO *lfo );

#ifndef SWIG
        // internal to the engine
        void process( AudioBuffer* sampleBuffer, bool isMonoSource );
        bool isCacheable();
#endif

    protected:
        float _cutoff;
        float _resonance;
        float _minFreq;
        float _maxFreq;

        // LFO related

        LFO* _lfo;
        float _tempCutoff; // current filter cutoff (is modulated by LFO sweep)

        // used internally

        float SAMPLE_RATE;
        int amountOfChannels;
        SAMPLE_TYPE a1;
        SAMPLE_TYPE a2;
        SAMPLE_TYPE a3;
        SAMPLE_TYPE b1;
        SAMPLE_TYPE b2;
        SAMPLE_TYPE c;
        SAMPLE_TYPE output;

        SAMPLE_TYPE* in1;
        SAMPLE_TYPE* in2;
        SAMPLE_TYPE* out1;
        SAMPLE_TYPE* out2;

    private:
        void init( float cutoff );
        void calculateParameters();
};
} // E.O namespace MWEngine

#endif
