#include <processors/limiter.h>

TEST( Limiter, getType )
{
    Limiter* processor = new Limiter();

    std::string expectedType( "Limiter" );
    ASSERT_TRUE( 0 == expectedType.compare( processor->getType() ));

    delete processor;
}
