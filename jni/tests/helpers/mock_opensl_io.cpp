#include "mock_opensl_io.h"
#include "../../audioengine.h"
#include "../../sequencer.h"
#include "../../utilities/debug.h"

inline OPENSL_STREAM* mock_android_OpenAudioDevice( int sr, int inchannels, int outchannels, int bufferframes )
{
    Debug::log( "mocked device opening" );

    OPENSL_STREAM *p = ( OPENSL_STREAM* ) calloc( sizeof( OPENSL_STREAM ), 1 );

    p->inchannels  = inchannels;
    p->outchannels = outchannels;
    p->sr = sr;

    AudioEngine::engine_started = true;

    return p;
}

inline void mock_android_CloseAudioDevice( OPENSL_STREAM *p )
{
    Debug::log( "mocked device closing" );

    if ( p != 0 )
        free( p );
}

inline int mock_android_AudioIn( OPENSL_STREAM *p, float *buffer, int size )
{
    AudioEngine::mock_opensl_time += ( float ) size / ( p->sr * p->inchannels );

    return size;
}

inline int mock_android_AudioOut( OPENSL_STREAM *p, float *buffer, int size )
{
    // AudioEngine thread will halt all unit test execution
    // android_AudioOut is called upon each iteration, here
    // we can check whether we can halt the thread

    // note the output buffer is interleaved audio at this point!
    int outputChannels   = AudioEngineProps::OUTPUT_CHANNELS;
    int singleBufferSize = size / outputChannels;

    switch ( AudioEngine::test_program )
    {
        case 0: // engine start test
        case 1: // engine tempo update test

            Debug::log( "stopping mocked engine" );
            ++AudioEngine::test_program;    // advance to next test
            AudioEngine::stop();
            break;

        case 2: // output test

            if ( Sequencer::playing )
            {
                // test 1. ensure all buffer iterations are calculated accordingly

                int currentIteration = AudioEngine::render_iterations;
                int maxIterations    = AudioEngine::samples_per_bar / AudioEngineProps::BUFFER_SIZE;

                int expectedBufferPosition = (( currentIteration + 1 ) * AudioEngineProps::BUFFER_SIZE );

                if ( currentIteration == 0 )
                    AudioEngine::test_successful = true; // will be falsified by assertions below

                else if ( currentIteration == maxIterations )
                    expectedBufferPosition -= ( AudioEngine::max_buffer_position + 1 );

                if ( AudioEngine::bufferPosition != expectedBufferPosition )
                    AudioEngine::test_successful = false;

                // test 2. evaluate buffer contents

                for ( int i = 0, c = 0; i < singleBufferSize; ++i, c += outputChannels )
                {
                    // expected samples as defined in audioengine_test.cpp
                    SAMPLE_TYPE expected[] = { -1,-1,-1,-1,0,0,0,0,1,1,1,1,0,0,0,0 };
                    SAMPLE_TYPE sample     = buffer[ c ];

                    int compareOffset = (( currentIteration * AudioEngineProps::BUFFER_SIZE ) + i ) % 16;

                    if ( sample != expected[ i ])
                    {
                        Debug::log( "expected %f, got %f at output position %d for sequencer buffer position %d",
                            expected[ i ], sample, c, AudioEngine::bufferPosition );

                        AudioEngine::test_successful = false;
                        AudioEngine::stop();
                        break;
                    }
                }

                // stop the engine once it has rendered an entire measure

                if ( ++AudioEngine::render_iterations > maxIterations )
                {
                    ++AudioEngine::test_program;    // advance to next test
                    AudioEngine::stop();
                }
            }
            break;

        case 3: // loop test

            if ( Sequencer::playing )
            {
                AudioEngine::test_successful = true; // will be falsified by assertions below

                // test 3. ensure the buffers of both the event at the end of the loop and
                // at the start of the Sequencers loop have been mixed into the output buffer

                for ( int i = 0, c = 0, bufferPosition = 88100; i < singleBufferSize; ++i, ++bufferPosition, c += outputChannels )
                {
                    // 77175 being audioEvent1 start, -0.25f being audioEvent1 contents, _0.5f being audioEvent2 contents
                    SAMPLE_TYPE expected = ( bufferPosition > 77175 ) ? -0.25f : +0.5f;
                    SAMPLE_TYPE sample   = buffer[ c ];

                    if ( sample != expected )
                    {
                        Debug::log( "expected %f, got %f at iteration %d (buffer pos %d)", expected, sample, i, bufferPosition );

                        AudioEngine::test_successful = false;
                        AudioEngine::stop();
                        break;
                    }

                    if ( bufferPosition >= AudioEngine::max_buffer_position )
                        bufferPosition = AudioEngine::min_buffer_position - 1; // will be incremented at end of iteration
                }

                // stop the engine

                ++AudioEngine::test_program;    // advance to next test
                AudioEngine::stop();
            }

            break;
    }
    return size;
}

inline float mock_android_GetTimestamp( OPENSL_STREAM *p )
{
    return AudioEngine::mock_opensl_time;
}
