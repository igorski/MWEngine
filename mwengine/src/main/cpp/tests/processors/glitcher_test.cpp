#include <processors/glitcher.h>

TEST( Glitcher, getType )
{
    Glitcher* processor = new Glitcher( 1, 10 );

    std::string expectedType( "Glitcher" );
    ASSERT_TRUE( 0 == expectedType.compare( processor->getType() ));

    delete processor;
}
