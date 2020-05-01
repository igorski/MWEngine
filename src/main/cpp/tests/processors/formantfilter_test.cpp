#include <processors/formantfilter.h>

TEST( FormantFilter, getType )
{
    FormantFilter* processor = new FormantFilter( 1.0 );

    std::string expectedType( "FormantFilter" );
    ASSERT_TRUE( 0 == expectedType.compare( processor->getType() ));

    delete processor;
}
