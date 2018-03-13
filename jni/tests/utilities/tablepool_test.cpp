#include "../../utilities/tablepool.h"
#include "../../wavetable.h"

TEST( TablePool, SetTable )
{
    WaveTable* table = new WaveTable( randomInt( 2, 8 ), randomFloat() );
    std::string id = "foo";

    ASSERT_FALSE( TablePool::hasTable( id ))
        << "expected TablePool not to contain the table yet";

    ASSERT_TRUE( TablePool::setTable( table, id ));

    ASSERT_TRUE( TablePool::hasTable( id ))
        << "expected TablePool to contain the table after registering it into the pool";

    TablePool::removeTable( id, true );

    // deletion of table has been performed by removal from TablePool
}

TEST( TablePool, SetTableDeduplicate )
{
    WaveTable* table = new WaveTable( randomInt( 2, 8 ), randomFloat() );
    std::string id = "foo";

    ASSERT_TRUE( TablePool::setTable( table, id ));
    ASSERT_FALSE( TablePool::setTable( table, id )) << "expected table for some identifier not to be accepted twice";

    TablePool::removeTable( id, true );

    // deletion of table has been performed by removal from TablePool
}

TEST( TablePool, RemoveTable )
{
    WaveTable* table = new WaveTable( randomInt( 2, 8 ), randomFloat() );
    std::string id = "foo";

    TablePool::setTable( table, id );

    TablePool::removeTable( id, false );

    ASSERT_FALSE( table == 0 ) << "expected Table not to have been deleted";

    delete table;
}
