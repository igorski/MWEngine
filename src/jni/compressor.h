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
#ifndef __COMPRESSOR_H_INCLUDED__
#define __COMPRESSOR_H_INCLUDED__

#include "baseprocessor.h"
#include <chunkware/SimpleComp.h>

class Compressor : public BaseProcessor
{
    public:
        Compressor( float aThreshold, float aAttack, float aRelease, float aRatio );
        ~Compressor();

        static const int THRESHOLD_MAX_NEGATIVE_VALUE = 40;
        static const int THRESHOLD_MAX_POSITIVE_VALUE = 20;

        void process( AudioBuffer* sampleBuffer, bool isMonoSource );
        bool isCacheable();

        float getAttack();
        void setAttack( float value );

        float getRelease();
        void setRelease( float value );

        float getThreshold();
        void setThreshold( float value );

        float getRatio();
        void setRatio( float value );

        void setSampleRate( int aSampleRate );

    protected:

    private:
        chunkware_simple::SimpleComp* _sc;
};

#endif
