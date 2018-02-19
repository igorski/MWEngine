#include "../../modules/adsr.h"
#include "../../audiobuffer.h"
#include "../../global.h"
#include "../../utilities/bufferutility.h"

TEST( ADSR, Constructor ) {
    ADSR* adsr = new ADSR();

    EXPECT_EQ( 0, adsr->getAttack() )  << "expected value to be 0 by default";
    EXPECT_EQ( 0, adsr->getDecay() )   << "expected value to be 0 by default";
    EXPECT_EQ( 0, adsr->getSustain() ) << "expected value to be 0 by default";
    EXPECT_EQ( 0, adsr->getRelease() ) << "expected value to be 0 by default";

    delete adsr;
}

TEST( ADSR, ConstructorWithArguments ) {
    float attack = randomFloat();
    float decay  = randomFloat();
    float sustain = randomFloat();
    float release = randomFloat();

    const ADSR* adsr = new ADSR( attack, decay, sustain, release );

    EXPECT_EQ( attack,  adsr->getAttack() )  << "expected value to equal constructor value";
    EXPECT_EQ( decay,   adsr->getDecay() )   << "expected value to equal constructor value";
    EXPECT_EQ( sustain, adsr->getSustain() ) << "expected value to equal constructor value";
    EXPECT_EQ( release, adsr->getRelease() ) << "expected value to equal constructor value";

    delete adsr;
}

TEST( ADSR, Apply ) {
    // create a short buffer where each envelope stage will
    // last for a fourth of the total buffer length
    int bufferLength = 8;
    float attack     = BufferUtility::bufferToMilliseconds( 2, AudioEngine::SAMPLE_RATE ) / 1000;
    float decay      = BufferUtility::bufferToMilliseconds( 2, AudioEngine::SAMPLE_RATE ) / 1000;
    float sustain    = BufferUtility::bufferToMilliseconds( 2, AudioEngine::SAMPLE_RATE ) / 1000;
    float release    = BufferUtility::bufferToMilliseconds( 2, AudioEngine::SAMPLE_RATE ) / 1000;

    ADSR* adsr               = new ADSR( attack, decay, sustain, release );
    AudioBuffer* inputBuffer = new AudioBuffer( 1, bufferLength );
    SAMPLE_TYPE* buffer      = inputBuffer->getBufferForChannel( c );

    for ( int i = 0, l = audioBuffer->bufferSize; i < l; ++i )
        buffer[ i ] = 1;

    // apply ADSR envelopes
    adsr->apply( inputBuffer );

    // assert results to expected envelope increments for buffer range

    float HALF_PHASE    = MAX_PHASE / 2;
    float QUARTER_PHASE = MAX_PHASE / 4;

    // attack phase
    EXPECT_FLOAT_EQ( buffer[ 0 ], 0 );
    EXPECT_FLOAT_EQ( buffer[ 1 ], HALF_PHASE );

    // decay phase
    EXPECT_FLOAT_EQ( buffer[ 2 ], MAX_PHASE );
    EXPECT_FLOAT_EQ( buffer[ 3 ], HALF_PHASE );

    // sustain phase
    EXPECT_FLOAT_EQ( buffer[ 4 ], HALF_PHASE );
    EXPECT_FLOAT_EQ( buffer[ 5 ], HALF_PHASE );

    // release phase
    EXPECT_FLOAT_EQ( buffer[ 6 ], QUARTER_PHASE );
    EXPECT_FLOAT_EQ( buffer[ 7 ], 0.0 );

    delete adsr;
    delete buffer;
}

TEST( ADSR, Clone ) {
    float attack = randomFloat();
    float decay  = randomFloat();
    float sustain = randomFloat();
    float release = randomFloat();

    ADSR* adsr  = new ADSR( attack, decay, sustain, release );
    ADSR* clone = adsr->clone();

    EXPECT_EQ( clone->getAttack(),  adsr->getAttack() ) << "expected clone value to equal original";
    EXPECT_EQ( clone->getDecay(),   adsr->getDecay() ) << "expected clone value to equal original";
    EXPECT_EQ( clone->getSustain(), adsr->getSustain() ) << "expected clone value to equal original";
    EXPECT_EQ( clone->getRelease(), adsr->getRelease() ) << "expected clone value to equal original";

    delete adsr;
    delete clone;
}
