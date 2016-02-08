#include "../../generators/envelopegenerator.h"
#include "../../wavetable.h"
#include "../../global.h"

TEST( EnvelopeGenerator, GenerateLinear )
{
    AudioEngineProps::SAMPLE_RATE = 44100;

    int length = randomInt( 24, 512 );

    SAMPLE_TYPE startAmplitude = randomSample( 0.0, MAX_PHASE );
    SAMPLE_TYPE endAmplitude   = randomSample( 0.0, MAX_PHASE );

    bool fadeIn = ( endAmplitude > startAmplitude );

    // generate envelope

    SAMPLE_TYPE* table = EnvelopeGenerator::generateLinear( length, startAmplitude, endAmplitude );

    // evaluate results

    SAMPLE_TYPE sample;
    SAMPLE_TYPE lastSample = fadeIn ? 0.0f : MAX_PHASE;

    for ( int i = 0; i < length; ++i )
    {
        sample = table[ i ];

        ASSERT_TRUE( fadeIn ? sample > lastSample : sample < lastSample )
            << "sample value " << sample << " doesn't match the expectation relative to previous value " << lastSample;

        lastSample = sample;
    }

    // clean up
    delete table;
}

TEST( EnvelopeGenerator, GenerateExponential )
{
    // generate envelope

    int length = randomInt( 25, 1024 );

    SAMPLE_TYPE* table = EnvelopeGenerator::generateExponential( length );

    // evaluate results

    EXPECT_EQ( 0.0, table[ 0 ])
        << "expected first table sample to be 0.0";

    SAMPLE_TYPE sample;
    SAMPLE_TYPE lastSample = 0.0f;

    for ( int i = 1; i < length; ++i )
    {
        sample = table[ i ];

        ASSERT_TRUE( sample > lastSample )
            << "sample value " << sample << " doesn't match the expectation relative to previous value " << lastSample;

        lastSample = sample;
    }

    // clean up
    delete table;
}
