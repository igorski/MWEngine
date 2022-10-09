#include <processors/gate.h>

TEST( Gate, GetSetThreshold )
{
    Gate* processor = new Gate();

    processor->setThreshold( 2.f );

    EXPECT_EQ( 2.f, processor->getThreshold() )
        << "expected threshold to be set to provided value as it is within range";

    delete processor;
}

TEST( Gate, getType )
{
    Gate* processor = new Gate();

    std::string expectedType( "Gate" );
    ASSERT_TRUE( 0 == expectedType.compare( processor->getType() ));

    delete processor;
}