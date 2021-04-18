/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2020 Igor Zinken - https://www.igorski.nl
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
#include <gtest/gtest.h>
#include <ctime>
#include <cstdlib>

#include "../global.h"
using namespace MWEngine;

#include "helpers/helper.cpp"

// NOTE: the audioengine test also sets up the Mocked audio driver
#include "audioengine_test.cpp"

// Unit tests for individual actors
#include "audiobuffer_test.cpp"
#include "audiochannel_test.cpp"
#include "channelgroup_test.cpp"
#include "processingchain_test.cpp"
#include "ringbuffer_test.cpp"
#include "sequencer_test.cpp"
#include "sequencercontroller_test.cpp"
#include "wavetable_test.cpp"
#include "events/baseaudioevent_test.cpp"
#include "events/basesynthevent_test.cpp"
//#include "events/drumevent_test.cpp"
#include "events/sampleevent_test.cpp"
#include "generators/envelopegenerator_test.cpp"
#include "instruments/baseinstrument_test.cpp"
#include "instruments/synthinstrument_test.cpp"
#include "modules/adsr_test.cpp"
#include "modules/lfo_test.cpp"
#include "processors/baseprocessor_test.cpp"
#include "processors/bitcrusher_test.cpp"
#include "processors/dcoffsetfilter_test.cpp"
#include "processors/decimator_test.cpp"
#include "processors/delay_test.cpp"
#include "processors/filter_test.cpp"
#include "processors/flanger_test.cpp"
#include "processors/fm_test.cpp"
#include "processors/formantfilter_test.cpp"
#include "processors/glitcher_test.cpp"
#include "processors/limiter_test.cpp"
#include "processors/lowpassfilter_test.cpp"
#include "processors/lpfhpfilter_test.cpp"
#include "processors/phaser_test.cpp"
#include "processors/pitchshifter_test.cpp"
#include "processors/reverb_test.cpp"
#include "processors/reverbsm_test.cpp"
#include "processors/tremolo_test.cpp"
#include "processors/waveshaper_test.cpp"
#include "utilities/eventutility_test.cpp"
#include "utilities/tablepool_test.cpp"
#include "utilities/samplemanager_test.cpp"
#include "utilities/sampleutility_test.cpp"
#include "utilities/waveutil_test.cpp"
#include "utilities/volumeutil_test.cpp"
#include "deprecation_test.cpp"

// the following aren't unit tests to spot regressions, but benchmarks to test certain performance assumptions
//#include "benchmarks/buffer_test.cpp"
//#include "benchmarks/inline_test.cpp"
//#include "benchmarks/table_test.cpp"
//#include "utilities/fastmath_test.cpp"

int main( int argc, char *argv[] )
{
    srand( time( 0 )); // setup for all randomized generators

    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}
