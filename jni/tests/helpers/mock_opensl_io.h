#ifndef MOCK_OPENSL_IO
#define MOCK_OPENSL_IO

#include "../../opensl_io.h"
#include "mock_opensl_io.cpp"

// we don't need to have an actual OpenSL connection when
// testing the AudioEngine, here we mock the calls to the OpenSL IO so
// we can intercept them in our unit tests (audioengine_test.cpp)

#define android_OpenAudioDevice( a, b, c, d ) mock_android_OpenAudioDevice( a, b, c, d )
#define android_CloseAudioDevice( a ) mock_android_CloseAudioDevice( a );
#define android_AudioIn( a, b, c ) mock_android_AudioIn( a, b, c );
#define android_AudioOut( a, b, c ) mock_android_AudioOut( a, b, c );
#define android_GetTimestamp( a ) mock_android_GetTimestamp( a );

extern int lastIteration;

#endif
