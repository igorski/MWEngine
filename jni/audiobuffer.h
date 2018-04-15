/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2018 Igor Zinken - http://www.igorski.nl
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
#ifndef __AUDIOBUFFER_H_INCLUDED__
#define __AUDIOBUFFER_H_INCLUDED__

#include "global.h"
#include <vector>

class AudioBuffer
{
    public:

        // construct an AudioBuffer for given channel amount and buffer size
        // the sample rate will default to the sample rate of the engine
        AudioBuffer( int aAmountOfChannels, int aBufferSize );

        // construct an AudioBuffer as above, but with a custom sampleRate
        // e.g. samples created outside of the engine
        AudioBuffer( int aAmountOfChannels, int aBufferSize, int sampleRate );
        ~AudioBuffer();

        int amountOfChannels;
        int bufferSize;
        int sampleRate;
        bool loopeable;

        SAMPLE_TYPE* getBufferForChannel( int aChannelNum );
        int mergeBuffers( AudioBuffer* aBuffer, int aReadOffset, int aWriteOffset, float aMixVolume );
        void silenceBuffers();
        void adjustBufferVolumes( SAMPLE_TYPE amp );
        void applyMonoSource();
        AudioBuffer* clone();

    protected:
        std::vector<SAMPLE_TYPE*>* _buffers;
        void init( int aAmountOfChannels, int aBufferSize, int sampleRate );
};

#endif
