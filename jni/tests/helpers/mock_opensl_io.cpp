#include "mock_opensl_io.h"
#include "../../utilities/debug.h"

OPENSL_STREAM* mock_android_OpenAudioDevice( int sr, int inchannels, int outchannels, int bufferframes )
{
    Debug::log( "mocked device opening" );

    OPENSL_STREAM *p = ( OPENSL_STREAM* ) calloc( sizeof( OPENSL_STREAM ), 1 );

    p->inchannels  = inchannels;
    p->outchannels = outchannels;
    p->sr = sr;

    AudioEngine::engine_started = true;

    return p;
}

void mock_android_CloseAudioDevice( OPENSL_STREAM *p )
{
    Debug::log( "mocked device closing" );

    if ( p != 0 )
        free( p );
}

int mock_android_AudioIn( OPENSL_STREAM *p, float *buffer, int size )
{
    AudioEngine::mock_opensl_time += ( float ) size / ( p->sr * p->inchannels );

    return size;
}

int mock_android_AudioOut( OPENSL_STREAM *p, float *buffer, int size )
{
    // AudioEngine thread will halt all unit test execution
    // android_AudioOut is called upon each iteration, here
    // we can check whether we can halt the thread

    switch ( AudioEngine::test_program )
    {
        case 0: // engine start test
        case 1: // engine tempo update test

            Debug::log( "stopping mocked engine" );
            ++AudioEngine::test_program;    // advance to next test
            AudioEngine::stop();
            break;

        case 2:

            if ( Sequencer::playing )
            {
                int currentIteration = AudioEngine::render_iterations;

                Debug::logToFile( "/sdcard/log.txt", "---- receival %d of max. %d", currentIteration, ( AudioEngine::samples_per_bar / AudioEngineProps::BUFFER_SIZE ) );

                for ( int i = 0; i < size; ++i ) {
                    // TODO : separate between mono/stereo buffer (see global.h) !!
                    Debug::logToFile( "/sdcard/log.txt", "value at %d is '%f'", i, buffer[ i ]);
                    if ( isnan( buffer[ i ]))
                    {
                        Debug::log("FUCK THIS SHIT %f occurred at buffer position %d", buffer[ i ], AudioEngine::bufferPosition);
                        AudioEngine::stop();
                        break;
                    }
                }

                // stop the engine once it has rendered an entire measure

                if ( ++AudioEngine::render_iterations > ( AudioEngine::samples_per_bar / AudioEngineProps::BUFFER_SIZE ))
                {
                    Debug::logToFile( "/sdcard/log.txt", "---- stopping engine" );
                    AudioEngine::stop();
                }
                Debug::logToFile( "/sdcard/log.txt", "--- end receival %d", currentIteration );
            }
            break;
    }
    return size;
}

float mock_android_GetTimestamp( OPENSL_STREAM *p )
{
    return AudioEngine::mock_opensl_time;
}
