#include <processors/decimator.h>

TEST( Decimator, getType )
{
    Decimator* processor = new Decimator( 16, 0.5F );

    std::string expectedType( "Decimator" );
    ASSERT_TRUE( 0 == expectedType.compare( processor->getType() ));

    delete processor;
}
