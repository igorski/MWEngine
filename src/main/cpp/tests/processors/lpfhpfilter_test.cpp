#include <processors/lpfhpfilter.h>

TEST( LPFHPFilter, getType )
{
    LPFHPFilter* processor = new LPFHPFilter( 40.F, 880.F, 1 );

    std::string expectedType( "LPFHPFilter" );
    ASSERT_TRUE( 0 == expectedType.compare( processor->getType() ));

    delete processor;
}
