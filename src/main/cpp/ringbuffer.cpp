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
#include "ringbuffer.h"
#include <utilities/bufferutility.h>

namespace MWEngine {

/* constructor / destructor */

RingBuffer::RingBuffer( int capacity )
{
    _bufferLength = capacity;
    _buffer       = BufferUtility::generateSilentBuffer( _bufferLength );
    _first        = 0;
    _last         = 0;
}

RingBuffer::~RingBuffer()
{
    delete[] _buffer;
    _buffer = nullptr;
}

/* public methods */

int RingBuffer::getBufferLength()
{
    return _bufferLength;
}

int RingBuffer::getSize()
{
    return _last - _first;
}

bool RingBuffer::isEmpty()
{
    return getSize() == 0;
}

bool RingBuffer::isFull()
{
    return getSize() == _bufferLength;
}

} // E.O namespace MWEngine
