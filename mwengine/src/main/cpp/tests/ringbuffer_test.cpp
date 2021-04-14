#include "../ringbuffer.h"

TEST( RingBuffer, Constructor )
{
    int capacity = randomInt( 2, 256 );
    RingBuffer* buffer = new RingBuffer( capacity );

    EXPECT_EQ( capacity, buffer->getBufferLength() )
        << "expected ring buffer length to equal the capacity passed in the constructor";

    EXPECT_EQ( 0, buffer->getSize() )
        << "expected ring buffer size to be 0 upon construction";

    ASSERT_TRUE( buffer->isEmpty() )
        << "expected ring buffer to be empty upon construction";

    ASSERT_FALSE( buffer->isFull() )
        << "expected ring buffer not to be full upon construction";

    delete buffer;
}

TEST( RingBuffer, Enqueue )
{
    int capacity = randomInt( 2, 256 );
    RingBuffer* buffer = new RingBuffer( capacity );

    for ( int i = 0; i < capacity; ++i )
    {
        buffer->enqueue( randomSample( -1.0, 1.0 ));

        if ( i < ( capacity - 1 ))
        {
            EXPECT_EQ( i, buffer->getSize() - 1 )
                << "expected ring buffer size to equal " << i << ", got " << ( buffer->getSize() - 1 ) << " instead";
        }
        else {
            EXPECT_EQ( 0, buffer->getSize() )
                << "expected ring buffer size to be 0 after reaching max. capacity during enqueing";
        }
    }
    delete buffer;
}

TEST( RingBuffer, Dequeue )
{
    int capacity = randomInt( 2, 256 );
    RingBuffer* buffer = new RingBuffer( capacity );
    SAMPLE_TYPE samples[ capacity ];

    // ensure buffer has sample contents
    for ( int i = 0; i < capacity; ++i )
    {
        samples[ i ] = randomSample( -1.0, 1.0 );
        buffer->enqueue( samples[ i ]);
    }

    for ( int i = 0; i < capacity; ++i )
    {
        SAMPLE_TYPE value = buffer->dequeue();

        EXPECT_EQ( samples[ i ], value )
            << "expected dequeued sample to equal " << samples[ i ] << " got " << value << " instead";
    }
    delete buffer;
}

TEST( RingBuffer, Peek )
{
    int capacity = randomInt( 2, 256 );
    RingBuffer* buffer = new RingBuffer( capacity );
    SAMPLE_TYPE samples[ capacity ];

    // ensure buffer has sample contents
    for ( int i = 0; i < capacity; ++i )
    {
        samples[ i ] = randomSample( -1.0, 1.0 );
        buffer->enqueue( samples[ i ]);
    }

    SAMPLE_TYPE expected;

    for ( int i = 0; i < capacity; ++i  )
    {
        buffer->dequeue();
        SAMPLE_TYPE value = buffer->peek();

        if ( i < ( capacity - 1 ))
        {
            expected = samples[ i + 1 ];
        }
        else {
            expected = samples[ 0 ];
        }
        EXPECT_EQ( expected, value )
            << "expected peeked sample at index " << i << " to equal " << expected << ", got " << value << " instead";
    }
    delete buffer;
}

TEST( RingBuffer, Flush )
{
    int capacity = randomInt( 2, 256 );
    RingBuffer* buffer = new RingBuffer( capacity );
    SAMPLE_TYPE samples[ capacity ];

    // ensure buffer has sample contents
    for ( int i = 0; i < capacity; ++i )
    {
        samples[ i ] = randomSample( -1.0, 1.0 );
        buffer->enqueue( samples[ i ]);
    }

    // flush the ring buffer

    buffer->flush();

    // ensure buffer sample values are at 0.0

    for ( int i = 0; i < capacity; ++i  )
    {
        buffer->dequeue();
        SAMPLE_TYPE value = buffer->peek();

        EXPECT_EQ( 0.0, value )
            << "expected peeked sample to equal 0.0 after flushing of ring buffer contents";
    }
    delete buffer;
}
