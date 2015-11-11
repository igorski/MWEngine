#include "../../processors/tremolo.h"

TEST( Tremolo, Construction )
{
    int leftWaveForm     = randomInt( 0, 5 );
    int rightWaveForm    = randomInt( 0, 5 );
    float leftFrequency  = ( float ) randomSample( 20, 22050 );
    float rightFrequency = ( float ) randomSample( 20, 22050 );

    Tremolo* tremolo = new Tremolo( leftWaveForm, rightWaveForm, leftFrequency, rightFrequency );

    ASSERT_EQ( tremolo->getWaveFormForChannel( 0 ),  leftWaveForm );
    ASSERT_EQ( tremolo->getWaveFormForChannel( 1 ),  rightWaveForm );
    ASSERT_EQ( tremolo->getFrequencyForChannel( 0 ), leftFrequency );
    ASSERT_EQ( tremolo->getFrequencyForChannel( 1 ), rightFrequency );

    delete tremolo;
}

TEST( Tremolo, IsStereo )
{
    // test with equal waveforms and frequencies

    int leftWaveForm     = 1;
    int rightWaveForm    = 1;
    float leftFrequency  = 4000;
    float rightFrequency = 4000;

    Tremolo* tremolo = new Tremolo( leftWaveForm, rightWaveForm, leftFrequency, rightFrequency );

    ASSERT_FALSE( tremolo->isStereo() ) << "expected tremolo to be operating in mono";

    // test with equal frequencies
    delete tremolo;

    rightWaveForm = 2;
    tremolo = new Tremolo( leftWaveForm, rightWaveForm, leftFrequency, rightFrequency );

    ASSERT_TRUE( tremolo->isStereo() ) << "expected tremolo to be operating in stereo";

    // test with equal waveforms
    delete tremolo;

    rightWaveForm = 1;
    rightFrequency = 5000;

    tremolo = new Tremolo( leftWaveForm, rightWaveForm, leftFrequency, rightFrequency );

    ASSERT_TRUE( tremolo->isStereo() ) << "expected tremolo to be operating in stereo";

    delete tremolo;
}
