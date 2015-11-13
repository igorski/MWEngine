/**
 * helper provides convenience methods to setup values
 * for use with unit tests
 */
#include <cstdlib>
#include "../../global.h"
#include "../../audiobuffer.h"
#include "../../events/baseaudioevent.h"
#include "../../instruments/baseinstrument.h"
#include "../../utilities/bufferutility.h"

// ---------------------
// HELPER MATH FUNCTIONS
// ---------------------

// random integer value between min - max range

int randomInt( int min, int max )
{
    return min + ( rand() % ( int )( max - min + 1 ));
}

// random true - false

bool randomBool()
{
    return 0 == ( rand() % 2 );
}

// return a random floating point value

float randomFloat()
{
    return rand() / float( RAND_MAX );
}

// return a random sample value

SAMPLE_TYPE randomSample( double min, double max )
{
    double f = ( double ) rand() / RAND_MAX;
    return ( SAMPLE_TYPE )( min + f * ( max - min ));
}

// ----------------------------
// HELPER AUDIOBUFFER FUNCTIONS
// ----------------------------

// generates an AudioBuffer with random channel / buffer size values

AudioBuffer* randomAudioBuffer()
{
    int amountOfChannels = randomInt( 1, 5 );
    int bufferSize       = randomInt( 64, 1024 );

    return new AudioBuffer( amountOfChannels, bufferSize );
}

// generates an AudioBuffer that is at least stereo (2 channels min)

AudioBuffer* randomMultiChannelBuffer()
{
    int amountOfChannels = randomInt( 2, 5 );
    int bufferSize       = randomInt( 64, 1024 );

    return new AudioBuffer( amountOfChannels, bufferSize );
}

// fill a given AudioBuffer with random sample contents

AudioBuffer* fillAudioBuffer( AudioBuffer* audioBuffer )
{
    for ( int c = 0, ca = audioBuffer->amountOfChannels; c < ca; ++c )
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

    for ( int c = 0, ca = audioBuffer->amountOfChannels; c < ca; ++c )
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

// check whether a given AudioBuffer has sample content (e.g. isn't only silent)

bool bufferHasContent( AudioBuffer* audioBuffer )
{
    for ( int c = 0, ca = audioBuffer->amountOfChannels; c < ca; ++c )
    {
        SAMPLE_TYPE* buffer = audioBuffer->getBufferForChannel( c );

        for ( int i = 0, l = audioBuffer->bufferSize; i < l; ++i )
        {
            if ( buffer[ i ] != 0.0 )
                return true;
        }
    }
    return false;
}

// ----------------------------
// HELPER AUDIO EVENT FUNCTIONS
// ----------------------------

// create a BaseAudioEvent (NOTE: delete using "deleteAudioEvent")

BaseAudioEvent* randomAudioEvent()
{
    BaseAudioEvent* audioEvent = new BaseAudioEvent( new BaseInstrument() );
    return audioEvent;
}

void deleteAudioEvent( BaseAudioEvent* audioEvent )
{
    // instrument was created by helper, BaseAudioEvent destructors don't
    // dispose Instruments (as they shouldn't!), do it here
    delete audioEvent->getInstrument();
    delete audioEvent;
}
