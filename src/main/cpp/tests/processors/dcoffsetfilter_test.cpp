#include <processors/dcoffsetfilter.h>

TEST( DCOffsetFilter, getType )
{
    DCOffsetFilter* processor = new DCOffsetFilter( 1 );

    std::string expectedType( "DCOffsetFilter" );
    ASSERT_TRUE( 0 == expectedType.compare( processor->getType() ));

    delete processor;
}
