/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Igor Zinken - https://www.igorski.nl
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
#ifndef __MWENGINE__RESIZABLE_AUDIOBUFFER_H_INCLUDED__
#define __MWENGINE__RESIZABLE_AUDIOBUFFER_H_INCLUDED__

#include "audiobuffer.h"

namespace MWEngine {
/**
 * A ResizableAudioBuffer can be used when buffer sizes should be dynamic throughout their lifetime.
 * For instance: when working with output buffers that should stretch/shrink to match the driver callback sample amount
 * during rendering, for instance when using AAudio as the driver which has dynamic sample sizes per callback slice.
 *
 * Within MWEngine, the bufferSize of an AudioBuffer determines the read/write limits when rendering
 * audio into/from AudioBuffers. A ResizableBuffer will "shrink" its buffer from its original size
 * without reallocating the sample vectors to prevent CPU and allocation overhead (as the updated bufferSize value
 * will act as a upper read/write limit). Only when the buffer expands beyond its original size will new vectors be
 * allocated. This will happen transparently but should occur outside of read/write cycles (!)
 */
class ResizableAudioBuffer : public AudioBuffer
{
    public:
        ResizableAudioBuffer( int amountOfChannels, int bufferSize ) : AudioBuffer( amountOfChannels, bufferSize ) {
            _vectorSize = bufferSize;
        };
        ~ResizableAudioBuffer();

        // expands/shrinks the current AudioBuffers sample vectors
        void resize( int newSize );

    protected:
        int _vectorSize; // original size
};
} // E.O namespace MWEngine

#endif
