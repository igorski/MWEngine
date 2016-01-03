#include "../audiobuffer.h"
#include "../audioengine.h"
#include "../sequencer.h"
#include "../sequencercontroller.h"
#include "../events/baseaudioevent.h"
#include "../instruments/baseinstrument.h"

TEST( AudioEngine, Start )
{
    AudioEngine::test_program = 0; // help mocked OpenSL IO identify which test is running

    // prepare engine environment

    SequencerController* controller = new SequencerController();
    controller->prepare( 240, 48000, 130.0f, 4, 4 ); // 130 BPM in 4/4 time at 48 kHz sample rate w/buffer size of 240 samples

    AudioEngine::engine_started = false;

    AudioEngine::start();

    EXPECT_EQ( 1, AudioEngine::test_program )
        << "expected program to have incremented";

    ASSERT_TRUE( AudioEngine::engine_started )
        << "expected engine to have started";

    delete controller;
}

TEST( AudioEngine, TempoUpdate )
{
    AudioEngine::test_program = 1; // help mocked OpenSL IO identify which test is running

    // prepare engine environment

    float oldTempo = randomFloat( 40.0f,  199.0f );
    float newTempo = randomFloat( 120.0f, 300.0f );

    SequencerController* controller = new SequencerController();
    controller->prepare( 240, 48000, oldTempo, 4, 4 );
    controller->setTempoNow( oldTempo, 4, 4 ); // ensure tempo is applied immediately
    controller->rewind();

    int oldSPBar  = AudioEngine::samples_per_bar;
    int oldSPBeat = AudioEngine::samples_per_beat;
    int oldSPStep = AudioEngine::samples_per_step;
    int oldMaxBP  = AudioEngine::max_buffer_position = randomInt( 11025, 8820  );

    EXPECT_EQ( oldTempo, controller->getTempo() )
        << "expected tempo to be set prior to engine start";

    // request new tempo via controller

    controller->setTempo( newTempo, 12, 8 );

    EXPECT_EQ( oldTempo, controller->getTempo() )
        << "expected tempo at the old value to be set prior to engine start";

    EXPECT_EQ( 4, controller->getTimeSigBeatAmount() )
        << "expected time signature to be at the old value prior to the engine start";

    EXPECT_EQ( 4, controller->getTimeSigBeatUnit() )
        << "expected time signature to be at the old value prior to the engine start";

    AudioEngine::start();
    //usleep( 50 ); // tempo update is executed after the engine is halted by the OpenSL mock

    // assert results

    EXPECT_EQ( newTempo, controller->getTempo() )
        << "expected engine to have updated the tempo during a single iteration of its render cycle";

    EXPECT_EQ( 12, controller->getTimeSigBeatAmount() )
        << "expected engine to have updated the time signature during a single iteration of its render cycle";

    EXPECT_EQ( 8, controller->getTimeSigBeatUnit() )
        << "expected engine to have updated the time signature during a single iteration of its render cycle";

    ASSERT_FALSE( AudioEngine::max_buffer_position == oldMaxBP )
        << "expected engine to have updated its max buffer position after tempo change";

    ASSERT_FALSE( AudioEngine::samples_per_bar == oldSPBar )
        << "expected engine to have updated its samples per bar value after tempo change";

    ASSERT_FALSE( AudioEngine::samples_per_beat == oldSPBeat )
        << "expected engine to have updated its samples per beat value after tempo change";

    ASSERT_FALSE( AudioEngine::samples_per_step == oldSPStep )
        << "expected engine to have updated its samples per step value after tempo change";

    delete controller;
}

TEST( AudioEngine, Output )
{
    AudioEngine::test_program = 2; // help mocked OpenSL IO identify which test is running

    // prepare engine environment

    SequencerController* controller = new SequencerController();
    controller->prepare( 16, 48000, 130.0f, 4, 4 ); // 130 BPM in 4/4 time at 48 kHz sample rate w/buffer size of 240 samples
    controller->rewind();

    AudioEngine::volume = 1;    // QQQ : later on we test mix volume ;)

    // create a SampleEvent that holds a simple waveform
    // the resulting 16 sample mono buffer contains the following samples:
    //
    // -1,-1,-1,-1,0,0,0,0,1,1,1,1,0,0,0,0
    //
    // the event will last for an entire measure in duration

    AudioBuffer* buffer    = new AudioBuffer( 1, 16 );
    SAMPLE_TYPE* rawBuffer = buffer->getBufferForChannel( 0 );

    for ( int i = 0; i < 4; ++i )
        rawBuffer[ i ] = ( SAMPLE_TYPE ) -MAX_PHASE;

    for ( int i = 4; i < 8; ++i )
        rawBuffer[ i ] = ( SAMPLE_TYPE ) 0;

    for ( int i = 8; i < 12; ++i )
        rawBuffer[ i ] = ( SAMPLE_TYPE ) MAX_PHASE;

    for ( int i = 12; i < 16; ++i )
        rawBuffer[ i ] = ( SAMPLE_TYPE ) 0;

    BaseInstrument* instrument = new BaseInstrument();
    BaseAudioEvent* event      = new BaseAudioEvent( instrument );
    event->setBuffer( buffer, false );
    event->setLoopeable( true );
    event->setSampleLength( AudioEngine::samples_per_bar );
    event->positionEvent( 0, 16, 0 );
    event->addToSequencer();

    // start the engine

    controller->setPlaying( true );

    AudioEngine::start();

    // evaluate results

    // clean up

    controller->setPlaying( false );
    AudioEngine::render_iterations = 0;

    delete controller;
    delete instrument;
    delete event;
    delete buffer;
}

/*
TEST( AudioEngine, GetAudioEventsAtLoopStart )
{
    // setup sequencer

    std::vector<AudioChannel*>* channels = new std::vector<AudioChannel*>();
    BaseInstrument* instrument1 = new BaseInstrument();
    BaseInstrument* instrument2 = new BaseInstrument();

    // setup audio events

    // assume 88200 samples per bar (emulates 44.1 kHz sample rate at 120 BPM 4/4 time)
    // use a buffer size that is the size of an 8th note (11025 samples)
    AudioEngine::samples_per_bar     = 88200;
    AudioEngine::min_buffer_position = 0;
    AudioEngine::max_buffer_position = AudioEngine::samples_per_bar - 1;
    Sequencer::playing               = true;
    int bufferSize                   = AudioEngine::samples_per_bar / 8;

    BaseAudioEvent* audioEvent1 = enqueuedAudioEvent( instrument1, bufferSize, 0, 16, 14 );
    BaseAudioEvent* audioEvent2 = enqueuedAudioEvent( instrument2, bufferSize, 0, 16, 0 );
    // audioEvent1 start: 77175 end: 88199 (length: 11025)
    // audioEvent2 start: 0 end: 11024

    // test check if Sequencer returns events when loop is about to start
    // e.g. start offset is 88100 while there are 88200 samples in the measure / loop range
    // buffer size is 11025 samples, we expect to collect events at offsets 88100 to 88199
    // and events at offset 0 to 10924 (as we read ( max_buffer_position - startOffset ) + 1 ) samples from the start
    // of the requested range and can subtract this value from the bufferSize to calculate how many samples
    // we need to read from the start of the full Sequencer loop range, as such the calculation is:
    // ( 11025 - (( 88199 - 88100 ) + 1 )) == 10925 samples to read from start (which is min_buffer_position == 0 )

    int startOffset = 88100;
    Sequencer::getAudioEvents( channels, startOffset, bufferSize, true, true );

    EXPECT_EQ( 1, channels->at( 0 )->audioEvents.size() )
        << "expected to have collected 1 event for AudioChannel 1";

    EXPECT_EQ( 1, channels->at( 1 )->audioEvents.size() )
        << "expected to have collected 1 event for AudioChannel 2";

    // free allocated memory
    delete channels;
    delete audioEvent1;
    delete audioEvent2;
    delete instrument1;
    delete instrument2;

    Sequencer::playing = false;
}
*/