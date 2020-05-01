#include <processors/filter.h>

TEST( Filter, ConstructorArguments )
{
    float minFreq   = randomFloat( 40.f, 440.f );
    float maxFreq   = randomFloat( 880.f, 22050.f );
    float cutoff    = randomFloat( minFreq, maxFreq );
    float resonance = randomFloat( 0.1f, 10.f );
    int channels    = randomInt( 1, 2 );

    Filter* filter = new Filter( cutoff, resonance, minFreq, maxFreq, channels );

    EXPECT_FLOAT_EQ( cutoff, filter->getCutoff() )
        << "Expected getter value to equal the constructor value";

    EXPECT_FLOAT_EQ( resonance, filter->getResonance() )
        << "Expected getter value to equal the constructor value";

    delete filter;
}

TEST( Filter, ConstructorEmpty )
{
    Filter* filter = new Filter();

    // max cutoff is at one eight of the engine sample rate

    float maxFreq = AudioEngineProps::SAMPLE_RATE / 8;

    EXPECT_FLOAT_EQ( maxFreq, filter->getCutoff() )
        << "Expected filter frequency by default to equal the maximum expected";

    delete filter;
}

TEST( Filter, SetCutOff )
{
    float minFreq   = randomFloat( 40.f, 440.f );
    float maxFreq   = randomFloat( 880.f, 22050.f );
    float cutoff    = randomFloat( minFreq, maxFreq );
    float resonance = randomFloat( 0.1f, 10.f );
    int channels    = randomInt( 1, 2 );

    Filter* filter = new Filter( cutoff, resonance, minFreq, maxFreq, channels );

    float newCutoff = randomFloat( minFreq, maxFreq );
    filter->setCutoff( newCutoff );

    EXPECT_FLOAT_EQ( newCutoff, filter->getCutoff() )
        << "Expected getter value to equal the set value";

    delete filter;
}

TEST( Filter, SetResonance )
{
    float minFreq   = randomFloat( 40.f, 440.f );
    float maxFreq   = randomFloat( 880.f, 22050.f );
    float cutoff    = randomFloat( minFreq, maxFreq );
    float resonance = randomFloat( 0.1f, 10.f );
    int channels    = randomInt( 1, 2 );

    Filter* filter = new Filter( cutoff, resonance, minFreq, maxFreq, channels );

    float newResonance = randomFloat( 20.f, 22050.f );
    filter->setResonance( newResonance );

    EXPECT_FLOAT_EQ( newResonance, filter->getResonance() )
        << "Expected getter value to equal the set value";

    delete filter;
}

TEST( Filter, LFO )
{
    float minFreq   = randomFloat( 40.f, 440.f );
    float maxFreq   = randomFloat( 880.f, 22050.f );
    float cutoff    = randomFloat( minFreq, maxFreq );
    float resonance = randomFloat( 0.1f, 10.f );
    int channels    = randomInt( 1, 2 );

    Filter* filter = new Filter( cutoff, resonance, minFreq, maxFreq, channels );

    ASSERT_FALSE( filter->hasLFO() )
        << "Expected Filter not to have an LFO by default";

    LFO* lfo = new LFO();
    filter->setLFO( lfo );

    ASSERT_TRUE( filter->hasLFO() )
        << "Expected Filter to have an LFO";

    filter->setLFO( 0 );

    ASSERT_FALSE( filter->hasLFO() )
        << "Expected Filter not to have an LFO after unset";

    delete filter;
    delete lfo;
}

TEST( Filter, IsCacheable )
{
    float minFreq   = randomFloat( 40.f, 440.f );
    float maxFreq   = randomFloat( 880.f, 22050.f );
    float cutoff    = randomFloat( minFreq, maxFreq );
    float resonance = randomFloat( 0.1f, 10.f );
    int channels    = randomInt( 1, 2 );

    Filter* filter = new Filter( cutoff, resonance, minFreq, maxFreq, channels );

    ASSERT_TRUE( filter->isCacheable() )
        << "expected Filter without an LFO to be cacheable";

    LFO* lfo = new LFO();
    filter->setLFO( lfo );

    ASSERT_FALSE( filter->isCacheable() )
        << "expected Filter with an LFO not to be cacheable";

    delete filter;
    delete lfo;
}

TEST( Filter, getType )
{
    Filter* processor = new Filter();

    std::string expectedType( "Filter" );
    ASSERT_TRUE( 0 == expectedType.compare( processor->getType() ));

    delete processor;
}