#include "../../opensl_io.h"
#include "../../utilities/debug.h"

float mockOpenSL_time = 0.0f;

OPENSL_STREAM* mock_android_OpenAudioDevice( int sr, int inchannels, int outchannels, int bufferframes )
{
    Debug::log( "mocked device opening" );

    OPENSL_STREAM *p;
    p = (OPENSL_STREAM *) calloc(sizeof(OPENSL_STREAM),1);

    p->inchannels = inchannels;
    p->outchannels = outchannels;
    p->sr = sr;

    return p;
}

void mock_android_CloseAudioDevice( OPENSL_STREAM *p )
{
    Debug::log( "mocked device closing" );
}

int mock_android_AudioIn( OPENSL_STREAM *p, float *buffer, int size )
{
    //mockOpenSL_time += (float) size/(p->sr*p->inchannels);
    return size;
}

int mock_android_AudioOut( OPENSL_STREAM *p, float *buffer, int size )
{
    Debug::log( "audio out" );

    AudioEngine::stop();

    return size;
}

float mock_android_GetTimestamp( OPENSL_STREAM *p )
{
    return mockOpenSL_time;
}
