#include <processors/fm.h>

TEST( FrequencyModulator, getType )
{
    FrequencyModulator* processor = new FrequencyModulator( 1, 10.F );

    std::string expectedType( "FrequencyModulator" );
    ASSERT_TRUE( 0 == expectedType.compare( processor->getType() ));

    delete processor;
}
