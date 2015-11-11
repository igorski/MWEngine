/**
 * helper provides convenience methods to setup values
 * for use with unit tests
 */
#include <cstdlib>
#include "../../global.h"
#include "../../audiobuffer.h"
#include "../../utilities/bufferutility.h"

// random integer value between min - max range

int randomInt( int min, int max )
{
    return min + ( rand() % ( int )( max - min + 1 ));
}

// returns a random sample value

SAMPLE_TYPE randomSample( double min, double max )
{
    double f = ( double ) rand() / RAND_MAX;
    return ( SAMPLE_TYPE )( min + f * ( max - min ));
}

// generates an AudioBuffer with random channel / buffer size values

AudioBuffer* randomAudioBuffer()
{
    int amountOfChannels = randomInt( 1, 5 );
    int bufferSize       = randomInt( 64, 1024 );

    return new AudioBuffer( amountOfChannels, bufferSize );
}

// generates an AudioBuffer with random sample contents

AudioBuffer* randomFilledAudioBuffer()
{
    AudioBuffer* audioBuffer = randomAudioBuffer();

    for ( int c = 0, ca = audioBuffer->amountOfChannels; c < ca; ++ c )
    {
        SAMPLE_TYPE* buffer = audioBuffer->getBufferForChannel( c );

        for ( int i = 0, l = audioBuffer->bufferSize; i < l; ++i )
            buffer[ i ] = randomSample( -MAX_PHASE, +MAX_PHASE );
    }
    return audioBuffer;
}

// get the maximum amplitude value from the given buffer

SAMPLE_TYPE getMaxAmpForBuffer( AudioBuffer* audioBuffer )
{
    SAMPLE_TYPE max = -MAX_PHASE;

    for ( int c = 0, ca = audioBuffer->amountOfChannels; c < ca; ++ c )
    {
        SAMPLE_TYPE* buffer = audioBuffer->getBufferForChannel( c );

        for ( int i = 0, l = audioBuffer->bufferSize; i < l; ++i )
        {
            if ( buffer[ i ] > max )
                max = buffer[ i ];
        }
    }
    return max;
}
