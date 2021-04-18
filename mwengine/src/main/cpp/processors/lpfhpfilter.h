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
#ifndef __MWENGINE__LPFHPFILTER_H_INCLUDED__
#define __MWENGINE__LPFHPFILTER_H_INCLUDED__

#include "baseprocessor.h"
#include "../audiobuffer.h"

namespace MWEngine {
class LPFHPFilter : public BaseProcessor
{
    public:
        LPFHPFilter( float aLPCutoff, float aHPCutoff, int amountOfChannels );
        ~LPFHPFilter();

        std::string getType() {
            return std::string( "LPFHPFilter" );
        }

        void setLPF( float aCutOffFrequency, int aSampleRate );
        void setHPF( float aCutOffFrequency, int aSampleRate );

#ifndef SWIG
        // internal to the engine
        void process( AudioBuffer* sampleBuffer, bool isMonoSource );
#endif

    private:
        SAMPLE_TYPE a0;
        SAMPLE_TYPE a1;
        SAMPLE_TYPE b1;

        // for each channel we store the previous in- and output samples
        SAMPLE_TYPE* outSamples;
        SAMPLE_TYPE* inSamples;
};
} // E.O namespace MWEngine

#endif
