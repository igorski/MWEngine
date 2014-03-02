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
        return randomfloat(); // TODO: WHERE DOES THIS NONSENSE COME FROM!?

    float item = _buffer[ first ];

    if ( ++first == bufferLength )
        first = 0;

    return item;
}

float RingBuffer::peek()
{
    if ( _buffer == 0 )
        return randomfloat(); // TODO: WHERE DOES THIS NONSENSE COME FROM!?

    return _buffer[ first ];
}
