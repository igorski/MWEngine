#include "../audioengine.h"
#include "../sequencer.h"
#include "../sequencercontroller.h"

TEST( AudioEngine, Start )
{
    AudioEngine::test_program = 0; // help mocked OpenSL IO identify which test is running

    // prepare engine environment

    SequencerController* controller = new SequencerController();
    controller->prepare( 48000, 240, 130.0f, 4, 4 ); // 130 BPM in 4/4 time at 48 kHz sample rate w/buffer size of 240 samples

    AudioEngine::engine_started = false;

    ASSERT_FALSE( AudioEngine::engine_started )
        << "expected engine not to have started yet";

    AudioEngine::start();

    EXPECT_EQ( 1, AudioEngine::test_program )
        << "expected program to have incremented";

    ASSERT_TRUE( AudioEngine::engine_started )
        << "expected engine to have started";

    delete controller;
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