/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2016 Igor Zinken - http://www.igorski.nl
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
#ifndef __MWENGINE__RINGBUFFER_H_INCLUDED__
#define __MWENGINE__RINGBUFFER_H_INCLUDED__

#include "global.h"
#include <cstring>

namespace MWEngine {
class RingBuffer
{
    public:
        RingBuffer( int capacity );
        ~RingBuffer();
        int getBufferLength();
        int getSize();
        bool isEmpty();
        bool isFull();

        inline void enqueue( SAMPLE_TYPE aSample )
        {
            _buffer[ _last ] = aSample;

            if ( ++_last >= _bufferLength )
                _last = 0;
        }

        inline SAMPLE_TYPE dequeue()
        {
            SAMPLE_TYPE item = _buffer[ _first ];

            if ( ++_first >= _bufferLength )
                _first = 0;

            return item;
        }

        inline SAMPLE_TYPE peek()
        {
            return _buffer[ _first ];
        }

        inline void flush()
        {
            _first = 0;
            _last  = 0;

            // set buffer values to 0.0 for silence

            if ( _buffer != nullptr )
                memset( _buffer, 0, _bufferLength * sizeof( SAMPLE_TYPE ));
        }

    protected:
        SAMPLE_TYPE* _buffer;
        int _bufferLength;
        int _first;
        int _last;
};
} // E.O namespace MWEngine

#endif
