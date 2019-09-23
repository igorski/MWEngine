#include "../audioengine.h"
#include "../audiobuffer.h"
#include "../sequencer.h"
#include "../sequencercontroller.h"
#include "../events/baseaudioevent.h"
#include "../instruments/baseinstrument.h"

TEST( AudioEngine, Start )
{
    AudioEngine::test_program = 0; // help mocked OpenSL IO identify which test is running

    // prepare engine environment
    SequencerController* controller = new SequencerController();

    // 240 BPM in 4/4 time
    controller->prepare( 240, 4, 4 );

    // mono output with 48 kHz sample rate and buffer size of 240 samples
    AudioEngine::setup( 240, 48000, 1 );

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
    float ratio    = oldTempo / newTempo;

    SequencerController* controller = new SequencerController();

    // 240 BPM in 4/4 time
    controller->prepare( 240, 4, 4 );

    // mono output with 48 kHz sample rate and buffer size of 240 samples
    AudioEngine::setup( 240, 48000, 1 );
    // define a custom loop range (rather than full 0 - end)
    AudioEngine::min_buffer_position = randomInt( 24, 5512 );
    AudioEngine::max_buffer_position = randomInt( 11025, 22050 );
    AudioEngine::bufferPosition = randomInt( AudioEngine::min_buffer_position, AudioEngine::max_buffer_position );

    controller->setTempoNow( oldTempo, 4, 4 ); // ensure tempo is applied immediately
    controller->rewind();

    int oldSPBar  = AudioEngine::samples_per_bar;
    int oldSPBeat = AudioEngine::samples_per_beat;
    int oldSPStep = AudioEngine::samples_per_step;
    int oldBP     = AudioEngine::bufferPosition;
    int oldMinBP  = AudioEngine::min_buffer_position;
    int oldMaxBP  = AudioEngine::max_buffer_position;

    int expectedMinBP = ( int )( oldMinBP * ratio );
    int expectedMaxBP = expectedMinBP + ( int )(( float )( oldMaxBP - oldMinBP ) * ratio );
    int expectedBP    = ( int )( oldBP * ratio );

    EXPECT_EQ( oldTempo, controller->getTempo() )
        << "expected tempo to be set prior to engine start";

    // request new tempo and time signature via controller

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

    EXPECT_EQ( AudioEngine::min_buffer_position, expectedMinBP )
        << "expected engine to have updated its max buffer position after tempo change by ratio";

    EXPECT_EQ( AudioEngine::max_buffer_position, expectedMaxBP )
        << "expected engine to have updated its max buffer position after tempo change by ratio";

    EXPECT_EQ( AudioEngine::bufferPosition, expectedBP )
        << "expected engine to have updated its current buffer position after tempo change by ratio";

    ASSERT_FALSE( AudioEngine::samples_per_bar == oldSPBar )
        << "expected engine to have updated its samples per bar value after tempo change";

    ASSERT_FALSE( AudioEngine::samples_per_beat == oldSPBeat )
        << "expected engine to have updated its samples per beat value after tempo change";

    ASSERT_FALSE( AudioEngine::samples_per_step == oldSPStep )
        << "expected engine to have updated its samples per step value after tempo change";

    EXPECT_EQ( 2, AudioEngine::test_program )
        << "expected program to have incremented";

    delete controller;
}

TEST( AudioEngine, Output )
{
    AudioEngine::test_program    = 2;   // help mocked OpenSL IO identify which test is running
    AudioEngine::test_successful = false;

    // prepare engine environment

    SequencerController* controller = new SequencerController();

    // 130 BPM in 4/4 time
    controller->prepare( 1340, 4, 4 );

    // stereo output at 48 kHz sample rate and buffer size of 16 samples
    AudioEngine::setup( 16, 48000, 2 );

    controller->setTempoNow( 130.0f, 4, 4 );
    controller->rewind();

    AudioEngine::volume = 1;    // QQQ : later on we test mix volume ;)

    // create three AudioEvents where each will have the following buffers
    SAMPLE_TYPE event1buffer[] = { -1,-1,-1,-1,0,0,0,0,1,1,1,1,0,0,0,0 };
    SAMPLE_TYPE event2buffer[] = { .5,.5,.5,.5,1,1,1,1,-.5,-.5,-.5,-.5,-1,-1,-1,-1 };
    SAMPLE_TYPE event3buffer[] = { .25,.25,.25,.25,0,0,0,0,-.25,-.25,-.25,-.25,0,0,0,0 };

    BaseInstrument* instrument = new BaseInstrument();

    // event1 has a mono buffer used for testing that mono content is
    // written to all output channels

    BaseAudioEvent* event1 = new BaseAudioEvent( instrument );
    AudioBuffer* buffer1   = new AudioBuffer( 1, 16 );

    for ( int i = 0; i < 16; ++i )
        buffer1->getBufferForChannel( 0 )[ i ] = event1buffer[ i ];

    event1->setBuffer( buffer1, false );
    event1->setEventLength( buffer1->bufferSize );
    event1->addToSequencer();

    // event2 has a stereo buffer that only has content in the right channel
    // used for testing that multichannel audio is renderer to the output correctly

    BaseAudioEvent* event2 = new BaseAudioEvent( instrument );
    AudioBuffer* buffer2   = new AudioBuffer( 2, 16 );

    for ( int i = 0; i < 16; ++i )
        buffer2->getBufferForChannel( 1 )[ i ] = event2buffer[ i ];

    event2->setBuffer( buffer2, false );
    event2->setEventLength( buffer2->bufferSize );
    event2->setEventStart( 16 ); // after event1 has finished playing
    event2->addToSequencer();

    // event3 has buffer contents in the left and right channels and overlaps
    // event2. this is used for testing sample summing

    BaseAudioEvent* event3 = new BaseAudioEvent( instrument );
    AudioBuffer* buffer3   = new AudioBuffer( 2, 16 );

    for ( int i = 0; i < 16; ++i ) {
        buffer3->getBufferForChannel( 0 )[ i ] = event3buffer[ i ];
        buffer3->getBufferForChannel( 1 )[ i ] = event3buffer[ i ];
    }
    event3->setBuffer( buffer3, false );
    event3->setEventLength( buffer3->bufferSize );
    event3->setEventStart( 24 ); // start half way through event2
    event3->addToSequencer();

    // max buffer position is at the end of the last event
    AudioEngine::min_buffer_position = 0;
    AudioEngine::max_buffer_position = event3->getEventStart() + event3->getEventLength();

    // start the engine

    controller->setPlaying( true );
    AudioEngine::start();

    // evaluate results (assertions are made in mock_opensl_io.cpp)

    ASSERT_TRUE( AudioEngine::test_successful )
        << "expected test to be successful";

    EXPECT_EQ( 3, AudioEngine::test_program )
        << "expected test program to have incremented";

    // clean up

    controller->setPlaying( false );
    AudioEngine::render_iterations = 0;

    delete controller;
    delete instrument;
    delete event1;
    delete event2;
    delete event3;
    delete buffer1;
    delete buffer2;
    delete buffer3;
}

TEST( AudioEngine, OutputAtLoopStart )
{
    AudioEngine::test_program    = 3;   // help mocked OpenSL IO identify which test is running
    AudioEngine::test_successful = false;

    // setup sequencer

    std::vector<AudioChannel*>* channels = new std::vector<AudioChannel*>();
    BaseInstrument* instrument1 = new BaseInstrument();
    BaseInstrument* instrument2 = new BaseInstrument();

    // prepare engine environment

    SequencerController* controller = new SequencerController();

    // 1240 BPM in 4/4 time
    controller->prepare( 120, 4, 4 );

    // mono output with 44.1 kHz sample rate and buffer size of 11025 samples
    AudioEngine::setup( 11025, 44100, 1 );

    controller->setTempoNow( 120.0f, 4, 4 );
    controller->rewind();

    // setup audio events

    // test is working at 88200 samples per bar (emulates 44.1 kHz sample rate at 120 BPM 4/4 time)
    // use a buffer size that is the size of an 8th note (11025 samples)

    AudioEngine::min_buffer_position = 0;
    AudioEngine::max_buffer_position = AudioEngine::samples_per_bar - 1;
    Sequencer::playing               = true;
    int bufferSize                   = AudioEngineProps::BUFFER_SIZE;

    // audioEvent1 start: 77175 end: 88199 (length: 11025)
    // audioEvent2 start: 0 end: 11024 (length: 11025)

    BaseAudioEvent* audioEvent1 = enqueuedAudioEvent( instrument1, bufferSize, 0, 16, 14 );
    BaseAudioEvent* audioEvent2 = enqueuedAudioEvent( instrument2, bufferSize, 0, 16, 0 );

    // fill event 1 and event 2 with values (are validated in mock_opensl_io)

    AudioBuffer* buffer1 = new AudioBuffer( 1, audioEvent1->getEventLength() );
    AudioBuffer* buffer2 = new AudioBuffer( 1, audioEvent2->getEventLength() );

    for ( int i = 0; i < buffer1->bufferSize; ++i )
        buffer1->getBufferForChannel( 0 )[ i ] = ( SAMPLE_TYPE ) -0.25;

    for ( int i = 0; i < buffer2->bufferSize; ++i )
        buffer2->getBufferForChannel( 0 )[ i ] = ( SAMPLE_TYPE ) +0.5;

    audioEvent1->setBuffer( buffer1, false );
    audioEvent2->setBuffer( buffer2, false );

    // test check if Sequencer returns events when loop is about to start
    // e.g. start offset is 88100 while there are 88200 samples in the measure / loop range
    // buffer size is 11025 samples, we expect to collect events at offsets 88100 to 88199
    // and events at offset 0 to 10924 (as we read ( max_buffer_position - startOffset ) + 1 ) samples from the start
    // of the requested range and can subtract this value from the bufferSize to calculate how many samples
    // we need to read from the start of the full Sequencer loop range, as such the calculation is:
    // ( 11025 - (( 88199 - 88100 ) + 1 )) == 10925 samples to read from start (which is min_buffer_position == 0 )

    // start the engine

    AudioEngine::bufferPosition = 88100;
    AudioEngine::volume         = 1;
    controller->setPlaying( true );
    AudioEngine::start();

    // evaluate results (assertions are made in mock_opensl_io.cpp)

    ASSERT_TRUE( AudioEngine::test_successful )
        << "expected test to be successful";

    EXPECT_EQ( 4, AudioEngine::test_program )
        << "expected test program to have incremented";

    // clean up

    controller->setPlaying( false );
    AudioEngine::render_iterations = 0;

    delete channels;
    delete controller;
    delete audioEvent1;
    delete audioEvent2;
    delete buffer1;
    delete buffer2;
    delete instrument1;
    delete instrument2;
}
