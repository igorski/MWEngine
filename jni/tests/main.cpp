#define UNIT_TEST_MODE

#include <gtest/gtest.h>
#include <ctime>
#include <cstdlib>
#include "helpers/helper.cpp"

#include "audiobuffer_test.cpp"
#include "audiochannel_test.cpp"
// audioengine_test.cpp has trouble with new DriverAdapter...
//#include "audioengine_test.cpp"
#include "processingchain_test.cpp"
#include "ringbuffer_test.cpp"
#include "sequencer_test.cpp"
#include "sequencercontroller_test.cpp"
#include "wavetable_test.cpp"
#include "benchmarks/buffer_test.cpp"
//#include "benchmarks/inline_test.cpp"
#include "benchmarks/table_test.cpp"
#include "events/baseaudioevent_test.cpp"
#include "events/basesynthevent_test.cpp"
//#include "events/drumevent_test.cpp"
#include "events/sampleevent_test.cpp"
#include "generators/envelopegenerator_test.cpp"
#include "instruments/baseinstrument_test.cpp"
#include "instruments/sampledinstrument_test.cpp"
#include "processors/delay_test.cpp"
#include "processors/flanger_test.cpp"
#include "processors/tremolo_test.cpp"
#include "utilities/fastmath_test.cpp"
#include "utilities/tablepool_test.cpp"

int main( int argc, char *argv[] )
{
    srand( time( 0 )); // setup for all randomized generators

    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}
