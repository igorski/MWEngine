#include "../../utilities/eventutility.h"
#include "../../events/baseaudioevent.h"

TEST( EventUtility, EventStartAndEndMeasures )
{
    AudioEngine::samples_per_bar = 512;
    BaseAudioEvent* audioEvent   = new BaseAudioEvent();

    // expected to occupy measure 0
    audioEvent->setEventStart(0);
    audioEvent->setEventLength(512);

    EXPECT_EQ(0, EventUtility::getStartMeasureForEvent( audioEvent ));
    EXPECT_EQ(0, EventUtility::getEndMeasureForEvent( audioEvent ));

    // expected to occupy measures 0 and 1
    audioEvent->setEventLength(1024);

    EXPECT_EQ(0, EventUtility::getStartMeasureForEvent( audioEvent ));
    EXPECT_EQ(1, EventUtility::getEndMeasureForEvent( audioEvent ));

    // expected to occupy measures 1 and 2

    audioEvent->setEventStart(512);

    EXPECT_EQ(1, EventUtility::getStartMeasureForEvent( audioEvent ));
    EXPECT_EQ(2, EventUtility::getEndMeasureForEvent( audioEvent ));

    delete audioEvent;
}

TEST( EventUtility, VectorContains )
{
    BaseAudioEvent* audioEvent = new BaseAudioEvent();
    std::vector<BaseAudioEvent*>* vector = new std::vector<BaseAudioEvent*>();

    ASSERT_FALSE( EventUtility::vectorContainsEvent( vector, audioEvent ));

    vector->push_back( audioEvent );

    ASSERT_TRUE( EventUtility::vectorContainsEvent( vector, audioEvent ));

    delete audioEvent;
    delete vector;
}
