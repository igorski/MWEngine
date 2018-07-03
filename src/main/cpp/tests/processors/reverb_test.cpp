#include "../../processors/reverb.h"

TEST( Reverb, GetSetSize )
{
    Reverb* reverb = new Reverb( 0.f, 0.f, 0.f, 0.f );

    float min = 0.f;
    float max = 1.0;

    float value = randomFloat( min, max );

    reverb->setSize( value );
    EXPECT_EQ( value, reverb->getSize() ) << "expected set size to have been returned unchanged";

    reverb->setSize( -max );
    EXPECT_EQ( min, reverb->getSize() ) << "expected size to have been normalized to defined min value";

    reverb->setSize( max * 2 );
    EXPECT_EQ( max, reverb->getSize() ) << "expected size to have been normalized to defined max value";

    delete reverb;
}

TEST( Reverb, GetSetHFDamp )
{
    Reverb* reverb = new Reverb( 0.f, 0.f, 0.f, 0.f );

    float min = 0.f;
    float max = 1.0;

    float value = randomFloat( min, max );

    reverb->setHFDamp( value );
    EXPECT_EQ( value, reverb->getHFDamp() ) << "expected set damp to have been returned unchanged";

    reverb->setHFDamp( -max );
    EXPECT_EQ( min, reverb->getHFDamp() ) << "expected damp to have been normalized to defined min value";

    reverb->setHFDamp( max * 2 );
    EXPECT_EQ( max, reverb->getHFDamp() ) << "expected damp to have been normalized to defined max value";

    delete reverb;
}

TEST( Reverb, GetSetMix )
{
    Reverb* reverb = new Reverb( 0.f, 0.f, 0.f, 0.f );

    float min = 0.f;
    float max = 1.0;

    float value = randomFloat( min, max );

    reverb->setMix( value );
    EXPECT_EQ( value, reverb->getMix() ) << "expected set mix to have been returned unchanged";

    reverb->setMix( -max );
    EXPECT_EQ( min, reverb->getMix() ) << "expected mix to have been normalized to defined min value";

    reverb->setMix( max * 2 );
    EXPECT_EQ( max, reverb->getMix() ) << "expected mix to have been normalized to defined max value";

    delete reverb;
}

TEST( Reverb, GetSetOutput )
{
    Reverb* reverb = new Reverb( 0.f, 0.f, 0.f, 0.f );

    float min = 0.f;
    float max = 1.0;

    float value = randomFloat( min, max );

    reverb->setOutput( value );
    EXPECT_EQ( value, reverb->getOutput() ) << "expected set output to have been returned unchanged";

    reverb->setOutput( -max );
    EXPECT_EQ( min, reverb->getOutput() ) << "expected output to have been normalized to defined min value";

    reverb->setOutput( max * 2 );
    EXPECT_EQ( max, reverb->getOutput() ) << "expected output to have been normalized to defined max value";

    delete reverb;
}
