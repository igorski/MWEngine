#include "../../utilities/bufferutility.h"
#include "../../utilities/utils.h"
#include <algorithm>

TEST( BufferBenchmark, LoopVersusMemset )
{
    int iterations = 100000;

    long long test1start;
    long long test1end;
    long long test2start;
    long long test2end;
    long long totalTest1;
    long long totalTest2;

    int bufferSize          = randomInt( 512, 8192 );
    SAMPLE_TYPE* tempBuffer = new SAMPLE_TYPE[ bufferSize ];

    int i, j;

    // test 1. creating an empty buffer by loop vs. memset

    test1start = now_ms();

    for ( i = 0; i < iterations; ++i )
    {
        for ( j = 0 ; j < bufferSize; ++j )
            tempBuffer[ j ] = 0.0;
    }

    test1end   = now_ms();
    test2start = now_ms();

    for ( i = 0; i < iterations; ++i ) {
        memset( tempBuffer, 0, bufferSize * sizeof( SAMPLE_TYPE )); // zero bits should equal 0.0f
    }

    test2end = now_ms();

    totalTest1 = test1end - test1start;
    totalTest2 = test2end - test2start;

    ASSERT_TRUE( totalTest2 < totalTest1 )
        << "expected memset to be faster than looping";

//    std::cout << "test 1 " << totalTest1 << " ms for " << iterations << " iterations\n";
//    std::cout << "test 2 " << totalTest2 << " ms for " << iterations << " iterations\n";

    delete tempBuffer;
}

TEST( BufferBenchmark, LoopingVersusMemcpy )
{
    int iterations = 100000;

    long long test1start;
    long long test1end;
    long long test2start;
    long long test2end;
    long long totalTest1;
    long long totalTest2;

    int bufferSize            = randomInt( 512, 8192 );
    SAMPLE_TYPE* silentBuffer = BufferUtility::generateSilentBuffer( bufferSize );
    SAMPLE_TYPE* tempBuffer   = new SAMPLE_TYPE[ bufferSize ];

    int i, j;

    // test 2. creating an empty buffer by loop vs. memcpy

    test1start = now_ms();

    for ( i = 0; i < iterations; ++i )
    {
        for ( j = 0 ; j < bufferSize; ++j )
            tempBuffer[ j ] = 0.0;
    }

    test1end   = now_ms();
    test2start = now_ms();

    for ( i = 0; i < iterations; ++i )
    {
        delete tempBuffer;
        tempBuffer = new SAMPLE_TYPE[ bufferSize ];
        memcpy( tempBuffer, silentBuffer, bufferSize * sizeof( SAMPLE_TYPE ));
    }

    test2end = now_ms();

    totalTest1 = test1end - test1start;
    totalTest2 = test2end - test2start;

    ASSERT_TRUE( totalTest2 < totalTest1 )
        << "expected memcpy to be faster than looping";

//    std::cout << "test 1 " << totalTest1 << " ms for " << iterations << " iterations\n";
//    std::cout << "test 2 " << totalTest2 << " ms for " << iterations << " iterations\n";

    delete silentBuffer;
    delete tempBuffer;
}

TEST( BufferBenchmark, FillVersusMemset )
{
    int iterations = 100000;

    long long test1start;
    long long test1end;
    long long test2start;
    long long test2end;
    long long totalTest1;
    long long totalTest2;

    int bufferSize          = randomInt( 512, 8192 );
    SAMPLE_TYPE* tempBuffer = new SAMPLE_TYPE[ bufferSize ];

    int i;

    // test 3. erasing buffer contents using fill versus memset

    test1start = now_ms();

    for ( i = 0; i < iterations; ++i ) {
        std::fill( tempBuffer, tempBuffer + bufferSize, 0.0 );
    }

    test1end   = now_ms();
    test2start = now_ms();

    for ( i = 0; i < iterations; ++i ) {
        memset( tempBuffer, 0, bufferSize * sizeof( SAMPLE_TYPE ));
    }

    test2end = now_ms();

    totalTest1 = test1end - test1start;
    totalTest2 = test2end - test2start;

    ASSERT_TRUE( totalTest2 < totalTest1 )
        << "expected memset to be faster than fill";

//    std::cout << "test 1 " << totalTest1 << " ms for " << iterations << " iterations\n";
//    std::cout << "test 2 " << totalTest2 << " ms for " << iterations << " iterations\n";

    delete tempBuffer;
}
