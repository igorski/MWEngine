#include "../../utilities/eventutility.h"
#include "../../events/baseaudioevent.h"

TEST( EventUtility, EventStartAndEndMeasures )
{
    AudioEngine::samples_per_bar = 512;
    BaseAudioEvent* audioEvent   = new BaseAudioEvent();

    // expected to occupy measure 0 (0 - 511)
    audioEvent->setEventStart(0);
    audioEvent->setEventLength(512);

    EXPECT_EQ(0, EventUtility::getStartMeasureForEvent( audioEvent ));
    EXPECT_EQ(0, EventUtility::getEndMeasureForEvent( audioEvent ));

    // expected to occupy measures 0 and 1 (0 - 1023)
    audioEvent->setEventLength(1024);

    EXPECT_EQ(0, EventUtility::getStartMeasureForEvent( audioEvent ));
    EXPECT_EQ(1, EventUtility::getEndMeasureForEvent( audioEvent ));

    // expected to occupy measures 1 and 2 (512 - 1535)

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

TEST( EventUtility, RemoveEventFromVector )
{
    BaseAudioEvent* audioEvent = new BaseAudioEvent();
    std::vector<BaseAudioEvent*>* vector = new std::vector<BaseAudioEvent*>();

    ASSERT_FALSE( EventUtility::removeEventFromVector( vector, audioEvent )) << "expected removal of non-added event to return false";

    vector->push_back( audioEvent );

    ASSERT_TRUE( EventUtility::removeEventFromVector( vector, audioEvent )) << "expected removal of added event to return true";
    ASSERT_FALSE( EventUtility::vectorContainsEvent( vector, audioEvent )) << "expected event to have been removed from vector";

    delete audioEvent;
    delete vector;
}
