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

    test1start = getTime();

    for ( i = 0; i < iterations; ++i )
    {
        for ( j = 0 ; j < bufferSize; ++j )
            tempBuffer[ j ] = 0.0;
    }

    test1end   = getTime();
    test2start = getTime();

    for ( i = 0; i < iterations; ++i ) {
        memset( tempBuffer, 0, bufferSize * sizeof( SAMPLE_TYPE )); // zero bits should equal 0.0f
    }

    test2end = getTime();

    totalTest1 = test1end - test1start;
    totalTest2 = test2end - test2start;

    ASSERT_TRUE( totalTest2 < totalTest1 )
        << "expected memset to be faster than looping";

//    std::cout << "test 1 " << totalTest1 << " ms for " << iterations << " iterations\n";
//    std::cout << "test 2 " << totalTest2 << " ms for " << iterations << " iterations\n";

    delete[] tempBuffer;
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

    test1start = getTime();

    for ( i = 0; i < iterations; ++i )
    {
        for ( j = 0 ; j < bufferSize; ++j )
            tempBuffer[ j ] = 0.0;
    }

    test1end   = getTime();
    test2start = getTime();

    for ( i = 0; i < iterations; ++i )
    {
        delete[] tempBuffer;
        tempBuffer = new SAMPLE_TYPE[ bufferSize ];
        memcpy( tempBuffer, silentBuffer, bufferSize * sizeof( SAMPLE_TYPE ));
    }

    test2end = getTime();

    totalTest1 = test1end - test1start;
    totalTest2 = test2end - test2start;

    ASSERT_TRUE( totalTest2 < totalTest1 )
        << "expected memcpy (clocked at " << totalTest2 << " ) to be faster than looping (clocked at " << totalTest1 << ")";

//    std::cout << "test 1 " << totalTest1 << " ms for " << iterations << " iterations\n";
//    std::cout << "test 2 " << totalTest2 << " ms for " << iterations << " iterations\n";

    delete silentBuffer;
    delete[] tempBuffer;
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

    test1start = getTime();

    for ( i = 0; i < iterations; ++i ) {
        std::fill( tempBuffer, tempBuffer + bufferSize, 0.0 );
    }

    test1end   = getTime();
    test2start = getTime();

    for ( i = 0; i < iterations; ++i ) {
        memset( tempBuffer, 0, bufferSize * sizeof( SAMPLE_TYPE ));
    }

    test2end = getTime();

    totalTest1 = test1end - test1start;
    totalTest2 = test2end - test2start;

    ASSERT_TRUE( totalTest2 < totalTest1 )
        << "expected memset (clocked at " << totalTest2 << ") to be faster than fill (clocked at " << totalTest1 << ")";

//    std::cout << "test 1 " << totalTest1 << " ms for " << iterations << " iterations\n";
//    std::cout << "test 2 " << totalTest2 << " ms for " << iterations << " iterations\n";

    delete[] tempBuffer;
}

TEST( BufferBenchmark, LoopingVersusMemcpyCloning )
{
    int iterations = 100000;

    long long test1start;
    long long test1end;
    long long test2start;
    long long test2end;
    long long totalTest1;
    long long totalTest2;

    int bufferSize            = randomInt( 512, 8192 );
    SAMPLE_TYPE* sourceBuffer = new SAMPLE_TYPE[ bufferSize ];
    SAMPLE_TYPE* tempBuffer   = new SAMPLE_TYPE[ bufferSize ];

    int i, j;

    for ( i = 0; i < bufferSize; ++i ) {
        sourceBuffer[ i ] = randomSample( -1.0, 1.0 );
    }

    // test 4. cloning buffer contents using looping versus memcpy

    test1start = getTime();

    for ( i = 0; i < iterations; ++i )
    {
        for ( j = 0; j < bufferSize; ++j )
            tempBuffer[ j ] = sourceBuffer[ j ];
    }

    test1end   = getTime();
    test2start = getTime();

    for ( i = 0; i < iterations; ++i ) {
        memcpy( tempBuffer, sourceBuffer, bufferSize * sizeof( SAMPLE_TYPE ));
    }

    test2end = getTime();

    totalTest1 = test1end - test1start;
    totalTest2 = test2end - test2start;

    ASSERT_TRUE( totalTest2 < totalTest1 )
        << "expected memcpy (clocked at " << totalTest2 << " ) to be faster than looping (clocked at " << totalTest1 << ")";

//    std::cout << "test 1 (looping) " << totalTest1 << " ms for " << iterations << " iterations\n";
//    std::cout << "test 2 (memcpy) " << totalTest2 << " ms for " << iterations << " iterations\n";

    delete[] sourceBuffer;
    delete[] tempBuffer;
}
