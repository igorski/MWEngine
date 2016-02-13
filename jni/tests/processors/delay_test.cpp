#include "../../processors/delay.h"

TEST( Delay, ConstructorArguments )
{
    AudioEngineProps::SAMPLE_RATE = 48000;

    int delayTime    = randomInt( 1, 2000);
    int maxDelayTime = randomInt( delayTime, delayTime * 2 );
    float mix        = randomFloat( 0.0f, 1.0f );
    float feedback   = randomFloat( 0.0f, 1.0f );
    int channels     = randomInt( 1, 5 );

    Delay* delay = new Delay( delayTime, maxDelayTime, mix, feedback, channels );

    EXPECT_EQ( delayTime, delay->getDelayTime() )
        << "expected delay time to be return unchanged after construction";

    EXPECT_EQ( mix, delay->getMix() )
        << "expected delay mix to be return unchanged after construction";

    EXPECT_EQ( feedback, delay->getFeedback() )
        << "expected delay feedback to be return unchanged after construction";

    delete delay;
}

TEST( Delay, GettersSetters )
{
    int delayTime    = randomInt( 1, 2000 );
    int maxDelayTime = randomInt( delayTime, delayTime * 2 );
    float mix        = randomFloat( 0.0f, 1.0f );
    float feedback   = randomFloat( 0.0f, 1.0f );
    int channels     = randomInt( 1, 5 );

    Delay* delay = new Delay( delayTime, maxDelayTime, mix, feedback, channels );

    // create new values

    int newDelayTime  = randomInt( 1, maxDelayTime - 1 );
    float newMix      = randomFloat( 0.0f, 1.0f );
    float newFeedback = randomFloat( 0.0f, 1.0f );

    delay->setDelayTime( newDelayTime );

    EXPECT_EQ( newDelayTime, delay->getDelayTime() )
        << "expected delay time to have updated after using setter method";

    delay->setMix( newMix );

    EXPECT_EQ( newMix, delay->getMix() )
        << "expected delay mix to have updated after using setter method";

    delay->setFeedback( newFeedback );

    EXPECT_EQ( newFeedback, delay->getFeedback() )
        << "expected delay feedback to have updated after using setter method";

    delete delay;
}

TEST( Delay, SetDelayTime )
{
    int delayTime    = randomInt( 1.0, 2000 );
    int maxDelayTime = randomInt( delayTime, delayTime * 2 );
    float mix        = randomFloat( 0.0f, 1.0f );
    float feedback   = randomFloat( 0.0f, 1.0f );
    int channels     = randomInt( 1, 5 );

    Delay* delay = new Delay( delayTime, maxDelayTime, mix, feedback, channels );

    float newDelayTime = maxDelayTime * 2;

    delay->setDelayTime( newDelayTime );

    EXPECT_EQ( maxDelayTime, delay->getDelayTime() )
        << "expected delay setter to have sanitized the delay value (may not exceed defined max time)";

    delete delay;
}