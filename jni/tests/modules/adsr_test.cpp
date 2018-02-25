#include "../../modules/adsr.h"
#include "../../audiobuffer.h"
#include "../../global.h"
#include "../../utilities/bufferutility.h"
#include "../../events/basesynthevent.h"
#include "../../instruments/synthinstrument.h"

TEST( ADSR, Constructor ) {
    ADSR* adsr = new ADSR();

    EXPECT_EQ( 0,         adsr->getAttackTime() )   << "expected value to be 0 by default";
    EXPECT_EQ( 0,         adsr->getDecayTime() )    << "expected value to be 0 by default";
    EXPECT_EQ( MAX_PHASE, adsr->getSustainLevel() ) << "expected value to be 1.0 by default";
    EXPECT_EQ( 0,         adsr->getReleaseTime() )  << "expected value to be 0 by default";

    delete adsr;
}

TEST( ADSR, ConstructorWithArguments ) {
    float attack  = randomFloat();
    float decay   = randomFloat();
    float sustain = randomFloat();
    float release = randomFloat();

    ADSR* adsr = new ADSR( attack, decay, sustain, release );

    EXPECT_EQ( attack,  adsr->getAttackTime() )   << "expected value to equal constructor value";
    EXPECT_EQ( decay,   adsr->getDecayTime() )    << "expected value to equal constructor value";
    EXPECT_EQ( sustain, adsr->getSustainLevel() ) << "expected value to equal constructor value";
    EXPECT_EQ( release, adsr->getReleaseTime() )  << "expected value to equal constructor value";

    delete adsr;
}

TEST( ADSR, ReleaseDuration )
{
    ADSR* adsr = new ADSR();

    EXPECT_EQ( 0, adsr->getReleaseDuration() ) << "expected 0 release duration on non-specified release time";

    float releaseTime = randomFloat();
    adsr->setReleaseTime( releaseTime );

    // calculate release duration in bufer samples
    int expectedReleaseDuration = BufferUtility::millisecondsToBuffer( releaseTime * 1000, AudioEngineProps::SAMPLE_RATE );

    EXPECT_EQ( expectedReleaseDuration, adsr->getReleaseDuration() )
        << "expected release duration in samples to match the expectation";

    delete adsr;
}

TEST( ADSR, Apply ) {
    float HALF_PHASE    = MAX_PHASE / 2;
    float QUARTER_PHASE = MAX_PHASE / 4;

    int bufferLength = 8;
    SynthInstrument* instrument = new SynthInstrument();
    BaseSynthEvent* synthEvent  = new BaseSynthEvent( 440.0f, 0, bufferLength, instrument);
    synthEvent->setEventLength( bufferLength );

    ADSR* adsr = new ADSR();

    // sustain level is at half volume
    adsr->setSustainLevel( HALF_PHASE );

    // create a short buffer where each envelope stage will
    // last for a fourth of the total buffer length
    // and the release for the full buffer length
    adsr->setDurations( 2, 2, 2, 8 );

    AudioBuffer* inputBuffer = new AudioBuffer( 1, bufferLength );
    SAMPLE_TYPE* buffer      = inputBuffer->getBufferForChannel( 0 );

    // fill buffer with maximum volume samples
    for ( int i = 0; i < inputBuffer->bufferSize; ++i )
        buffer[ i ] = MAX_PHASE;

    // apply ADSR envelopes
    adsr->apply( inputBuffer, synthEvent, 0 );

    // assert results to expected envelope increments for buffer range

    // for buffer [ 1, 1, 1, 1, 1, 1, 1, 1 ]
    // we expect [ 0, 0.5, 1, 0.75, 0.5, 0.5, 0.5, 0.25 ]
dumpBufferContents(inputBuffer);
    // attack phase
    EXPECT_FLOAT_EQ( buffer[ 0 ], 0 );
    EXPECT_FLOAT_EQ( buffer[ 1 ], HALF_PHASE );

    // decay phase
    EXPECT_FLOAT_EQ( buffer[ 2 ], MAX_PHASE );
    EXPECT_FLOAT_EQ( buffer[ 3 ], HALF_PHASE + QUARTER_PHASE );

    // sustain phase
    EXPECT_FLOAT_EQ( buffer[ 4 ], HALF_PHASE );
    EXPECT_FLOAT_EQ( buffer[ 5 ], HALF_PHASE );

    // release phase
    EXPECT_FLOAT_EQ( buffer[ 6 ], HALF_PHASE );
    EXPECT_FLOAT_EQ( buffer[ 7 ], QUARTER_PHASE);

    delete adsr;
    delete inputBuffer;
    delete synthEvent;
    delete instrument;
}

TEST( ADSR, ApplyOnLiveEvent ) {
    float HALF_PHASE    = MAX_PHASE / 2;
    float QUARTER_PHASE = MAX_PHASE / 4;

    int bufferLength = 8;
    SynthInstrument* instrument = new SynthInstrument();
    BaseSynthEvent* synthEvent  = new BaseSynthEvent( 440.0f, instrument);
    synthEvent->setEventLength( bufferLength );

    ADSR* adsr = new ADSR();

    // sustain level is at half volume
    adsr->setSustainLevel( HALF_PHASE );
    // release envelope that lasts for the full buffer length

    // create a short buffer where each envelope stage will
    // last for a fourth of the total buffer length
    // and the release for the full buffer length
    adsr->setDurations( 2, 2, BufferUtility::bufferToSeconds( bufferLength, AudioEngineProps::BUFFER_SIZE ), 8 );

    AudioBuffer* inputBuffer = new AudioBuffer( 1, bufferLength );
    SAMPLE_TYPE* buffer      = inputBuffer->getBufferForChannel( 0 );

    // TEST 1. sustained live event

    // fill buffer with maximum volume samples
    for ( int i = 0; i < inputBuffer->bufferSize; ++i )
        buffer[ i ] = MAX_PHASE;

    // apply ADSR envelopes
    adsr->apply( inputBuffer, synthEvent, 0 );

    // assert results to expected envelope increments for buffer range
    // for buffer [ 1, 1, 1, 1, 1, 1, 1, 1 ]
    // we expect [ 0, 0.5, 1, 0.75, 0.5, 0.5, 0.5, 0.5 ]
    // as the release envelope is only applied when synthEvent->stop() is invoked (sets its release state)
 dumpBufferContents(inputBuffer);
    // attack phase
    EXPECT_FLOAT_EQ( buffer[ 0 ], 0 );
    EXPECT_FLOAT_EQ( buffer[ 1 ], HALF_PHASE );

    // decay phase
    EXPECT_FLOAT_EQ( buffer[ 2 ], MAX_PHASE );
    EXPECT_FLOAT_EQ( buffer[ 3 ], HALF_PHASE + QUARTER_PHASE );

    // sustain phase (longer as the release phase is only initiated when synthEvent is released
    EXPECT_FLOAT_EQ( buffer[ 4 ], HALF_PHASE );
    EXPECT_FLOAT_EQ( buffer[ 5 ], HALF_PHASE );
    EXPECT_FLOAT_EQ( buffer[ 6 ], HALF_PHASE );
    EXPECT_FLOAT_EQ( buffer[ 7 ], HALF_PHASE);

    // TEST 2. released event
    // fill buffer with maximum volume samples again
    for ( int i = 0; i < inputBuffer->bufferSize; ++i )
        buffer[ i ] = MAX_PHASE;

    // release event (by instantly stopping)
    synthEvent->stop();

    // apply ADSR envelopes
    adsr->apply( inputBuffer, synthEvent, 0 );

    // assert results to expected envelope increments for buffer range
    // for buffer [ 1, 1, 1, 1, 1, 1, 1, 1 ]
    // we expect [ 0.5, 0.4375, 0.375, 0.3124, 0.25, 0.1875, 0.125, 0.0625 ]

    // release phase
    EXPECT_FLOAT_EQ( buffer[ 0 ], HALF_PHASE );
    EXPECT_FLOAT_EQ( buffer[ 1 ], 0.4375 );

    // unprocessed buffer
    EXPECT_FLOAT_EQ( buffer[ 2 ], 0.375 );
    EXPECT_FLOAT_EQ( buffer[ 3 ], 0.3125 );
    EXPECT_FLOAT_EQ( buffer[ 4 ], QUARTER_PHASE );
    EXPECT_FLOAT_EQ( buffer[ 5 ], 0.1875 );
    EXPECT_FLOAT_EQ( buffer[ 6 ], 0.125 );
    EXPECT_FLOAT_EQ( buffer[ 7 ], 0.0625 );


    delete adsr;
    delete inputBuffer;
    delete synthEvent;
    delete instrument;
}

TEST( ADSR, LastEnvelope )
{
    float HALF_PHASE    = MAX_PHASE / 2;
    float QUARTER_PHASE = MAX_PHASE / 4;

    int bufferLength = 8;
    SynthInstrument* instrument = new SynthInstrument();
    BaseSynthEvent* synthEvent  = new BaseSynthEvent( 440.0f, instrument);
    synthEvent->setEventLength( bufferLength );

    ADSR* adsr = new ADSR();

    // sustain level is at half volume
    adsr->setSustainLevel( HALF_PHASE );

    // create a short buffer where the attack stage will
    // last for the entirety of the total buffer length
    adsr->setDurations( bufferLength, 0, 0, bufferLength );

    AudioBuffer* inputBuffer = new AudioBuffer( 1, bufferLength );
    SAMPLE_TYPE* buffer      = inputBuffer->getBufferForChannel( 0 );

    // fill buffer with maximum volume samples
    for ( int i = 0; i < inputBuffer->bufferSize; ++i )
        buffer[ i ] = MAX_PHASE;

    EXPECT_FLOAT_EQ( 0.0, synthEvent->cachedProps.envelope )
        << "expected cached envelope to be 0 at start of application";

    // apply ADSR envelopes
    adsr->apply( inputBuffer, synthEvent, 0 );

    EXPECT_FLOAT_EQ( 0.875, synthEvent->cachedProps.envelope )
        << "expected cached envelope to be 1 at end of application";

    delete adsr;
    delete inputBuffer;
    delete synthEvent;
    delete instrument;
}

TEST( ADSR, Clone ) {
    float attack  = randomFloat();
    float decay   = randomFloat();
    float sustain = randomFloat();
    float release = randomFloat();

    ADSR* adsr  = new ADSR( attack, decay, sustain, release );
    ADSR* clone = adsr->clone();

    EXPECT_EQ( clone->getAttackTime(),   adsr->getAttackTime() )   << "expected clone value to equal original";
    EXPECT_EQ( clone->getDecayTime(),    adsr->getDecayTime() )    << "expected clone value to equal original";
    EXPECT_EQ( clone->getSustainLevel(), adsr->getSustainLevel() ) << "expected clone value to equal original";
    EXPECT_EQ( clone->getReleaseTime(),  adsr->getReleaseTime() )  << "expected clone value to equal original";

    delete adsr;
    delete clone;
}
