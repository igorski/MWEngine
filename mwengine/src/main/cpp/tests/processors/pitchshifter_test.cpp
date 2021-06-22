#include <processors/pitchshifter.h>

TEST( PitchShifter, getType )
{
    PitchShifter* processor = new PitchShifter( 1.0F, 1L );

    std::string expectedType( "PitchShifter" );
    ASSERT_TRUE( 0 == expectedType.compare( processor->getType() ));

    delete processor;
}
