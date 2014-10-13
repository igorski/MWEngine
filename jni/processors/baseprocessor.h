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
#ifndef __BASEPROCESSOR_H_INCLUDED__
#define __BASEPROCESSOR_H_INCLUDED__

#include "../global.h"
#include "../audiobuffer.h"

class BaseProcessor
{
    public:
        BaseProcessor();
        ~BaseProcessor();

        /**
         * @param {audioBuffer*} sampleBuffer the buffer to write into
         * @param {bool} isMonoSource whether the source signal is mono (save CPU cycles
         *               by solely processing a single channel and writing its values into the
         *               remaining channels
         */
        virtual void process( AudioBuffer* sampleBuffer, bool isMonoSource );

        /**
         * if this processors effect is non-dynamic its output
         * can be cached to omit unnecessary repeated calculations
         * consuming unnecessary CPU cycles
         */
        virtual bool isCacheable();
};

#endif
