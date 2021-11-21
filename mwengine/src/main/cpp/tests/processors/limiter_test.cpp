#include <processors/limiter.h>

TEST( Limiter, getType )
{
    Limiter* processor = new Limiter();

    std::string expectedType( "Limiter" );
    ASSERT_TRUE( 0 == expectedType.compare( processor->getType() ));

    delete processor;
}

TEST( Limiter, DefaultConstructor )
{
    Limiter* processor = new Limiter();
    ASSERT_TRUE( processor->getSoftKnee() );

    delete processor;
}

TEST( Limiter, NormalizedConstructor )
{
    float attack    = 0.5f;
    float release   = 0.6f;
    float threshold = 0.7f;

    Limiter* processor = new Limiter( attack, release, threshold );

    ASSERT_FLOAT_EQ( processor->getAttack(),    attack );
    ASSERT_FLOAT_EQ( processor->getRelease(),   release );
    ASSERT_FLOAT_EQ( processor->getThreshold(), threshold );
    ASSERT_TRUE( !processor->getSoftKnee() );

    delete processor;
}

TEST( Limiter, ConstructorWithTimeUnits )
{
    float attackInMicroseconds  = 600.f;
    float releaseInMilliseconds = 400.f;
    float threshold             = 0.5f;
    float softKnee              = randomBool();

    Limiter* processor = new Limiter( attackInMicroseconds, releaseInMilliseconds, threshold, softKnee );

    ASSERT_FLOAT_EQ(round( processor->getAttackMicroseconds()),  round( attackInMicroseconds ));
    ASSERT_FLOAT_EQ(round( processor->getReleaseMilliseconds()), round( releaseInMilliseconds ));
    ASSERT_FLOAT_EQ( processor->getThreshold(), threshold );
    ASSERT_TRUE( processor->getSoftKnee() == softKnee );

    delete processor;
}

TEST( Limiter, GetSetAttackNormalized )
{
    Limiter* processor = new Limiter();

    processor->setAttack( 0.99f );
    ASSERT_FLOAT_EQ( processor->getAttack(), 0.99f );

    delete processor;
}

TEST( Limiter, GetSetAttackMicroSeconds )
{
    Limiter* processor = new Limiter();

    processor->setAttack( 1.0f );
    ASSERT_FLOAT_EQ( round( processor->getAttackMicroseconds()), 1564.f );
    ASSERT_FLOAT_EQ( processor->getAttack(), 1.0f );

    processor->setAttackMicroseconds( 149.17946f );
    ASSERT_FLOAT_EQ( processor->getAttack(), 0.5f );

    delete processor;
}

TEST( Limiter, GetSetReleaseNormalized )
{
    Limiter* processor = new Limiter();

    processor->setRelease( 0.99f );
    ASSERT_FLOAT_EQ( processor->getRelease(), 0.99f );

    delete processor;
}

TEST( Limiter, GetSetReleaseMilliSeconds )
{
    Limiter* processor = new Limiter();

    processor->setRelease( 1.0f );
    ASSERT_FLOAT_EQ( round( processor->getReleaseMilliseconds()), 1570.f );

    processor->setReleaseMilliseconds( 49.699593f );
    ASSERT_FLOAT_EQ( processor->getRelease(), 0.50001144f );

    delete processor;
}

TEST( Limiter, GetSetThresholdNormalized )
{
    Limiter* processor = new Limiter();

    processor->setThreshold( 0.99f );
    ASSERT_FLOAT_EQ( processor->getThreshold(), 0.99f );

    delete processor;
}

TEST( Limiter, GetSetSoftKnee )
{
    Limiter* processor = new Limiter();

    processor->setSoftKnee( false );
    ASSERT_TRUE( !processor->getSoftKnee() );

    delete processor;
}
