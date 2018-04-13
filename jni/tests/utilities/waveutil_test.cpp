#include "../../utilities/waveutil.h"

TEST( WaveUtil, ToUnipolar )
{
    SAMPLE_TYPE buffer[ 5 ] = { -1.0, -0.5, 0, 0.5, 1.0 };

    WaveUtil::toUnipolar( buffer, 5 );

    EXPECT_FLOAT_EQ( buffer[ 0 ], 0 );
    EXPECT_FLOAT_EQ( buffer[ 1 ], 0.25 );
    EXPECT_FLOAT_EQ( buffer[ 2 ], 0.5 );
    EXPECT_FLOAT_EQ( buffer[ 3 ], 0.75 );
    EXPECT_FLOAT_EQ( buffer[ 4 ], 1 );
}

TEST( WaveUtil, ToBipolar )
{
    SAMPLE_TYPE buffer[ 5 ] = { 0, 0.25, 0.5, 0.75, 1 };

    WaveUtil::toBipolar( buffer, 5 );

    EXPECT_FLOAT_EQ( buffer[ 0 ], -1.0 );
    EXPECT_FLOAT_EQ( buffer[ 1 ], -0.5 );
    EXPECT_FLOAT_EQ( buffer[ 2 ], 0.0 );
    EXPECT_FLOAT_EQ( buffer[ 3 ], 0.5 );
    EXPECT_FLOAT_EQ( buffer[ 4 ], 1.0 );
}

TEST( WaveUtil, IsUnipolar )
{
    SAMPLE_TYPE buffer1[ 5 ] = { 0, 0.25, 0.5, 0.75, 1 };
    ASSERT_TRUE( WaveUtil::isUnipolar( buffer1, 5 )) << "buffer holds no bipolar contents";

    SAMPLE_TYPE buffer2[ 5 ] = { -1.0, -0.5, 0, 0.5, 1.0 };
    ASSERT_FALSE( WaveUtil::isUnipolar( buffer2, 5 )) << "buffer holds bipolar contents";
}

TEST( WaveUtil, IsBipolar )
{
    SAMPLE_TYPE buffer1[ 5 ] = { 0, 0.25, 0.5, 0.75, 1 };
    ASSERT_FALSE( WaveUtil::isBipolar( buffer1, 5 )) << "buffer holds no bipolar contents";

    SAMPLE_TYPE buffer2[ 5 ] = { -1.0, -0.5, 0, 0.5, 1.0 };
    ASSERT_TRUE( WaveUtil::isBipolar( buffer2, 5 )) << "buffer holds bipolar contents";
}
