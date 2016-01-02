#include "../../utilities/fastmath.h"
#include "../../utilities/utils.h"

TEST( FastMath, fmod )
{
    float value1 = randomFloat( 1, 100000 );
    float value2 = randomFloat( 1, 100000 );

    int iterations = 1000000;

    long long test1start;
    long long test1end;
    long long test2start;
    long long test2end;

    float stdValue, fmValue;

    // test 1 std::fmod

    test1start = now_ms();

    for ( int i = 0; i < iterations; ++i )
    {
        stdValue = std::fmod( value1, value2 );
    }

    test1end = now_ms();

    // test 2 FastMath::fmod

    test2start = now_ms();

    for ( int i = 0; i < iterations; ++i )
    {
        fmValue = FastMath::fmod( value1, value2 );
    }

    test2end = now_ms();

    long long totalTest1 = test1end - test1start;
    long long totalTest2 = test2end - test2start;

    // round to a .1 precision (FastMath modulo is less precise and can give different result to std::fmod on occassion)

    float sanitizedStdValue = floor( stdValue * 5 + .5 ) / 5;
    float sanitizedFMValue  = floor( fmValue  * 5 + .5 ) / 5;

    EXPECT_EQ( sanitizedStdValue, sanitizedFMValue )
        << "expected FastMath::fmod value to equal std::fmod value";

    ASSERT_TRUE( totalTest1 < totalTest2 )
        << "expected std::fmod (" << totalTest1 << "ms) to be faster than FastMath::fmod (" << totalTest2 << "ms) but it wasn't";

    // well that was an eye opener ;)
//    ASSERT_TRUE( totalTest1 > totalTest2 )
//        << "expected FastMath::fmod (" << totalTest2 << "ms) to be faster than std::fmod (" << totalTest1 << "ms) but it wasn't";
}

/**
 * This test doesn't test any FastMath code but tests the assumption
 * that integer modulo operations are faster than floating point modulo operations
 * this logic is used by audioengine.cpp during rendering
 */
TEST( FastMath, Modulo )
{
    float value1  = randomFloat( 1, 100000 );
    float value2  = randomFloat( 1, 100000 );
    int intValue1 = ( int ) value1;

    int iterations = 1000000;

    long long test1start;
    long long test1end;
    long long test2start;
    long long test2end;

    // test 1 integer modulo operations

    test1start = now_ms();

    int value;
    float flValue;

    for ( int i = 0; i < iterations; ++i )
    {
        // we keep an integer conversion here to mimic the usage in audioengine.cpp
        value = intValue1 % ( int ) value2;
    }

    test1end = now_ms();

    // test 2 floating point modulo operations

    test2start = now_ms();

    for ( int i = 0; i < iterations; ++i )
    {
       flValue = std::fmod( value1, value2 );
    }

    test2end = now_ms();

    long long totalTest1 = test1end - test1start;
    long long totalTest2 = test2end - test2start;

    ASSERT_TRUE( totalTest1 < totalTest2 )
        << "expected integer modulo operations to be faster than floating point modulo operations";
}
