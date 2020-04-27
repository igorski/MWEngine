/**
 * helper provides convenience methods to setup values
 * for use with unit tests
 */
#include <cstdlib>
#include <time.h>
#include "../../global.h"
#include "../../audiobuffer.h"
#include "../../events/baseaudioevent.h"
#include "../../instruments/baseinstrument.h"
#include "../../utilities/bufferutility.h"
#include "../../utilities/samplemanager.h"
#include "../../utilities/utils.h"

#define NANOS_IN_SECOND 1000000000

// ---------------------
// HELPER MATH FUNCTIONS
// ---------------------

// random integer value between given min - max range

int randomInt( int min, int max )
{
    return min + ( rand() % ( int )( max - min + 1 ));
}

// random true - false

bool randomBool()
{
    return 0 == ( rand() % 2 );
}

// return a random floating point value between given min - max range

float randomFloat( float min, float max )
{
    float f = ( float ) rand() / float( RAND_MAX );
    return ( min + f * ( max - min ));
}

// round the precision of a floating point value
// e.g. src of 7.1234 with precision of 2 becomes 7.12

float floatRounding( float src, int precision ) {
    long long des;
    double tmp;
    tmp = ( double ) src * pow( 10, precision );

    if ( tmp < 0 ) {
        //negative double
        des = ( long long ) (tmp - 0.5 );
    }
    else {
        des = ( long long )( tmp + 0.5 );
    }
    return ( float )(( float ) des * pow( 10, -precision ));
}

// return a random sample value between given min - max range

SAMPLE_TYPE randomSample( double min, double max )
{
    double f = ( double ) rand() / RAND_MAX;
    return ( SAMPLE_TYPE )( min + f * ( max - min ));
}

// verify whether given floating point values are identical enough ;)

bool compareFloat( float value1, float value2 )
{
    return std::fabs( value1 - value2 ) < std::fabs(std::min( value1, value2 )) * std::numeric_limits<double>::epsilon();
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
            buffer[ i ] = randomSample( -1.0, +1.0 );
    }
    return audioBuffer;
}

// get the maximum amplitude value from the given buffer

SAMPLE_TYPE getMaxAmpForBuffer( AudioBuffer* audioBuffer )
{
    SAMPLE_TYPE max = -1.0;

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

// dump the sample contents of AudioBuffers into the console (NOTE : this will easily
// flood the console with messages for large buffers!!)

void dumpBufferContents( SAMPLE_TYPE* buffer, int bufferSize )
{
    for ( int i = 0; i < bufferSize; ++i )
        std::cout << i << ":" << buffer[ i ] << "\n";
}

void dumpBufferContents( AudioBuffer* audioBuffer )
{
    std::cout << "\ndumping AudioBuffer with " << audioBuffer->amountOfChannels << " channel(s) of "
        << audioBuffer->bufferSize << " samples in size\n";

    for ( int c = 0; c < audioBuffer->amountOfChannels; ++c )
    {
        std::cout << "---------\n";
        std::cout << "CHANNEL " << c << ":\n";
        std::cout << "---------\n";

        SAMPLE_TYPE* buffer = audioBuffer->getBufferForChannel( c );
        dumpBufferContents( buffer, audioBuffer->bufferSize );
    }
}

long long getTime() {
    struct timespec res;
    clock_gettime(CLOCK_MONOTONIC, &res);
    return (res.tv_sec * NANOS_IN_SECOND) + res.tv_nsec;
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
    BaseInstrument* instrument = audioEvent->getInstrument();
    delete audioEvent;

    if ( instrument != nullptr )
        instrument->unregisterFromSequencer();

    delete instrument;
}

// create a BaseAudioEvent that is enqueued into the sequencer, you'll
// likely want to delete both this event and its instrument manually

BaseAudioEvent* enqueuedAudioEvent( BaseInstrument* instrument, int eventLength, int measureNum,
                                    int subdivision, int offset )
{
    BaseAudioEvent* audioEvent = new BaseAudioEvent( instrument );
    audioEvent->setEventLength( eventLength );
    audioEvent->positionEvent ( measureNum, subdivision, offset );
    audioEvent->addToSequencer();

    return audioEvent;
}

void prepareSampleManager() {
    // creates all samples that might be used by DrumEvent
    SampleManager::setSample( "kd",  randomAudioBuffer(), AudioEngineProps::SAMPLE_RATE );
    SampleManager::setSample( "kdg", randomAudioBuffer(), AudioEngineProps::SAMPLE_RATE );
    SampleManager::setSample( "st",  randomAudioBuffer(), AudioEngineProps::SAMPLE_RATE );
    SampleManager::setSample( "stg", randomAudioBuffer(), AudioEngineProps::SAMPLE_RATE );
    SampleManager::setSample( "sn",  randomAudioBuffer(), AudioEngineProps::SAMPLE_RATE );
    SampleManager::setSample( "sng", randomAudioBuffer(), AudioEngineProps::SAMPLE_RATE );
    SampleManager::setSample( "hh",  randomAudioBuffer(), AudioEngineProps::SAMPLE_RATE );
    SampleManager::setSample( "hhg", randomAudioBuffer(), AudioEngineProps::SAMPLE_RATE );
}

void dumpEventProperties( BaseAudioEvent* audioEvent )
{
    std::cout << "\n AudioEvent start: " << audioEvent->getEventStart() <<
        " end: " << audioEvent->getEventEnd() << " (length: " << audioEvent->getEventLength() << ")";
}
