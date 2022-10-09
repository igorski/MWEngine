#include <processors/compressor.h>

TEST( Compressor, GetSetThreshold )
{
    Compressor* processor = new Compressor();

    processor->setThreshold( -50.f );

    EXPECT_EQ( Compressor::MIN_THRESHOLD_VALUE, processor->getThreshold() )
        << "expected negative threshold to be capped to the minimum allowed value";

    processor->setThreshold( 30.f );

    EXPECT_EQ( Compressor::MAX_THRESHOLD_VALUE, processor->getThreshold() )
        << "expected positive threshold to be capped to the maximum allowed value";

    processor->setThreshold( 11.f );

    EXPECT_EQ( 11.f, processor->getThreshold() )
        << "expected threshold to be set to provided value as it is within range";

    delete processor;
}

TEST( Compressor, GetSetRatio )
{
    Compressor* processor = new Compressor();

    processor->setRatio( -1.0f );

    EXPECT_EQ( 0.0f, processor->getRatio() )
        << "expected ratio to be capped at 0 as negative values are not allowed";

    processor->setRatio( 2.0f );

    EXPECT_EQ( 2.0f, processor->getRatio() )
        << "expected ratio to be set to provided value as it is within range";

    delete processor;
}

TEST( Compressor, getType )
{
    Compressor* processor = new Compressor();

    std::string expectedType( "Compressor" );
    ASSERT_TRUE( 0 == expectedType.compare( processor->getType() ));

    delete processor;
}