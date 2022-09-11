/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 Igor Zinken - https://www.igorski.nl
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
#ifndef __MWENGINE__RESIZABLE_BUFFERGROUP_H_INCLUDED__
#define __MWENGINE__RESIZABLE_BUFFERGROUP_H_INCLUDED__

#include "audiobuffer.h"
#include "resizable_audiobuffer.h"
#include <vector>

namespace MWEngine {
class ResizableBufferGroup
{
    public:
        ResizableBufferGroup( int bufferAmount, int amountOfChannels, int bufferSize );
        ~ResizableBufferGroup();

        ResizableAudioBuffer* getBufferAtIndex( int index );

        int getGroupSize();

        /**
         * Retrieves the content of all buffers in the group within a (new!) AudioBuffer instance
         * where every buffer within this ResizableBufferGroup has serialized its content one after
         * the other. The size of this new buffer is equal to the size of a single buffer multiplied
         * by the amount of buffers in this group.
         */
        AudioBuffer* getContentSerialized();

        /**
         * expands/shrinks the current AudioBuffers sample vectors
         * NOTE: shrinking will keep existing content, expanding will
         * clear the existing contents.
         */
        void resize( int newSize );

    private:
        std::vector<ResizableAudioBuffer*>* _buffers = nullptr;
};
} // E.O namespace MWEngine

#endif
