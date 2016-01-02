#include "mock_opensl.h"

float mockOpenSL_time = 0.0f;

OPENSL_STREAM* mock_android_OpenAudioDevice( int sr, int inchannels, int outchannels, int bufferframes )
{
    std::cout << "mocked device opening";
    return 0;
}

void mock_android_CloseAudioDevice( OPENSL_STREAM *p )
{
    std::cout << "mocked device closing";
}

int mock_android_AudioIn( OPENSL_STREAM *p, float *buffer, int size )
{
    //mockOpenSL_time += (float) size/(p->sr*p->inchannels);
    return size;
}

int mock_android_AudioOut( OPENSL_STREAM *p, float *buffer, int size )
{
    return size;
}

float mock_android_GetTimestamp( OPENSL_STREAM *p )
{
    return mockOpenSL_time;
}
