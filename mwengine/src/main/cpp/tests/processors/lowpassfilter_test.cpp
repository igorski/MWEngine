#include <processors/lowpassfilter.h>

TEST( LowPassFilter, getType )
{
    LowPassFilter* processor = new LowPassFilter( 11025.F );

    std::string expectedType( "LowPassFilter" );
    ASSERT_TRUE( 0 == expectedType.compare( processor->getType() ));

    delete processor;
}
