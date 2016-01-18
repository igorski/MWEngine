#include "../../generators/envelopegenerator.h"
#include "../../wavetable.h"
#include "../../global.h"

TEST( EnvelopeGenerator, Generation )
{
    AudioEngineProps::SAMPLE_RATE = 44100;

    int length = randomInt( 24, 512 );
    float freq = randomFloat() * 440;

    WaveTable* table = new WaveTable( length, freq );

    SAMPLE_TYPE startAmplitude = randomSample( 0.0, MAX_PHASE );
    SAMPLE_TYPE endAmplitude   = randomSample( 0.0, MAX_PHASE );

    bool fadeIn = ( endAmplitude > startAmplitude );

    // generate envelope

    EnvelopeGenerator::generate( table, startAmplitude, endAmplitude );

    // evaluate results

    SAMPLE_TYPE sample;
    SAMPLE_TYPE lastSample = fadeIn ? 0.0f : MAX_PHASE;

    for ( int i = 0; i < length; ++i )
    {
        sample = table->getBuffer()[ i ];

        ASSERT_TRUE( fadeIn ? sample > lastSample : sample < lastSample )
            << "sample value " << sample << " doesn't match the expectation relative to " << lastSample;
    }

    // clean up

    delete table;
}
