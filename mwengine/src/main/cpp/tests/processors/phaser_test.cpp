#include <processors/phaser.h>

TEST( Phaser, getType )
{
    Phaser* processor = new Phaser( 1.F, 0.5F, 0.5F, 40.F, 880.F, 1 );

    std::string expectedType( "Phaser" );
    ASSERT_TRUE( 0 == expectedType.compare( processor->getType() ));

    delete processor;
}
