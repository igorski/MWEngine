#include "../../utilities/utils.h"
#include "../../global.h"
#include "../../wavetable.h"

TEST( TableBenchmark, TableLookupVersusLiveCalculation )
{
    int sampleRate = 44100;
    AudioEngineProps::SAMPLE_RATE = sampleRate;

    int iterations = 5000;

    long long test1start;
    long long test1end;
    long long test2start;
    long long test2end;
    long long test3start;
    long long test3end;
    long long totalTest1;
    long long totalTest2;
    long long totalTest3;

    int bufferSize          = 1024;
    float frequency         = randomFloat() * 440.0f;
    WaveTable* table        = new WaveTable( bufferSize, frequency );
    SAMPLE_TYPE* tempBuffer = table->getBuffer();

    SAMPLE_TYPE SR_OVER_LENGTH = ( SAMPLE_TYPE ) sampleRate / ( SAMPLE_TYPE ) bufferSize;
    SAMPLE_TYPE phaseStep      = ( frequency / ( SAMPLE_TYPE ) sampleRate ) * TWO_PI;
    SAMPLE_TYPE phase          = 0.0, tmp, amp;
    int i, j;

    // test generating a sine wave on the fly

    test1start = now_ms();

    for ( i = 0; i < iterations; ++i )
    {
        for ( j = 0; j < bufferSize; ++j )
        {
            // fast sine calculation

            if ( phase < .5 )
            {
                tmp = ( phase * 4.0 - 1.0 );
                amp = ( 1.0 - tmp * tmp );
            }
            else {
                tmp = ( phase * 4.0 - 3.0 );
                amp = ( tmp * tmp - 1.0 );
            }
            // write sine sample
            tempBuffer[ j ] = amp;

            // increment phase
            phase += phaseStep;

            // and be sure to keep phase within range (-MAX_PHASE to +MAX_PHASE)
            if ( phase > MAX_PHASE )
                phase -= MAX_PHASE;
        }
    }

    test1end   = now_ms();
    test2start = now_ms();

    // test reading from the table

    for ( i = 0; i < iterations; ++i )
    {
        for ( j = 0; j < bufferSize; ++j ) {
            amp = table->peek();
        }
    }

    test2end = now_ms();

    // additional test : test reading from the table using inline table (omits need for repeated function calls)

    int readOffset, accumulator = 0;
    test3start = now_ms();

    for ( i = 0; i < iterations; ++i )
    {
        for ( j = 0; j < bufferSize; ++j )
        {
            readOffset = 0;   // the wave table offset to read from

            if ( accumulator == 0 )
                readOffset = 0;
            else
                readOffset = ( int ) ( accumulator / SR_OVER_LENGTH );

            // increment the accumulators read offset
            accumulator += frequency;

            // keep the accumulator in the bounds of the sample frequency
            if ( accumulator > sampleRate )
                 accumulator -= sampleRate;

            // return the sample present at the calculated offset within the table
            amp = tempBuffer[ readOffset ];
        }
    }

    test3end = now_ms();

    totalTest1 = test1end - test1start;
    totalTest2 = test2end - test2start;
    totalTest3 = test3end - test3start;

    std::cout << "test 1 " << totalTest1 << " ms for " << iterations << " iterations\n";
    std::cout << "test 2 " << totalTest2 << " ms for " << iterations << " iterations\n";
    std::cout << "test 3 " << totalTest3 << " ms for " << iterations << " iterations\n";

    ASSERT_TRUE( totalTest2 > totalTest1 )
        << "expected table lookup to be slower than inline generation due to repeated function calls";

    ASSERT_TRUE( totalTest3 < totalTest1 )
        << "expected inline table lookup to be faster than inline generation";

    delete table;
}
