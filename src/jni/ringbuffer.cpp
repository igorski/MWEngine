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
#include "ringbuffer.h"
#include "utils.h"

/* constructor / destructor */

RingBuffer::RingBuffer( int capacity )
{
    bufferLength = capacity;
    _buffer      = BufferUtil::generateSilentBuffer( capacity );

    first        = 0;
    last         = 0;
}

RingBuffer::~RingBuffer()
{
    if ( _buffer != 0 )
    {
        delete _buffer;
        _buffer = 0;
    }
}

/* public methods */

int RingBuffer::getSize()
{
    return ( last - first );
}

bool RingBuffer::isEmpty()
{
    return ( getSize() == 0 );
}

bool RingBuffer::isFull()
{
    return ( getSize() == bufferLength );
}

void RingBuffer::flush()
{
    first = 0;
    last  = 0;

    if ( _buffer != 0 )
    {
        for ( int i = 0; i < bufferLength; i++ )
            _buffer[ i ] = 0.0;
    }
}

void RingBuffer::enqueue( float aSample )
{
    if ( _buffer == 0 )
        return;             // TODO: WHERE DOES THIS NONSENSE COME FROM!?

    _buffer[ last ] = aSample;

    if ( ++last == bufferLength )
        last = 0;
}

float RingBuffer::dequeue()
{
    if ( _buffer == 0 )
        return randomFloat();

    float item = _buffer[ first ];

    if ( ++first == bufferLength )
        first = 0;

    return item;
}

float RingBuffer::peek()
{
    if ( _buffer == 0 )
        return randomFloat();

    return _buffer[ first ];
}
