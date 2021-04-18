#include <processors/bitcrusher.h>

TEST( BitCrusher, getType )
{
    BitCrusher* processor = new BitCrusher( 1.F, 1.F, 1.F );

    std::string expectedType( "BitCrusher" );
    ASSERT_TRUE( 0 == expectedType.compare( processor->getType() ));

    delete processor;
}
