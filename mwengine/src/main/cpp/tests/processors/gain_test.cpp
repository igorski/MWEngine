#include <processors/gain.h>
#include <global.h>

TEST( Gain, Constructors )
{
    auto processor = new Gain();

    EXPECT_EQ( MAX_VOLUME, processor->getAmount() )
        << "expected default gain to equal the maximum volume";

    delete processor;

    processor = new Gain( 0.5f );

    EXPECT_EQ( 0.5f, processor->getAmount() )
        << "expected default gain to equal the amount provided to the constructor";

    delete processor;
}

TEST( Gain, getType )
{
    auto processor = new Gain();

    std::string expectedType( "Gain" );
    ASSERT_TRUE( 0 == expectedType.compare( processor->getType() ));

    delete processor;
}

TEST( Gain, setAmount )
{
    auto processor = new Gain();

    processor->setAmount( 2.f );

    EXPECT_EQ( 2.f, processor->getAmount() )
        << "expected gain amount to equal the value passed to the setter";

    processor->setAmount( Gain::MIN_GAIN - 1 );

    EXPECT_EQ( Gain::MIN_GAIN, processor->getAmount() )
        << "expected out of range gain amount to have been capped to the minimum supported value";

    processor->setAmount( Gain::MAX_GAIN + 1 );

    EXPECT_EQ( Gain::MAX_GAIN, processor->getAmount() )
        << "expected out of range gain amount to have been capped to the maximum supported value";

    delete processor;
}
