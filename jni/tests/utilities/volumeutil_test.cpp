#include "../../utilities/volumeutil.h"

TEST( VolumeUtil, ToLogarithmic )
{
    // hard wired for a curve of 2.f, see volumeutil.h
    EXPECT_FLOAT_EQ( 0.f,   VolumeUtil::toLog( 0.f ));
    EXPECT_FLOAT_EQ( 0.25f, VolumeUtil::toLog( 0.5f ));
    EXPECT_FLOAT_EQ( 1.f,   VolumeUtil::toLog( 1.f ));
}

TEST( VolumeUtil, ToLinear )
{
    // hard wired for a curve of 2.f, see volumeutil.h
    EXPECT_FLOAT_EQ( 0.f,  VolumeUtil::toLinear( 0.f ));
    EXPECT_FLOAT_EQ( 0.5f, VolumeUtil::toLinear( 0.25f ));
    EXPECT_FLOAT_EQ( 1.f,  VolumeUtil::toLinear( 1.f ));
}
