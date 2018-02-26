#include "../events/baseaudioevent.h"
#include "../utilities/bufferutility.h"
#include "../instruments/baseinstrument.h"
#include "../audioengine.h"

// test of deprecated setSampleStart, setSampleLength... etc methods

TEST( BaseAudioEventDeprecation, PositionInSamples )
{
    BaseAudioEvent* audioEvent = new BaseAudioEvent();

    int eventLength = randomInt( 512, 8192 );
    int eventStart  = randomInt( 0, eventLength / 2 );
    int expectedEnd = eventStart + ( eventLength - 1 );

    audioEvent->setSampleStart ( eventStart );
    audioEvent->setSampleLength( eventLength );

    EXPECT_EQ( eventStart,  audioEvent->getSampleStart() );
    EXPECT_EQ( expectedEnd,  audioEvent->getSampleEnd() );
    EXPECT_EQ( eventLength, audioEvent->getSampleLength() );

    // test whether values in seconds have updated accordingly

    int SAMPLE_RATE = 44100;
    float expectedStartPosition = BufferUtility::bufferToSeconds( eventStart, SAMPLE_RATE );
    float expectedEndPosition   = BufferUtility::bufferToSeconds( expectedEnd, SAMPLE_RATE );
    float expectedDuration      = expectedEndPosition - expectedStartPosition;

    EXPECT_FLOAT_EQ( expectedStartPosition, audioEvent->getStartPosition() );
    EXPECT_FLOAT_EQ( expectedEndPosition,   audioEvent->getEndPosition() );
    EXPECT_FLOAT_EQ( expectedDuration,      audioEvent->getDuration() );

    // test auto sanitation of properties

    audioEvent->setEventEnd( expectedEnd * 2 );
    EXPECT_EQ( expectedEnd, audioEvent->getEventEnd() )
        << "expected sample end not to exceed the range set by the sample start and length properties";

    eventLength /= 2;
    audioEvent->setSampleLength( eventLength );
    expectedEnd = eventStart + ( eventLength - 1 );

    EXPECT_EQ( expectedEnd, audioEvent->getSampleEnd() )
        << "expected sample end not to exceed the range set by the sample start and updated length properties";

    // test non sanitation of properties for loopeable events

    audioEvent->setLoopeable( true );

    expectedEnd *= 2;
    audioEvent->setSampleEnd( expectedEnd );

    EXPECT_EQ( expectedEnd, audioEvent->getSampleEnd() )
        << "expected sample end to exceed the range set by the sample start and length properties for loopeable event";

    eventLength /= 2;
    audioEvent->setSampleLength( eventLength );

    EXPECT_EQ( expectedEnd, audioEvent->getSampleEnd() )
        << "expected sample end to exceed the range set by the sample start and updated length properties for loopeable event";

    delete audioEvent;
}

TEST( BaseAudioEventDeprecation, PositionInSamplesMethodStub )
{
    BaseAudioEvent* audioEvent = new BaseAudioEvent();

    int eventLength = randomInt( 512, 8192 );
    int eventStart  = randomInt( 0, eventLength / 2 );
    int expectedEnd = eventStart + ( eventLength - 1 );

    audioEvent->setSampleStart ( eventStart );
    audioEvent->setSampleLength( eventLength );

    EXPECT_EQ( audioEvent->getSampleStart(), audioEvent->getEventStart() )
        << "expected equal response as these getSampleStart stubs getEventStart";

    EXPECT_EQ( audioEvent->getSampleLength(), audioEvent->getEventLength() )
        << "expected equal response as getSampleLength stubs getEventLength";

    EXPECT_EQ( audioEvent->getSampleEnd(), audioEvent->getEventEnd() )
        << "expected equal response as these getSampleEnd stubs getEventEnd";

    delete audioEvent;
}
