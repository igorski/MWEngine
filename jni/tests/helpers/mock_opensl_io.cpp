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

            break;
    }
    return size;
}

float mock_android_GetTimestamp( OPENSL_STREAM *p )
{
    return AudioEngine::mock_opensl_time;
}
