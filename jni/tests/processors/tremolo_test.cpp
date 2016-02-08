#include "../../processors/tremolo.h"
#include "../../global.h"

TEST( Tremolo, Construction )
{
    AudioEngineProps::SAMPLE_RATE = 44100;

    int leftType  = randomInt( 0, 1 ); // 0 = LINEAR, 1 = EXPONENTIAL
    int rightType = randomInt( 0, 1 ); // 0 = LINEAR, 1 = EXPONENTIAL

    // envelope property values are in milliseconds

    int leftAttack  = randomInt( 1, 1000 );
    int rightAttack = randomInt( 1, 1000 );
    int leftDecay   = randomInt( 1, 1000 );
    int rightDecay  = randomInt( 1, 1000 );

    Tremolo* tremolo = new Tremolo( leftType, leftAttack, leftDecay, rightType, rightAttack, rightDecay  );

    ASSERT_EQ( leftAttack, tremolo->getLeftAttack() )
        << "expected tremolo left attack to equal the value given to the constructor";

    ASSERT_EQ( rightAttack, tremolo->getRightAttack() )
        << "expected tremolo right attack to equal the value given to the constructor";

    ASSERT_EQ( leftDecay, tremolo->getLeftDecay() )
        << "expected tremolo left decay to equal the value given to the constructor";

    ASSERT_EQ( rightDecay, tremolo->getRightDecay() )
        << "expected tremolo right decay to equal the value given to the constructor";
        
    delete tremolo;
}

TEST( Tremolo, GettersSetters )
{
    AudioEngineProps::SAMPLE_RATE = 44100;

    int leftType  = randomInt( 0, 1 ); // 0 = LINEAR, 1 = EXPONENTIAL
    int rightType = randomInt( 0, 1 ); // 0 = LINEAR, 1 = EXPONENTIAL

    // values are in milliseconds
    int leftAttack  = randomInt( 1, 1000 );
    int rightAttack = randomInt( 1, 1000 );
    int leftDecay   = randomInt( 1, 1000 );
    int rightDecay  = randomInt( 1, 1000 );

    Tremolo* tremolo = new Tremolo( leftType, leftAttack, leftDecay, rightType, rightAttack, rightDecay  );
    
    // randomize values

    leftAttack  = randomInt( 1, 1000 );
    rightAttack = randomInt( 1, 1000 );
    leftDecay   = randomInt( 1, 1000 );
    rightDecay  = randomInt( 1, 1000 );
    
    tremolo->setLeftAttack( leftAttack );
    tremolo->setRightAttack( rightAttack );
    tremolo->setLeftDecay( leftDecay );
    tremolo->setRightDecay( rightDecay );
    
    ASSERT_EQ( leftAttack, tremolo->getLeftAttack() )
        << "expected tremolo left attack to equal the value given to the setter";

    ASSERT_EQ( rightAttack, tremolo->getRightAttack() )
        << "expected tremolo right attack to equal the value given to the setter";

    ASSERT_EQ( leftDecay, tremolo->getLeftDecay() )
        << "expected tremolo left decay to equal the value given to the setter";

    ASSERT_EQ( rightDecay, tremolo->getRightDecay() )
        << "expected tremolo right decay to equal the value given to the setter";
        
    delete tremolo;
}

TEST( Tremolo, IsStereo )
{
    AudioEngineProps::SAMPLE_RATE = 44100;

    // test with equal waveforms and envelopes

    int leftType    = 1;
    int rightType   = 1;
    int leftAttack  = 10;
    int rightAttack = 10;
    int leftDecay   = 10;
    int rightDecay  = 10;

    Tremolo* tremolo = new Tremolo( leftType, leftAttack, leftDecay, rightType, rightAttack, rightDecay  );

    ASSERT_FALSE( tremolo->isStereo() ) << "expected tremolo to be operating in mono";

    // test 1. unequal attack

    tremolo->setRightAttack( 11 );
    ASSERT_TRUE( tremolo->isStereo() ) << "expected tremolo to be operating in stereo as attack is unequal";
    tremolo->setRightAttack( rightAttack ); // restore

    // test 2. unequal decay

    tremolo->setLeftDecay( 10 );
    ASSERT_TRUE( tremolo->isStereo() ) << "expected tremolo to be operating in stereo as decay is unequal";

    // test. 3 unequal types

    delete tremolo;

    rightType = 0;
    tremolo = new Tremolo( leftType, leftAttack, leftDecay, rightType, rightAttack, rightDecay  );

    ASSERT_TRUE( tremolo->isStereo() ) << "expected tremolo to be operating in stereo as type is unequal";

    delete tremolo;
}
