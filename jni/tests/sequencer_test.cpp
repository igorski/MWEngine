#include "../audiochannel.h"
#include "../audioengine.h"
#include "../sequencer.h"
#include "../instruments/baseinstrument.h"
#include "../events/basecacheableaudioevent.h"
#include "../events/synthevent.h"

TEST( Sequencer, InstrumentRegistration )
{
    EXPECT_EQ( 0, Sequencer::instruments.size() )
        << "expected sequencer to contain no instruments at test start";

    // constructing an instrument will automatically invoke registerInstrument() on the Sequencer
    BaseInstrument* instrument1 = new BaseInstrument();

    EXPECT_EQ( 1, Sequencer::instruments.size() )
        << "expected sequencer to contain one instrument after construction of an instrument";

    BaseInstrument* instrument2 = new BaseInstrument();

    EXPECT_EQ( 2, Sequencer::instruments.size() )
            << "expected sequencer to contain two instruments after construction of another instrument";

    Sequencer::unregisterInstrument( instrument1 );

    EXPECT_EQ( 1, Sequencer::instruments.size() )
        << "expected sequencer to contain one instrument after removal";

    // destructing an instrument will automatically invoke unregisterInstrument() on the Sequencer
    delete instrument2;

    EXPECT_EQ( 0, Sequencer::instruments.size() )
        << "expected sequencer to contain no instruments after destruction of the final instrument";

    delete instrument1;
}

TEST( Sequencer, GetAudioEvents )
{
    // setup sequencer with two instruments

    std::vector<AudioChannel*>* channels = new std::vector<AudioChannel*>();

    BaseInstrument* instrument1 = new BaseInstrument();
    BaseInstrument* instrument2 = new BaseInstrument();
    BaseInstrument* instrument3 = new BaseInstrument();

    // setup audio events

    // assume 88200 samples per bar (emulates 44.1 kHz sample rate at 120 BPM 4/4 time)
    // use a buffer size that is the size of an 8th note (11025 samples)
    AudioEngine::samples_per_bar = 88200;
    Sequencer::playing           = true;
    int bufferSize               = AudioEngine::samples_per_bar / 8;

    BaseAudioEvent* audioEvent1 = enqueuedAudioEvent( instrument1, bufferSize * 1.5, 0, 16, 0 );
    BaseAudioEvent* audioEvent2 = enqueuedAudioEvent( instrument2, bufferSize * 2,   0, 16, 1 );
    BaseAudioEvent* audioEvent3 = enqueuedAudioEvent( instrument3, bufferSize,       0, 16, 0 );

    // with a size of samples per bar of 88200 (defined above)
    // AudioEvent1 start: 0 end: 16536 (length: 16537)
    // AudioEvent2 start: 5512 end: 27561 (length: 22050)
    // AudioEvent3 start: 0 end: 11024 (length: 11025)

    // test 1 : expect 3 events (all within range of first 8th note)

    int startOffset = 0;

    Sequencer::getAudioEvents( channels, startOffset, bufferSize, true, true );

    EXPECT_EQ( 3, channels->size() )
        << "expected to receive all AudioChannels";

    EXPECT_EQ( 1, channels->at( 0 )->audioEvents.size() )
        << "expected AudioChannel 1 to contain 1 audio event";

    EXPECT_EQ( 1, channels->at( 1 )->audioEvents.size() )
        << "expected AudioChannel 2 to contain 1 audio event";

    EXPECT_EQ( 1, channels->at( 2 )->audioEvents.size() )
        << "expected AudioChannel 3 to contain 1 audio event";

    ASSERT_TRUE( channels->at( 0 )->audioEvents.at( 0 ) == audioEvent1 )
        << "expected to have collected event for AudioChannel 1";

    ASSERT_TRUE( channels->at( 1 )->audioEvents.at( 0 ) == audioEvent2 )
        << "expected to have collected event for AudioChannel 2";

    ASSERT_TRUE( channels->at( 2 )->audioEvents.at( 0 ) == audioEvent3 )
        << "expected to have collected event for AudioChannel 3";

    // test 2 : expect 2 events (1st event sampleEnd is within current range,
    // 2nd event sampleStart is at this requested offset, 3rd event has finished playback
    // at the end of the previous requested buffer range)

    startOffset = bufferSize;
    Sequencer::getAudioEvents( channels, startOffset, bufferSize, true, true );

    EXPECT_EQ( 3, channels->size() )
        << "expected to receive all AudioChannels";

    EXPECT_EQ( 1, channels->at( 0 )->audioEvents.size() )
        << "expected AudioChannel 1 to contain 1 audio event";

    EXPECT_EQ( 1, channels->at( 1 )->audioEvents.size() )
        << "expected AudioChannel 2 to contain 1 audio event";

    EXPECT_EQ( 0, channels->at( 2 )->audioEvents.size() )
        << "expected AudioChannel 3 to contain no events";

    ASSERT_TRUE( channels->at( 0 )->audioEvents.at( 0 ) == audioEvent1 )
        << "expected to have collected event for AudioChannel 1";

    ASSERT_TRUE( channels->at( 1 )->audioEvents.at( 0 ) == audioEvent2 )
        << "expected to have collected event for AudioChannel 2";

    // test 3 : expect 1 event (1st event sampleEnd is outside of current range,
    // 2nd event sampleEnd is still within range, 3rd event is outside of range)

    startOffset = bufferSize * 2;
    Sequencer::getAudioEvents( channels, startOffset, bufferSize, true, true );

    EXPECT_EQ( 3, channels->size() )
        << "expected to receive all AudioChannels";

    EXPECT_EQ( 0, channels->at( 0 )->audioEvents.size() )
        << "expected to have received 0 events for AudioChannel 1";

    EXPECT_EQ( 1, channels->at( 1 )->audioEvents.size() )
        << "expected to have received 1 events for AudioChannel 2";

    EXPECT_EQ( 0, channels->at( 2 )->audioEvents.size() )
        << "expected to have received no events for AudioChannel 3";

    ASSERT_TRUE( channels->at( 1 )->audioEvents.at( 0 ) == audioEvent2 )
        << "expected to have collected event for AudioChannel 2";

    // test 4 : expect no event (1st event sampleEnd is outside of current range,
    // 2nd event sampleEnd is still within range)

    startOffset = bufferSize * 3;
    Sequencer::getAudioEvents( channels, startOffset, bufferSize, true, true );

    EXPECT_EQ( 3, channels->size() )
        << "expected to receive all AudioChannels";

    EXPECT_EQ( 0, channels->at( 0 )->audioEvents.size() )
        << "expected to have no collected events for AudioChannel 1";

    EXPECT_EQ( 0, channels->at( 1 )->audioEvents.size() )
        << "expected to have no collected events for AudioChannel 2";

    EXPECT_EQ( 0, channels->at( 2 )->audioEvents.size() )
        << "expected to have no collected events for AudioChannel 3";

    // free allocated memory

    delete channels;
    delete audioEvent1;
    delete audioEvent2;
    delete audioEvent3;
    delete instrument1;
    delete instrument2;
    delete instrument3;

    // reset Sequencer

    Sequencer::playing = false;
}

TEST( Sequencer, GetAudioEventsAtBoundaries )
{
    // setup sequencer

    std::vector<AudioChannel*>* channels = new std::vector<AudioChannel*>();
    BaseInstrument* instrument = new BaseInstrument();

    // setup audio events

    // assume 88200 samples per bar (emulates 44.1 kHz sample rate at 120 BPM 4/4 time)
    // use a buffer size that is the size of an 8th note (11025 samples)
    AudioEngine::samples_per_bar = 88200;
    Sequencer::playing           = true;
    int bufferSize               = AudioEngine::samples_per_bar / 8;

    // AudioEvent start: 11025 end: 22049 (length: 11025)
    BaseAudioEvent* audioEvent = enqueuedAudioEvent( instrument, bufferSize, 0, 16, 2 );

    // test 1 : collect audio event that will only sound for 1 sample in length at requested range
    // e.g. requested range start = 22049 while sample end is 22049

    int startOffset = audioEvent->getSampleEnd();
    Sequencer::getAudioEvents( channels, startOffset, bufferSize, true, true );

    EXPECT_EQ( 1, channels->at( 0 )->audioEvents.size() )
        << "expected to have collected 1 audio event";

    // test 2 : ensure no events are collected when the requested range starts beyond the sample end offset
    // e.g. requested range start = 22050 whereas sample end is 22049

    startOffset = audioEvent->getSampleEnd() + 1;
    Sequencer::getAudioEvents( channels, startOffset, bufferSize, true, true );

    EXPECT_EQ( 0, channels->at( 0 )->audioEvents.size() )
        << "expected to have collected no events";

    // test 3 : ensure no events are collected when the requested range end is 1 sample before the sample start offset
    // e.g. requested range end = 11024 whereas sample start is 11025

    startOffset = 0;
    Sequencer::getAudioEvents( channels, startOffset, bufferSize, true, true );

    EXPECT_EQ( 0, channels->at( 0 )->audioEvents.size() )
        << "expected to have collected no events";

    // test 4 : collect audio event when the requested range end is 1 sample before the sample end offset
    // e.g. requested range end = 11025 while sample start is 11025

    startOffset = 1;
    Sequencer::getAudioEvents( channels, startOffset, bufferSize, true, true );

    EXPECT_EQ( 1, channels->at( 0 )->audioEvents.size() )
        << "expected to have collected 1 audio event";

    // free allocated memory
    delete channels;
    delete audioEvent;
    delete instrument;

    // reset Sequencer
    Sequencer::playing = false;
}

TEST( Sequencer, GetLiveEvents )
{
    // setup sequencer

    std::vector<AudioChannel*>* channels = new std::vector<AudioChannel*>();
    BaseInstrument* instrument1 = new BaseInstrument();
    BaseInstrument* instrument2 = new BaseInstrument();

    // setup audio events

    // assume 88200 samples per bar (emulates 44.1 kHz sample rate at 120 BPM 4/4 time)
    // use a buffer size that is the size of an 8th note (11025 samples)
    AudioEngine::samples_per_bar = 88200;
    Sequencer::playing           = false;
    int bufferSize               = AudioEngine::samples_per_bar / 8;

    BaseAudioEvent* audioEvent1 = enqueuedAudioEvent( instrument1, bufferSize, 0, 16, 0 );
    BaseAudioEvent* audioEvent2 = new BaseAudioEvent( instrument2 );
    audioEvent2->isSequenced = false; // making it a "live" event
    audioEvent2->addToSequencer();

    ASSERT_TRUE( instrument1->hasEvents() )
        << "expected instrument 1 to contain sequenced events";

    ASSERT_FALSE( instrument1->hasLiveEvents() )
        << "expected instrument 1 to contain no live events";

    ASSERT_FALSE( instrument2->hasEvents() )
        << "expected instrument 2 to contain no sequenced events";

    ASSERT_TRUE( instrument2->hasLiveEvents() )
        << "expected instrument 2 to contain live events";

    // test 1 : collect only live events (should collect even when Sequencer isn't running)

    Sequencer::getAudioEvents( channels, 0, bufferSize, true, true );

    EXPECT_EQ( 0, channels->at( 0 )->audioEvents.size() )
        << "expected to have collected no sequenced Events for AudioChannel 1 (Sequencer wasn't playing)";

    EXPECT_EQ( 0, channels->at( 0 )->liveEvents.size() )
        << "expected to have collected no live Events for AudioChannel 1 (none were existent for its instrument)";

    EXPECT_EQ( 0, channels->at( 1 )->audioEvents.size() )
        << "expected to have collected no sequenced Events for AudioChannel 2 (none were existent for its instrument)";

    EXPECT_EQ( 1, channels->at( 1 )->liveEvents.size() )
        << "expected to have collected 1 live Event for AudioChannel 2";

    // test 2 : collect live and sequenced events (Sequencer is now running)

    Sequencer::playing = true;
    Sequencer::getAudioEvents( channels, 0, bufferSize, true, true );

    EXPECT_EQ( 1, channels->at( 0 )->audioEvents.size() )
        << "expected to have collected 1 sequenced Events for AudioChannel 1";

    EXPECT_EQ( 0, channels->at( 0 )->liveEvents.size() )
        << "expected to have collected no live Events for AudioChannel 1 (none were existent for its instrument)";

    EXPECT_EQ( 0, channels->at( 1 )->audioEvents.size() )
        << "expected to have collected no sequenced Events for AudioChannel 2 (nore were existent for its instrument)";

    EXPECT_EQ( 1, channels->at( 1 )->liveEvents.size() )
        << "expected to have collected 1 live Event for AudioChannel 2";

    // test 3 : collect only sequenced events (Sequencer is now running but no live events are requested)

    Sequencer::getAudioEvents( channels, 0, bufferSize, false, true );

    EXPECT_EQ( 1, channels->at( 0 )->audioEvents.size() )
        << "expected to have collected 1 sequenced Events for AudioChannel 1";

    EXPECT_EQ( 0, channels->at( 0 )->liveEvents.size() )
        << "expected to have collected no live Events for AudioChannel 1 (none were existent for its instrument and none were requested)";

    EXPECT_EQ( 0, channels->at( 1 )->audioEvents.size() )
        << "expected to have collected no sequenced Events for AudioChannel 2 (nore were existent for its instrument)";

    EXPECT_EQ( 0, channels->at( 1 )->audioEvents.size() )
        << "expected to have collected no live events for AudioChannel 2 (wasn't requested)";

    // free allocated memory
    delete channels;
    delete audioEvent1;
    delete audioEvent2;
    delete instrument1;
    delete instrument2;

    // reset Sequencer
    Sequencer::playing = false;
}

TEST( Sequencer, GetEventsFlushChannel )
{
    // setup sequencer

    std::vector<AudioChannel*>* channels = new std::vector<AudioChannel*>();
    BaseInstrument* instrument = new BaseInstrument();

    // setup audio events

    // assume 88200 samples per bar (emulates 44.1 kHz sample rate at 120 BPM 4/4 time)
    // use a buffer size that is the size of an 8th note (11025 samples)
    AudioEngine::samples_per_bar = 88200;
    Sequencer::playing           = true;
    int bufferSize               = AudioEngine::samples_per_bar / 8;

    BaseAudioEvent* audioEvent1 = enqueuedAudioEvent( instrument, bufferSize, 0, 16, 0 );
    BaseAudioEvent* audioEvent2 = enqueuedAudioEvent( instrument, bufferSize, 0, 16, 8 );

    // test 1 : retrieve events with flushing of channel contents

    Sequencer::playing = true;
    Sequencer::getAudioEvents( channels, audioEvent2->getSampleStart(), bufferSize, true, true );

    EXPECT_EQ( 1, channels->at( 0 )->audioEvents.size() )
        << "expected to have collected 1 event for AudioChannel 1";

    ASSERT_TRUE( channels->at( 0 )->audioEvents.at( 0 ) == audioEvent2 )
        << "expected to have retrieved the second AudioEvent";

    Sequencer::getAudioEvents( channels, 0, bufferSize, true, true );

    EXPECT_EQ( 1, channels->at( 0 )->audioEvents.size() )
        << "expected to have collected 1 event for AudioChannel 1";

    ASSERT_TRUE( channels->at( 0 )->audioEvents.at( 0 ) == audioEvent1 )
        << "expected to have retrieved the first AudioEvent";

    // test 2 : retrieve events without flushing of channel contents
    // first ensure channels have been flushed of previous test contents
    channels->at( 0 )->reset();

    Sequencer::getAudioEvents( channels, audioEvent2->getSampleStart(), bufferSize, true, false );

    EXPECT_EQ( 1, channels->at( 0 )->audioEvents.size() )
        << "expected to have collected 1 event for AudioChannel 1";

    ASSERT_TRUE( channels->at( 0 )->audioEvents.at( 0 ) == audioEvent2 )
        << "expected to have retrieved the second AudioEvent";

    Sequencer::getAudioEvents( channels, 0, bufferSize, true, false );

    EXPECT_EQ( 2, channels->at( 0 )->audioEvents.size() )
        << "expected to have collected 2 events for AudioChannel 1 (flushing was disabled)";

    ASSERT_TRUE( channels->at( 0 )->audioEvents.at( 0 ) == audioEvent2 )
        << "expected to have retrieved the second AudioEvent";

    ASSERT_TRUE( channels->at( 0 )->audioEvents.at( 1 ) == audioEvent1 )
        << "expected to have retrieved the first AudioEvent by merging new request into non flushed channel";

    // add audioEvent1 to the non flushed channel
    Sequencer::getAudioEvents( channels, 0, bufferSize, true, false );

    EXPECT_EQ( 3, channels->at( 0 )->audioEvents.size() )
        << "expected to have collected 3 events for AudioChannel 1 (flushing was disabled)";

    ASSERT_TRUE( channels->at( 0 )->audioEvents.at( 0 ) == audioEvent2 )
        << "expected to have retrieved the second AudioEvent in previous request";

    ASSERT_TRUE( channels->at( 0 )->audioEvents.at( 1 ) == audioEvent1 )
        << "expected to have retrieved the first AudioEvent in previous request";

    ASSERT_TRUE( channels->at( 0 )->audioEvents.at( 2 ) == audioEvent1 )
        << "expected to have retrieved the first AudioEvent by merging new request into non flushed channel";

    // free allocated memory

    delete channels;
    delete audioEvent1;
    delete audioEvent2;
    delete instrument;

    // reset Sequencer
    Sequencer::playing = false;
}

TEST( Sequencer, RemoveDeletableEvents )
{
    // setup sequencer

    std::vector<AudioChannel*>* channels = new std::vector<AudioChannel*>();
    BaseInstrument* instrument1 = new BaseInstrument();
    BaseInstrument* instrument2 = new BaseInstrument();

    // setup audio events

    // assume 88200 samples per bar (emulates 44.1 kHz sample rate at 120 BPM 4/4 time)
    // use a buffer size that is the size of an 8th note (11025 samples)
    AudioEngine::samples_per_bar = 88200;
    Sequencer::playing           = true;
    int bufferSize               = AudioEngine::samples_per_bar / 8;

    BaseAudioEvent* audioEvent1 = enqueuedAudioEvent( instrument1, bufferSize, 0, 16, 0 );
    BaseAudioEvent* audioEvent2 = enqueuedAudioEvent( instrument2, bufferSize, 0, 16, 0 );

    // test 1 : collect both events

    Sequencer::getAudioEvents( channels, 0, bufferSize, true, true );

    EXPECT_EQ( 1, channels->at( 0 )->audioEvents.size() )
        << "expected to have collected 1 event for AudioChannel 1";

    EXPECT_EQ( 1, channels->at( 1 )->audioEvents.size() )
        << "expected to have collected 1 event for AudioChannel 2";

    // test 2 : set event 2 as deletable

    audioEvent2->setDeletable( true );

    ASSERT_TRUE( instrument2->hasEvents() )
        << "expected instrument 2 to have events as event is marked as deletable, but hasn't been deleted yet";

    Sequencer::getAudioEvents( channels, 0, bufferSize, true, true );

    EXPECT_EQ( 1, channels->at( 0 )->audioEvents.size() )
        << "expected to have collected 1 event for AudioChannel 1";

    EXPECT_EQ( 0, channels->at( 1 )->audioEvents.size() )
        << "expected to have collected no events for AudioChannel 2";

    ASSERT_FALSE( instrument2->hasEvents() )
        << "expected instrument 2 to have no events as event has been deleted";

    // test 3 : set event 1 as deletable

    audioEvent1->setDeletable( true );

    ASSERT_TRUE( instrument1->hasEvents() )
        << "expected instrument 1 to have events as event is marked as deletable, but hasn't been deleted yet";

    Sequencer::getAudioEvents( channels, 0, bufferSize, true, true );

    EXPECT_EQ( 0, channels->at( 0 )->audioEvents.size() )
        << "expected to have collected no events for AudioChannel 1";

    EXPECT_EQ( 0, channels->at( 1 )->audioEvents.size() )
        << "expected to have collected no events for AudioChannel 2";

    ASSERT_FALSE( instrument1->hasEvents() )
        << "expected instrument 1 to have no events as event has been deleted";

    // free allocated memory
    delete channels;
    delete audioEvent1;
    delete audioEvent2;
    delete instrument1;
    delete instrument2;

    // reset Sequencer
    Sequencer::playing = false;
}

/*
TEST( Sequencer, GetCacheableEvents )
{
    // setup sequencer

    std::vector<AudioChannel*>* channels = new std::vector<AudioChannel*>();
    BaseInstrument* instrument1 = new BaseInstrument();
    BaseInstrument* instrument2 = new BaseInstrument();

    // setup audio events

    // assume 88200 samples per bar (emulates 44.1 kHz sample rate at 120 BPM 4/4 time)
    // use a buffer size that is the size of an 8th note (11025 samples)
    AudioEngine::samples_per_bar = 88200;
    Sequencer::playing           = true;
    int bufferSize               = AudioEngine::samples_per_bar / 8;

    BaseAudioEvent* audioEvent1 = enqueuedAudioEvent( instrument1, bufferSize, 0, 16, 0 );

    std::vector<BaseCacheableAudioEvent*>* events = Sequencer::collectCacheableSequencerEvents( 0, bufferSize );

    EXPECT_EQ( 0, events->size() )
        << "expected to have collected no cacheable events (none have been added yet)";

    delete events;
    BaseAudioEvent* audioEvent2 = new BaseCacheableAudioEvent( instrument2 );
    audioEvent2->setSampleLength( bufferSize );
    audioEvent2->positionEvent( 0, 16, 0 );
    audioEvent2->addToSequencer();

    events = Sequencer::collectCacheableSequencerEvents( 0, bufferSize );

    EXPECT_EQ( 1, events->size() )
        << "expected to have collected one cacheable event";

    ASSERT_TRUE( events->at( 0 ) == audioEvent2 )
        << "expected to have retrieved the cacheable event";

    // free allocated memory
    delete channels;
    delete events;
    delete audioEvent1;
    delete audioEvent2;
    delete instrument1;
    delete instrument2;

    // reset Sequencer
    Sequencer::playing = false;
}
*/

TEST( Sequencer, IgnoreEventsForMutedChannels )
{
    // setup sequencer
    Sequencer::playing = true;

    std::vector<AudioChannel*>* channels = new std::vector<AudioChannel*>();
    BaseInstrument* instrument1 = new BaseInstrument();
    BaseInstrument* instrument2 = new BaseInstrument();

    // setup audio events

    BaseAudioEvent* audioEvent1 = enqueuedAudioEvent( instrument1, 512, 0, 16, 0 );
    BaseAudioEvent* audioEvent2 = enqueuedAudioEvent( instrument2, 512, 0, 16, 0 );

    // collect events

    Sequencer::getAudioEvents( channels, 0, audioEvent1->getSampleLength(), true, true );

    EXPECT_EQ( 2, channels->size() )
        << "expected to have collected events for 2 AudioChannels";

    EXPECT_EQ( 1, channels->at( 0 )->audioEvents.size() )
        << "expected to have collected one event for AudioChannel 1";

    EXPECT_EQ( 1, channels->at( 1 )->audioEvents.size() )
        << "expected to have collected one event for AudioChannel 2";

    // mute instrument 2 channel

    instrument2->audioChannel->muted = true;
    Sequencer::getAudioEvents( channels, 0, audioEvent1->getSampleLength(), true, true );

    EXPECT_EQ( 1, channels->size() )
        << "expected to have collected events for only 1 AudioChannel (other was muted)";

    EXPECT_EQ( 1, channels->at( 0 )->audioEvents.size() )
        << "expected to have collected one event for AudioChannel 1";

    // mute instrument 1 channel too

    instrument1->audioChannel->muted = true;
    Sequencer::getAudioEvents( channels, 0, audioEvent1->getSampleLength(), true, true );

    EXPECT_EQ( 0, channels->size() )
        << "expected to have collected no events as all AudioChannels are muted";

    // free allocated memory

    delete channels;
    delete audioEvent1;
    delete audioEvent2;
    delete instrument1;
    delete instrument2;

    // reset Sequencer
    Sequencer::playing = false;
}

TEST( Sequencer, EventClearing )
{
    BaseInstrument* instrument1 = new BaseInstrument();
    BaseInstrument* instrument2 = new BaseInstrument();

    int amountOfEvents1 = randomInt( 1, 10 );
    int amountOfEvents2 = randomInt( 1, 10 );

    std::vector<BaseAudioEvent*> events;

    for ( int i = 0; i < amountOfEvents1; ++i ) {
        BaseAudioEvent* event = new BaseAudioEvent( instrument1 );
        event->addToSequencer();
        events.push_back( event );
    }

    for ( int i = 0; i < amountOfEvents2; ++i ) {
        BaseAudioEvent* event = new BaseAudioEvent( instrument2 );
        event->addToSequencer();
        events.push_back( event );
    }

    ASSERT_TRUE( instrument1->hasEvents() )
        << "expected instrument to contain events";

    ASSERT_TRUE( instrument2->hasEvents() )
        << "expected instrument to contain events";

    Sequencer::clearEvents();

    ASSERT_FALSE( instrument1->hasEvents() )
        << "expected instrument to contain no events after clearing";

    ASSERT_FALSE( instrument2->hasEvents() )
        << "expected instrument to contain no events after clearing";

    for ( int i = 0; i < events.size(); ++i )
        delete events.at( i );

    delete instrument1;
    delete instrument2;
}
