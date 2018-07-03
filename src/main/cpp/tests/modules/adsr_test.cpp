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
    EXPECT_EQ( 1.0, adsr->getSustainLevel() ) << "expected value to be 1.0 by default";
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

TEST( ADSR, ApplyPositiveSustain ) {
    float HALF_PHASE    = 1.0 / 2;
    float QUARTER_PHASE = 1.0 / 4;

    int bufferLength = 8;
    SynthInstrument* instrument = new SynthInstrument();
    BaseSynthEvent* synthEvent  = new BaseSynthEvent( 440.0f, 0, 0, instrument);
    synthEvent->setEventLength( bufferLength );

    ADSR* adsr = new ADSR();

    // sustain level is at half volume
    adsr->setSustainLevel( HALF_PHASE );

    // create a short buffer where the Attack and Decay stages will
    // last for a fourth of the total buffer length (implying that the
    // Sustain stage stays for half the buffer length)
    // additionally we request a release equal to the full buffer length
    adsr->setDurations( 2, 2, bufferLength, bufferLength );

    // note we request the full event length plus the release duration
    // as a positive release time will extend the lifetime of the events playback
    AudioBuffer* inputBuffer = new AudioBuffer( 1, bufferLength + adsr->getReleaseDuration() );
    SAMPLE_TYPE* buffer      = inputBuffer->getBufferForChannel( 0 );

    // fill buffer with maximum volume samples
    for ( int i = 0; i < inputBuffer->bufferSize; ++i )
        buffer[ i ] = 1.0;

    // apply ADSR envelopes
    adsr->apply( inputBuffer, synthEvent, 0 );

    // assert results to expected envelope increments for buffer range

    // for buffer [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ]
    // we expect [ 0, 0.5, 1, 0.75, 0.5, 0.5, 0.5, 0.5, 0.5, 0.4375, 0.375, 0.3125, 0.25, 0.1875, 0.125, 0.0625 ]

    // attack phase
    EXPECT_FLOAT_EQ( buffer[ 0 ], 0 );
    EXPECT_FLOAT_EQ( buffer[ 1 ], HALF_PHASE );

    // decay phase
    EXPECT_FLOAT_EQ( buffer[ 2 ], 1.0 );
    EXPECT_FLOAT_EQ( buffer[ 3 ], 1.0 - QUARTER_PHASE );

    // sustain phase
    EXPECT_FLOAT_EQ( buffer[ 4 ], HALF_PHASE );
    EXPECT_FLOAT_EQ( buffer[ 5 ], HALF_PHASE );
    EXPECT_FLOAT_EQ( buffer[ 6 ], HALF_PHASE );
    EXPECT_FLOAT_EQ( buffer[ 7 ], HALF_PHASE );

    // release phase
    EXPECT_FLOAT_EQ( buffer[ 8 ],  HALF_PHASE );
    EXPECT_FLOAT_EQ( buffer[ 9 ],  0.4375 );
    EXPECT_FLOAT_EQ( buffer[ 10 ], 0.375 );
    EXPECT_FLOAT_EQ( buffer[ 11 ], 0.3125 );
    EXPECT_FLOAT_EQ( buffer[ 12 ], 0.25 );
    EXPECT_FLOAT_EQ( buffer[ 13 ], 0.1875 );
    EXPECT_FLOAT_EQ( buffer[ 14 ], 0.125 );
    EXPECT_FLOAT_EQ( buffer[ 15 ], 0.0625 );

    delete adsr;
    delete inputBuffer;
    delete synthEvent;
    delete instrument;
}

TEST( ADSR, ApplyZeroSustain ) {
    float HALF_PHASE    = 1.0 / 2;
    float QUARTER_PHASE = 1.0 / 4;

    int bufferLength = 16;
    SynthInstrument* instrument = new SynthInstrument();
    BaseSynthEvent* synthEvent  = new BaseSynthEvent( 440.0f, 0, 0, instrument);
    synthEvent->setEventLength( bufferLength );

    ADSR* adsr = new ADSR();

    // sustain level is at zero volume
    adsr->setSustainLevel( 0.0 );

    // create a short buffer where the Attack stage lasts for an eight of the total
    // buffer length while the Decay stage will last for a fourth of the total buffer
    // length, additionally we request a release equal to half the buffer length
    adsr->setDurations( 2, 4, 8, bufferLength );

    // note we request the full event length plus the release duration
    // as a positive release time will extend the lifetime of the events playback
    AudioBuffer* inputBuffer = new AudioBuffer( 1, bufferLength + adsr->getReleaseDuration() );
    SAMPLE_TYPE* buffer      = inputBuffer->getBufferForChannel( 0 );

    // fill buffer with maximum volume samples
    for ( int i = 0; i < inputBuffer->bufferSize; ++i )
        buffer[ i ] = 1.0;

    // apply ADSR envelopes
    adsr->apply( inputBuffer, synthEvent, 0 );

    // assert results to expected envelope increments for buffer range

    // for buffer [ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ]
    // we expect [ 0, 0.5, 1, 0.75, 0.5, 0.25, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ]

    // attack phase
    EXPECT_FLOAT_EQ( buffer[ 0 ], 0 );
    EXPECT_FLOAT_EQ( buffer[ 1 ], HALF_PHASE );

    // decay phase
    EXPECT_FLOAT_EQ( buffer[ 2 ], 1.0 );
    EXPECT_FLOAT_EQ( buffer[ 3 ], 1.0 - QUARTER_PHASE );

    // sustain phase
    EXPECT_FLOAT_EQ( buffer[ 4 ], HALF_PHASE );
    EXPECT_FLOAT_EQ( buffer[ 5 ], QUARTER_PHASE );
    EXPECT_FLOAT_EQ( buffer[ 6 ], 0 );
    EXPECT_FLOAT_EQ( buffer[ 7 ], 0 );

    // release phase
    EXPECT_FLOAT_EQ( buffer[ 8 ],  0 );
    EXPECT_FLOAT_EQ( buffer[ 9 ],  0 );
    EXPECT_FLOAT_EQ( buffer[ 10 ], 0 );
    EXPECT_FLOAT_EQ( buffer[ 11 ], 0 );
    EXPECT_FLOAT_EQ( buffer[ 12 ], 0 );
    EXPECT_FLOAT_EQ( buffer[ 13 ], 0 );
    EXPECT_FLOAT_EQ( buffer[ 14 ], 0 );
    EXPECT_FLOAT_EQ( buffer[ 15 ], 0 );

    delete adsr;
    delete inputBuffer;
    delete synthEvent;
    delete instrument;
}

TEST( ADSR, ApplyOnLiveEvent ) {
    float HALF_PHASE    = 1.0 / 2;
    float QUARTER_PHASE = 1.0 / 4;

    int bufferLength = 8;
    SynthInstrument* instrument = new SynthInstrument();
    BaseSynthEvent* synthEvent  = new BaseSynthEvent( 440.0f, instrument );
    synthEvent->setEventLength( bufferLength );

    ADSR* adsr = new ADSR();

    // sustain level is at half volume
    adsr->setSustainLevel( HALF_PHASE );
    // release envelope that lasts for the full buffer length

    // create a short buffer where each envelope stage will
    // last for a fourth of the total buffer length
    // and the release for the full buffer length
    adsr->setDurations( 2, 2, 8, 8 );

    AudioBuffer* inputBuffer = new AudioBuffer( 1, bufferLength );
    SAMPLE_TYPE* buffer      = inputBuffer->getBufferForChannel( 0 );

    // TEST 1. sustained live event

    synthEvent->play();

    // fill buffer with maximum volume samples
    for ( int i = 0; i < inputBuffer->bufferSize; ++i )
        buffer[ i ] = 1.0;

    // apply ADSR envelopes
    adsr->apply( inputBuffer, synthEvent, 0 );

    // assert results to expected envelope increments for buffer range
    // for buffer [ 1, 1, 1, 1, 1, 1, 1, 1 ]
    // we expect [ 0, 0.5, 1, 0.75, 0.5, 0.5, 0.5, 0.5 ]
    // as the release envelope is only applied when synthEvent->stop() is invoked (sets its release state)

    // attack phase
    EXPECT_FLOAT_EQ( buffer[ 0 ], 0 );
    EXPECT_FLOAT_EQ( buffer[ 1 ], HALF_PHASE );

    // decay phase
    EXPECT_FLOAT_EQ( buffer[ 2 ], 1.0 );
    EXPECT_FLOAT_EQ( buffer[ 3 ], 1.0 - QUARTER_PHASE );

    // sustain phase (longer as the release phase is only initiated when synthEvent is released
    EXPECT_FLOAT_EQ( buffer[ 4 ], HALF_PHASE );
    EXPECT_FLOAT_EQ( buffer[ 5 ], HALF_PHASE );
    EXPECT_FLOAT_EQ( buffer[ 6 ], HALF_PHASE );
    EXPECT_FLOAT_EQ( buffer[ 7 ], HALF_PHASE);

    // TEST 2. released event
    // fill buffer with maximum volume samples again
    for ( int i = 0; i < inputBuffer->bufferSize; ++i )
        buffer[ i ] = 1.0;

    // release event (by stopping)
    synthEvent->stop();
    // TODO: this next line would be set by synthEvent.stop() WTF is wrong in this test mode?
    synthEvent->cachedProps.envelopeOffset = adsr->getReleaseStartOffset();

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
    float HALF_PHASE    = 1.0 / 2;
    float QUARTER_PHASE = 1.0 / 4;

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
        buffer[ i ] = 1.0;

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
