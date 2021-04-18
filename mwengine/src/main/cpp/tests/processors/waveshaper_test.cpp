#include <processors/waveshaper.h>

TEST( WaveShaper, getType )
{
    WaveShaper* processor = new WaveShaper( 0.F, 1.F );

    std::string expectedType( "WaveShaper" );
    ASSERT_TRUE( 0 == expectedType.compare( processor->getType() ));

    delete processor;
}
