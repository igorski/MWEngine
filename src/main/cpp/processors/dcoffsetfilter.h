/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2014 Igor Zinken - http://www.igorski.nl
 *
 * DCOffsetFilter is based on work by Julius O. Smith III (jos@ccrma.stanford.edu)
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
#ifndef __MWENGINE__DCOFFSETFILTER_H_INCLUDED__
#define __MWENGINE__DCOFFSETFILTER_H_INCLUDED__

#include "baseprocessor.h"

namespace MWEngine {
class DCOffsetFilter : public BaseProcessor
{
    public:
        DCOffsetFilter( int amountOfChannels );
        ~DCOffsetFilter();

        void process( AudioBuffer* sampleBuffer, bool isMonoSource );

    private:
        SAMPLE_TYPE* _lastInSamples;
        SAMPLE_TYPE* _lastOutSamples;
        SAMPLE_TYPE  R;
};
} // E.O namespace MWEngine

#endif
