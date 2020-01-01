/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2016-2020 Igor Zinken - https://www.igorski.nl
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
#include "mock_io.h"
#include "../audioengine.h"
#include "../sequencer.h"
#include <utilities/debug.h>

Mock_IO::Mock_IO()
{
    MockData::engine_started = true;
}

Mock_IO::~Mock_IO()
{
    // nowt...
}

int Mock_IO::writeOutput( float *buffer, int size )
{
    // AudioEngine thread will halt all unit test execution
    // android_AudioOut is called upon each iteration, here
    // we can check whether we can halt the thread

    int outputChannels   = AudioEngineProps::OUTPUT_CHANNELS;
    int singleBufferSize = size / outputChannels;

    Debug::log( "Audio Engine test %d running", MockData::test_program );

    switch ( MockData::test_program )
    {
        case 0: // engine start test
        case 1: // engine tempo update test

            ++MockData::test_program;    // advance to next test
            AudioEngine::stop();
            break;

        case 2: // output test

            if ( Sequencer::playing )
            {
                // test 1. ensure all buffer iterations are calculated accordingly
                // when this method runs the engine is writing its renderer output (and has thus
                // incremented buffer position pointers accordingly), we can thus assume that on
                // first run the current iteration is 1, not 0 (as it has completed its run)

                int currentIteration = ++MockData::render_iterations;
                int maxIterations    = ( AudioEngine::max_buffer_position - AudioEngine::min_buffer_position ) / AudioEngineProps::BUFFER_SIZE;

                int expectedBufferPosition = currentIteration * AudioEngineProps::BUFFER_SIZE;

                if ( currentIteration == 1 )
                    MockData::test_successful = true; // will be falsified by assertions below

                if ( AudioEngine::bufferPosition != expectedBufferPosition )
                    MockData::test_successful = false;

                // test 2. evaluate buffer contents

                // expected samples as defined in audioengine_test.cpp
                SAMPLE_TYPE event1buffer[] = { -1,-1,-1,-1, 0,0,0,0, 1,1,1,1, 0,0,0,0 };
                SAMPLE_TYPE event2buffer[] = { .5,.5,.5,.5, 1,1,1,1, -.5,-.5,-.5,-.5, -1,-1,-1,-1 };
                SAMPLE_TYPE event3buffer[] = { .25,.25,.25,.25, 0,0,0,0, -.25,-.25,-.25,-.25, 0,0,0,0 };

                int event2start = 16;
                int event2end   = event2start + 16;
                int event3start = 24; // event 3 ends at singleBufferSize end

                for ( int i = 0, j = 0; i < singleBufferSize; ++i, j += AudioEngineProps::OUTPUT_CHANNELS )
                {
                    // minus 1 as we are testing the last iterations output buffer
                    int sequencerPos = (( currentIteration - 1 ) * AudioEngineProps::BUFFER_SIZE ) + i;

                    // 16 == size of the expected event buffers
                    // note that the contents of float* are interleaved
                    // (every other value is a sample for the other channel, hence the j increment)
                    int readOffset = (( currentIteration * AudioEngineProps::BUFFER_SIZE ) + j ) % 16;

                    SAMPLE_TYPE leftSample  = buffer[ j ];
                    SAMPLE_TYPE rightSample = buffer[ j + 1 ];

                    SAMPLE_TYPE expectedLeftSample  = 0.f;
                    SAMPLE_TYPE expectedRightSample = 0.f;

                    // test 2.1 test event1buffer (range 0 - 16)

                    if ( sequencerPos < event2start ) {
                        // mono event will be mixed into both channels
                        expectedLeftSample = expectedRightSample = event1buffer[ sequencerPos ];
                    }
                    else if ( sequencerPos >= event2start && sequencerPos < event3start ) {
                        // stereo event that only has right channel contents
                        expectedRightSample = event2buffer[ sequencerPos - event2start ];
                    }
                    else if ( sequencerPos >= event3start )
                    {
                        // left buffer is expected to contain event 3 samples only
                        expectedLeftSample  = event3buffer[ sequencerPos - event3start ];

                        // right buffer will have overlap with tail of event 2
                        expectedRightSample = event3buffer[ sequencerPos - event3start ];

                        if ( sequencerPos <= event2end )
                            expectedRightSample += event2buffer[ sequencerPos - event2start ];
                    }

                    if ( leftSample != expectedLeftSample )
                    {
                        Debug::log( "TEST 2 expected left sample: %f, got %f at buffer readoffset %d at sequencer position %d",
                                    expectedLeftSample, leftSample, readOffset, sequencerPos );
                    }
                    else if ( rightSample != expectedRightSample )
                    {
                        Debug::log( "TEST 2 expected right sample: %f, got %f at buffer readoffset %d at sequencer position %d",
                                    expectedRightSample, rightSample, readOffset, sequencerPos );
                    }
                    else {
                        continue;
                    }
                    MockData::test_successful = false;
                    AudioEngine::stop();
                    break;
                }

                // stop the engine once it has rendered the full loop range

                if ( currentIteration == maxIterations )
                {
                    Debug::log( "Audio Engine test %d done", MockData::test_program );

                    ++MockData::test_program;    // advance to next test
                    AudioEngine::stop();
                }
            }
            break;

        case 3: // loop test

            if ( Sequencer::playing )
            {
                MockData::test_successful = true; // will be falsified by assertions below

                // test 3. ensure the buffers of both the event at the end of the loop and
                // at the start of the Sequencers loop have been mixed into the output buffer

                for ( int i = 0, c = 0, bufferPosition = 88100; i < singleBufferSize; ++i, ++bufferPosition, c += outputChannels )
                {
                    // 77175 being audioEvent1 start, -0.25f being audioEvent1 contents, _0.5f being audioEvent2 contents
                    SAMPLE_TYPE expected = ( bufferPosition > 77175 ) ? -0.25f : +0.5f;
                    // divide by amount of channels (as volume is corrected for summing purposes)
                    expected /= 2;

                    SAMPLE_TYPE sample = buffer[ c ];

                    if ( sample != expected )
                    {
                        Debug::log( "TEST 3 expected %f, got %f at iteration %d (buffer pos %d)", expected, sample, i, bufferPosition );

                        MockData::test_successful = false;
                        AudioEngine::stop();
                        break;
                    }

                    if ( bufferPosition >= AudioEngine::max_buffer_position )
                        bufferPosition = AudioEngine::min_buffer_position - 1; // will be incremented at end of iteration
                }

                // stop the engine

                ++MockData::test_program;    // advance to next test
                AudioEngine::stop();
            }

            break;
    }
    return size;
}

bool  MockData::engine_started    = false;
int   MockData::test_program      = 0;
bool  MockData::test_successful   = false;
int   MockData::render_iterations = 0;
