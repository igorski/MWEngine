#include <processors/flanger.h>

TEST( Flanger, ConstructorArguments )
{
    AudioEngineProps::SAMPLE_RATE = 48000;
    AudioEngineProps::BUFFER_SIZE = 8192;

    float rate     = randomFloat( 0.0f, 1.0f );
    float width    = randomFloat( 0.0f, 1.0f );
    float feedback = randomFloat( 0.0f, 1.0f );
    float delay    = randomFloat( 0.0f, 1.0f );
    float mix      = randomFloat( 0.0f, 1.0f );

    Flanger* flanger = new Flanger( rate, width, feedback, delay, mix );

    EXPECT_EQ( rate, flanger->getRate() )
        << "expected rate to be returned unchanged after construction";

    EXPECT_EQ( width, flanger->getWidth() )
        << "expected width to be returned unchanged after construction";

    EXPECT_EQ( feedback, flanger->getFeedback() )
        << "expected feedback to be returned unchanged after construction";

    EXPECT_EQ( delay, flanger->getDelay() )
        << "expected delay to be returned unchanged after construction";

    EXPECT_EQ( mix, flanger->getMix() )
        << "expected mix to be returned unchanged after construction";

    delete flanger;
}

TEST( Flanger, GettersSetters )
{
    float rate     = randomFloat( 0.0f, 1.0f );
    float width    = randomFloat( 0.0f, 1.0f );
    float feedback = randomFloat( 0.0f, 1.0f );
    float delay    = randomFloat( 0.0f, 1.0f );
    float mix      = randomFloat( 0.0f, 1.0f );

    Flanger* flanger = new Flanger( rate, width, feedback, delay, mix );

    // create new values

    float newRate     = randomFloat( 0.0f, 1.0f );
    float newWidth    = randomFloat( 0.0f, 1.0f );
    float newFeedback = randomFloat( 0.0f, 1.0f );
    float newDelay    = randomFloat( 0.0f, 1.0f );
    float newMix      = randomFloat( 0.0f, 1.0f );

    flanger->setRate( newRate );

    EXPECT_EQ( newRate, flanger->getRate() )
        << "expected rate to have updated after using setter method";

    flanger->setWidth( newWidth );

    EXPECT_EQ( newWidth, flanger->getWidth() )
        << "expected width to have updated after using setter method";

    flanger->setFeedback( newFeedback );

    EXPECT_EQ( newFeedback, flanger->getFeedback() )
        << "expected feedback to have updated after using setter method";

    flanger->setDelay( newDelay );

    EXPECT_EQ( newDelay, flanger->getDelay() )
        << "expected rate to have updated after using setter method";

    flanger->setMix( newMix );

    EXPECT_EQ( newMix, flanger->getMix() )
        << "expected rate to have updated after using setter method";

    delete flanger;
}

TEST( Flanger, getType )
{
    Flanger* processor = new Flanger();

    std::string expectedType( "Flanger" );
    ASSERT_TRUE( 0 == expectedType.compare( processor->getType() ));

    delete processor;
}