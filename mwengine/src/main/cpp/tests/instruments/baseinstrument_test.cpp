#include "../../instruments/baseinstrument.h"
#include "../../events/baseaudioevent.h"
#include "../../utilities/eventutility.h"
#include "../../sequencer.h"
#include "../../audioengine.h"

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
    AudioEngine::samples_per_bar = 512;
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

    int eventStart  = 1000;
    int eventLength = 500;
    int eventEnd    = eventStart + eventLength;

    event->setEventStart ( eventStart );
    event->setEventEnd   ( eventEnd );
    event->setEventLength( eventLength );
    event->addToSequencer();

    // increase tempo by given factor

    float factor = 2.0f;

    AudioEngine::tempo *= factor;

    // invoke updateEvents() (would have been executed by the Sequencer when running)

    instrument->updateEvents();

    int expectedEventStart = ( int ) ( eventStart / factor );
    int expectedEventEnd   = expectedEventStart + ( eventLength - 1 );

    EXPECT_EQ( expectedEventStart, event->getEventStart() )
        << "expected event start offset to have updated after tempo change and invocation of updateEvents()";

    EXPECT_EQ( expectedEventEnd, event->getEventEnd() )
        << "expected event end offset to have updated after tempo change and invocation of updateEvents()";

    EXPECT_EQ( eventLength, event->getEventLength() )
        << "expected event length not to have updated after tempo change and invocation of updateEvents()";

    // decrease tempo again by given factor

    factor = 0.5f;  // restores to original
    AudioEngine::tempo *= factor;

    instrument->updateEvents();

    EXPECT_EQ( eventStart, event->getEventStart() )
        << "expected event start offset to have updated after tempo change and invocation of updateEvents()";

    EXPECT_EQ(( eventEnd - 1 ), event->getEventEnd() )
        << "expected event end offset to have updated after tempo change and invocation of updateEvents()";

    EXPECT_EQ( eventLength, event->getEventLength() )
        << "expected event length not to have updated after tempo change and invocation of updateEvents()";

    delete event;
    delete instrument;
}

TEST( BaseInstrument, MeasureCache )
{
    BaseInstrument* instrument  = new BaseInstrument();
    BaseAudioEvent* audioEvent1 = new BaseAudioEvent( instrument );
    BaseAudioEvent* audioEvent2 = new BaseAudioEvent( instrument );
    BaseAudioEvent* audioEvent3 = new BaseAudioEvent( instrument );

    ASSERT_TRUE( nullptr == instrument->getEventsForMeasure( 0 )) << "expected no events yet";

    AudioEngine::samples_per_bar = 512;

    // expect event to span from 0 - 511 which spans measure 0
    audioEvent1->setEventStart(0);
    audioEvent1->setEventLength(512);

    // expect event to span from 0 - 1023 which spans measures 0 and 1
    audioEvent2->setEventStart(0);
    audioEvent2->setEventLength(1024);

    // expect event to span from 512 - 1535 which spans measure 1, 2 and 3
    audioEvent3->setEventStart(512);
    audioEvent3->setEventLength(1536);

    // add events to sequencer
    instrument->addEvent( audioEvent1, false );
    instrument->addEvent( audioEvent2, false );
    instrument->addEvent( audioEvent3, false );
    
    // assert

    auto measure0events = instrument->getEventsForMeasure( 0 );
    
    ASSERT_FALSE( nullptr == measure0events ) << "expected an event vector to have been instantiated";
    EXPECT_EQ( measure0events->size(), 2 ) << "expected two events in vector";
    ASSERT_TRUE( EventUtility::vectorContainsEvent( measure0events, audioEvent1 ));
    ASSERT_TRUE( EventUtility::vectorContainsEvent( measure0events, audioEvent2 ));
    ASSERT_FALSE( EventUtility::vectorContainsEvent( measure0events, audioEvent3 ));
    
    auto measure1events = instrument->getEventsForMeasure( 1 );
    
    ASSERT_FALSE( nullptr == measure1events ) << "expected an event vector to have been instantiated";
    EXPECT_EQ( measure1events->size(), 2 ) << "expected two events in vector";
    ASSERT_FALSE( EventUtility::vectorContainsEvent( measure1events, audioEvent1 ));
    ASSERT_TRUE( EventUtility::vectorContainsEvent( measure1events, audioEvent2 ));
    ASSERT_TRUE( EventUtility::vectorContainsEvent( measure1events, audioEvent3 ));

    auto measure2events = instrument->getEventsForMeasure( 2 );
    
    ASSERT_FALSE( nullptr == measure2events ) << "expected an event vector to have been instantiated";
    EXPECT_EQ( measure2events->size(), 1 ) << "expected one event in vector";
    ASSERT_FALSE( EventUtility::vectorContainsEvent( measure2events, audioEvent1 ));
    ASSERT_FALSE( EventUtility::vectorContainsEvent( measure2events, audioEvent2 ));
    ASSERT_TRUE( EventUtility::vectorContainsEvent( measure2events, audioEvent3 ));

    auto measure3events = instrument->getEventsForMeasure( 3 );
    
    ASSERT_FALSE( nullptr == measure3events ) << "expected an event vector to have been instantiated";
    EXPECT_EQ( measure3events->size(), 1 ) << "expected one event in vector";
    ASSERT_FALSE( EventUtility::vectorContainsEvent( measure3events, audioEvent1 ));
    ASSERT_FALSE( EventUtility::vectorContainsEvent( measure3events, audioEvent2 ));
    ASSERT_TRUE( EventUtility::vectorContainsEvent( measure3events, audioEvent3 ));

    instrument->removeEvent( audioEvent3, false );

    measure3events = instrument->getEventsForMeasure( 3 );
    EXPECT_EQ( measure3events->size(), 0 ) << "expected no events left in vector";
    EXPECT_EQ( measure2events->size(), 0 ) << "expected no events left in vector";
    EXPECT_EQ( measure1events->size(), 1 ) << "expected one event left in vector";
    EXPECT_EQ( measure0events->size(), 2 ) << "expected two events left in vector";

    ASSERT_TRUE( nullptr == instrument->getEventsForMeasure( 4 ))
        << "expected to retrieve a null pointer when requesting an out-of-range measure";

    delete audioEvent1;
    delete audioEvent2;
    delete audioEvent3;
    delete instrument;
}