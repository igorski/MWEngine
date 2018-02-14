#include "../../instruments/baseinstrument.h"
#include "../../events/baseaudioevent.h"
#include "../../sequencer.h"

TEST( BaseInstrument, Constructor )
{
    BaseInstrument* instrument = new BaseInstrument();

    ASSERT_FALSE( 0 == instrument->audioChannel )
        << "expected instrument to have created an AudioChannel during construction";

    ASSERT_FALSE( 0 == instrument->getEvents() )
        << "expected instrument to have created an events vector during construction";

    ASSERT_FALSE( 0 == instrument->getLiveEvents() )
        << "expected instrument to have created an live events vector during construction";

    EXPECT_EQ( Sequencer::instruments.size() - 1, instrument->index )
        << "expected instrument to have the highest index after addition to the sequencer";

    delete instrument;
}

TEST( BaseInstrument, SequencerRegistration )
{
    BaseInstrument* instrument = new BaseInstrument();

    EXPECT_EQ( Sequencer::instruments.size() - 1, instrument->index )
        << "expected instrument to have the highest index after addition to the sequencer after construction";

    bool wasPresent = false;
    for ( int i = 0; i < Sequencer::instruments.size(); i++ )
    {
        if ( Sequencer::instruments.at( i ) == instrument )
            wasPresent = true;
    }

    ASSERT_TRUE( wasPresent )
        << "expected instrument to be present in Sequencers instrument list after construction";

    instrument->unregisterFromSequencer();

    EXPECT_EQ( -1, instrument->index )
        << "expected instrument to have a negative index indicating its not added to the sequencer";

    wasPresent = false;
    for ( int i = 0; i < Sequencer::instruments.size(); i++ )
    {
        if ( Sequencer::instruments.at( i ) == instrument )
            wasPresent = true;
    }

    ASSERT_FALSE( wasPresent )
        << "expected instrument not to be present in Sequencers instrument list after unregistration";

    instrument->registerInSequencer();

    EXPECT_EQ( Sequencer::instruments.size() - 1, instrument->index )
        << "expected instrument to have the highest index after addition to the sequencer";

    wasPresent = false;
    for ( int i = 0; i < Sequencer::instruments.size(); i++ )
    {
        if ( Sequencer::instruments.at( i ) == instrument )
            wasPresent = true;
    }

    ASSERT_TRUE( wasPresent )
        << "expected instrument to be present in Sequencers instrument list after registration";

    delete instrument;
}

TEST( BaseInstrument, Events )
{
    BaseInstrument* instrument = new BaseInstrument();

    ASSERT_FALSE( instrument->hasEvents() )
        << "expected instrument to contain no events after construction";

    ASSERT_FALSE( instrument->hasLiveEvents() )
        << "expected instrument to contain no live events after construction";

    // create audio events

    BaseAudioEvent* audioEvent = new BaseAudioEvent( instrument );
    BaseAudioEvent* liveEvent  = new BaseAudioEvent( instrument );
    liveEvent->isSequenced     = false; // making it a live event

    // add an event

    audioEvent->addToSequencer();

    ASSERT_TRUE( instrument->hasEvents() )
        << "expected instrument to contain events after addition";

    ASSERT_FALSE( instrument->hasLiveEvents() )
        << "expected instrument to contain no live events after addition of sequenced event";

    bool wasPresent = false;
    for ( int i = 0; i < instrument->getEvents()->size(); i++ )
    {
        if ( instrument->getEvents()->at( i ) == audioEvent )
            wasPresent = true;
    }

    ASSERT_TRUE( wasPresent )
        << "expected added event to be present in instruments event list after addition";

    // remove event

    audioEvent->removeFromSequencer();

    ASSERT_FALSE( instrument->hasEvents() )
        << "expected instrument to contain no events after removal";

    wasPresent = false;
    for ( int i = 0; i < instrument->getEvents()->size(); i++ )
    {
        if ( instrument->getEvents()->at( i ) == audioEvent )
            wasPresent = true;
    }

    ASSERT_FALSE( wasPresent )
        << "expected event not to be present in events list after removal";

    // add a live event

    liveEvent->addToSequencer();

    ASSERT_FALSE( instrument->hasEvents() )
        << "expected instrument to contain no sequenced events after addition of live event";

    ASSERT_TRUE( instrument->hasLiveEvents() )
        << "expected instrument to contain live events after addition";

    wasPresent = false;
    for ( int i = 0; i < instrument->getLiveEvents()->size(); i++ )
    {
        if ( instrument->getLiveEvents()->at( i ) == liveEvent )
            wasPresent = true;
    }

    ASSERT_TRUE( wasPresent )
        << "expected added event to be present in instruments live event list after addition";

    // remove live event

    liveEvent->removeFromSequencer();

    ASSERT_FALSE( instrument->hasLiveEvents() )
        << "expected instrument to contain no live events after removal";

    wasPresent = false;
    for ( int i = 0; i < instrument->getLiveEvents()->size(); i++ )
    {
        if ( instrument->getLiveEvents()->at( i ) == liveEvent )
            wasPresent = true;
    }

    ASSERT_FALSE( wasPresent )
        << "expected event not to be present in live events list after removal";

    // remove event method test

    audioEvent->addToSequencer();
    instrument->removeEvent( audioEvent, false );

    wasPresent = false;
    for ( int i = 0; i < instrument->getEvents()->size(); i++ )
    {
        if ( instrument->getEvents()->at( i ) == audioEvent )
            wasPresent = true;
    }

    ASSERT_FALSE( wasPresent )
        << "expected event not to be present in event list after removal";

    // remove event method test with live event

    liveEvent->addToSequencer();
    instrument->removeEvent( liveEvent, true );

    wasPresent = false;
    for ( int i = 0; i < instrument->getLiveEvents()->size(); i++ )
    {
        if ( instrument->getLiveEvents()->at( i ) == liveEvent )
            wasPresent = true;
    }

    ASSERT_FALSE( wasPresent )
        << "expected event not to be present in live event list after removal";

    // clear events method test

    audioEvent->addToSequencer();
    liveEvent->addToSequencer();

    ASSERT_TRUE( instrument->hasEvents() );
    ASSERT_TRUE( instrument->hasLiveEvents() );

    instrument->clearEvents(); // clear events vectors

    ASSERT_FALSE( instrument->hasEvents() )
        << "expected instrument to contain no events after clearing";

    ASSERT_FALSE( instrument->hasLiveEvents() )
        << "expected instrument to contain no live events after clearing";

    delete audioEvent;
    delete liveEvent;
    delete instrument;
}

TEST( BaseInstrument, UpdateEvents )
{
    // test for tempo change updates

    AudioEngine::tempo = 120.0f;

    BaseInstrument* instrument = new BaseInstrument();
    BaseAudioEvent* event      = new BaseAudioEvent( instrument );

    int sampleStart  = 1000;
    int sampleLength = 500;
    int sampleEnd    = sampleStart + sampleLength;

    event->setEventStart ( sampleStart );
    event->setEventEnd   ( sampleEnd );
    event->setEventLength( sampleLength );
    event->addToSequencer();

    // increase tempo by given factor

    float factor = 2.0f;

    AudioEngine::tempo *= factor;

    // invoke updateEvents() (would have been executed by the Sequencer when running)

    instrument->updateEvents();

    EXPECT_EQ(( int )( sampleStart / factor ), event->getEventStart() )
        << "expected event start offset to have updated after tempo change and invocation of updateEvents()";

    EXPECT_EQ(( int )(( sampleEnd - 1 ) / factor ), event->getEventEnd() )
        << "expected event end offset to have updated after tempo change and invocation of updateEvents()";

    EXPECT_EQ(( int )( sampleLength / factor ), event->getEventLength() )
        << "expected event length to have updated after tempo change and invocation of updateEvents()";

    // decrease tempo again by given factor

    factor = 0.5f;  // restores to original
    AudioEngine::tempo *= factor;

    instrument->updateEvents();

    EXPECT_EQ( sampleStart, event->getEventStart() )
        << "expected event start offset to have updated after tempo change and invocation of updateEvents()";

    EXPECT_EQ(( sampleEnd - 1 ), event->getEventEnd() )
        << "expected event end offset to have updated after tempo change and invocation of updateEvents()";

    EXPECT_EQ( sampleLength, event->getEventLength() )
        << "expected event length to have updated after tempo change and invocation of updateEvents()";

    delete event;
    delete instrument;
}
