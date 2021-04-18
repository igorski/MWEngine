#include <processors/reverbsm.h>

TEST( ReverbSM, getType )
{
    ReverbSM* processor = new ReverbSM();

    std::string expectedType( "ReverbSM" );
    ASSERT_TRUE( 0 == expectedType.compare( processor->getType() ));

    delete processor;
}
