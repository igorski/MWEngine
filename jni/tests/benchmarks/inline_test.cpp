#include "../../modules/arpeggiator.h"
#include "../../utilities/utils.h"
#include "../../global.h"
#include "../../ringbuffer.h"

// these tests were created to measure the speed enhancements
// provided by inlining these functions

TEST( RingBufferBenchmark, PeekBenchmark )
{
    int iterations = 500000;

    long long test1start;
    long long test1end;
    long long totalTest1;

    int bufferSize     = 1024;
    RingBuffer* buffer = new RingBuffer( bufferSize );

    int i, j;

    SAMPLE_TYPE value;
    test1start = now_ms();

    for ( i = 0; i < iterations; ++i )
    {
        for ( j = 0; j < bufferSize; ++j )
        {
            value = buffer->peek();
        }
    }

    test1end   = now_ms();
    totalTest1 = test1end - test1start;

    std::cout << "ringbuffer peek test " << totalTest1 << " ms for " << iterations << " iterations\n";

    delete buffer;
}

TEST( RingBufferBenchmark, EnqueueBenchmark )
{
    int iterations = 5000;

    long long test1start;
    long long test1end;
    long long totalTest1;

    int bufferSize     = 1024;
    RingBuffer* buffer = new RingBuffer( bufferSize );

    int i, j;

    SAMPLE_TYPE value;
    SAMPLE_TYPE sample = randomSample( -MAX_PHASE, MAX_PHASE );

    test1start = now_ms();

    for ( i = 0; i < iterations; ++i )
    {
        for ( j = 0; j < bufferSize; ++j )
            buffer->enqueue( sample );
    }

    test1end   = now_ms();
    totalTest1 = test1end - test1start;

    std::cout << "ringbuffer enqueue test " << totalTest1 << " ms for " << iterations << " iterations\n";

    delete buffer;
}

TEST( RingBufferBenchmark, DequeueBenchmark )
{
    int iterations = 5000;

    long long test1start;
    long long test1end;
    long long totalTest1;

    int bufferSize     = 1024;
    RingBuffer* buffer = new RingBuffer( bufferSize );

    int i, j;

    SAMPLE_TYPE value;
    test1start = now_ms();

    for ( i = 0; i < iterations; ++i )
    {
        for ( j = 0; j < bufferSize; ++j )
        {
            value = buffer->dequeue();
        }
    }

    test1end   = now_ms();
    totalTest1 = test1end - test1start;

    std::cout << "ringbuffer dequeue test " << totalTest1 << " ms for " << iterations << " iterations\n";

    delete buffer;
}

TEST( RingBufferBenchmark, FlushBenchmark )
{
    int iterations = 5000;

    long long test1start;
    long long test1end;
    long long totalTest1;

    int bufferSize     = 1024;
    RingBuffer* buffer = new RingBuffer( bufferSize );

    int i, j;

    test1start = now_ms();

    for ( i = 0; i < iterations; ++i )
    {
        for ( j = 0; j < bufferSize; ++j )
        {
            buffer->flush();
        }
    }

    test1end   = now_ms();
    totalTest1 = test1end - test1start;

    std::cout << "ringbuffer flush test " << totalTest1 << " ms for " << iterations << " iterations\n";

    delete buffer;
}

TEST( ArpeggiatorBenchmark, PeekBenchmark )
{
    int iterations = 5000000;
    Arpeggiator* arpeggiator = new Arpeggiator();

    long long test1start;
    long long test1end;
    long long totalTest1;

    int i;
    SAMPLE_TYPE value;

    test1start = now_ms();

    for ( i = 0; i < iterations; ++i )
    {
        value = arpeggiator->peek();
    }

    test1end   = now_ms();
    totalTest1 = test1end - test1start;

    std::cout << "arpeggiator peek test " << totalTest1 << " ms for " << iterations << " iterations\n";

    delete arpeggiator;
}

TEST( ArpeggiatorBenchmark, GetStepBenchmark )
{
    int iterations = 5000000;
    Arpeggiator* arpeggiator = new Arpeggiator();

    long long test1start;
    long long test1end;
    long long totalTest1;

    int i;
    SAMPLE_TYPE value;

    test1start = now_ms();

    for ( i = 0; i < iterations; ++i )
    {
        value = arpeggiator->getStep();
    }

    test1end   = now_ms();
    totalTest1 = test1end - test1start;

    std::cout << "arpeggiator get step test " << totalTest1 << " ms for " << iterations << " iterations\n";

    delete arpeggiator;
}

TEST( ArpeggiatorBenchmark, GetPitchForStepBenchmark )
{
    int iterations = 5000000;
    Arpeggiator* arpeggiator = new Arpeggiator();

    long long test1start;
    long long test1end;
    long long totalTest1;

    int i;
    SAMPLE_TYPE value;
    int step = 1;
    float pitch = randomFloat( 40.0f, 880.f );

    test1start = now_ms();

    for ( i = 0; i < iterations; ++i )
    {
        value = arpeggiator->getPitchForStep( step, pitch );
    }

    test1end   = now_ms();
    totalTest1 = test1end - test1start;

    std::cout << "arpeggiator get pitch fot step test " << totalTest1 << " ms for " << iterations << " iterations\n";

    delete arpeggiator;
}
