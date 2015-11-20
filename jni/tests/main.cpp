#include <gtest/gtest.h>
#include <ctime>
#include <cstdlib>
#include "helpers/helper.cpp"

#include "audiobuffer_test.cpp"
#include "audiochannel_test.cpp"
#include "events/baseaudioevent_test.cpp"
#include "events/basesynthevent_test.cpp"
//#include "events/drumevent_test.cpp"
//#include "events/sampleevent_test.cpp"
#include "instruments/baseinstrument_test.cpp"
//#include "processors/tremolo_test.cpp"

int main( int argc, char *argv[] )
{
    srand( time( 0 )); // setup for all randomized generators

    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}
