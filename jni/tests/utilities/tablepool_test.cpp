#include "../../utilities/tablepool.h"
#include "../../wavetable.h"

TEST( TablePool, Pooling )
{
    WaveTable* table = new WaveTable( randomInt( 2, 8 ), randomFloat() );
    int waveformType = randomInt( 0, 12 );

    ASSERT_FALSE( TablePool::hasTable( waveformType ))
        << "expected TablePool not to contain a table";

    TablePool::setTable( table, waveformType );

    ASSERT_TRUE( TablePool::hasTable( waveformType ))
        << "expected TablePool to contain the table after setting";

    EXPECT_EQ( table, TablePool::getTable( waveformType ))
        << "expected TablePool to return pointer to the set table";

    TablePool::removeTable( waveformType );

    ASSERT_FALSE( TablePool::hasTable( waveformType ))
        << "expected TablePool not to contain a removed table";

    // deletion of table has been performed by removal from TablePool
}
