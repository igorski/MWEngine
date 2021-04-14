#include "../sequencercontroller.h"
#include "../utilities/volumeutil.h"

TEST( SequencerController, StepsConstructor )
{
    AudioEngine::max_step_position = 2;

    SequencerController* controller = new SequencerController();

    EXPECT_EQ( 15, AudioEngine::max_step_position )
        << "expected AudioEngine to have been reconfigured as a 16 step sequencer upon controller construction";

    delete controller;
}

TEST( SequencerController, Prepare )
{
    SequencerController* controller = new SequencerController();

    float tempo      = randomFloat( 40.0f, 300.0f );
    int tsBeatAmount = randomInt( 2, 12 );
    int tsBeatUnit   = randomInt( 2, 8 );

    controller->prepare( tempo, tsBeatAmount, tsBeatUnit );

    EXPECT_EQ( tempo, AudioEngine::queuedTempo )
        << "expected tempo to have been enqueued";

    EXPECT_EQ( tsBeatAmount, AudioEngine::queuedTime_sig_beat_amount )
        << "expected time signature beat amount to have been enqueued";

    EXPECT_EQ( tsBeatUnit, AudioEngine::queuedTime_sig_beat_unit )
        << "expected time signature beat amount to have been enqueued";

    EXPECT_EQ( 0, AudioEngine::min_buffer_position )
        << "expected min buffer position to be 0";

    EXPECT_EQ( AudioEngine::samples_per_bar - 1, AudioEngine::max_buffer_position )
        << "expected max buffer position to equal the calculated samples per bar - 1";

    delete controller;
}

TEST( SequencerController, GetSetTempo )
{
    SequencerController* controller = new SequencerController();

    int orgTempo       = 120.0f;
    AudioEngine::tempo = orgTempo;
    float newTempo     = orgTempo;

    while ( newTempo == orgTempo )
        newTempo = randomFloat( 40.0f, 300.0f );

    AudioEngine::time_sig_beat_amount = 3;
    AudioEngine::time_sig_beat_unit   = 4;

    EXPECT_EQ( AudioEngine::tempo, controller->getTempo() )
        << "expected SequencerController to return the AudioEngines tempo value";

    controller->setTempo( newTempo, 12, 8 );

    EXPECT_EQ( orgTempo, controller->getTempo() )
        << "expected SequencerController to not have updated the tempo to the new value immediately";

    EXPECT_EQ( newTempo, AudioEngine::queuedTempo )
        << "expected SequencerController to have enqueued the requested tempo into the AudioEngine";

    EXPECT_EQ( 3, AudioEngine::time_sig_beat_amount )
        << "expected SequencerController to not have updated the time signature immediately";

    EXPECT_EQ( 4, AudioEngine::time_sig_beat_unit )
        << "expected SequencerController to not have updated the time signature immediately";

    EXPECT_EQ( 12, AudioEngine::queuedTime_sig_beat_amount )
        << "expected SequencerController to have enqueued the requested time signature";

    EXPECT_EQ( 8, AudioEngine::queuedTime_sig_beat_unit )
        << "expected SequencerController to have enqueued the requested time signature";

    controller->setTempoNow( newTempo, 12, 8 );

    EXPECT_EQ( newTempo, controller->getTempo() )
        << "expected SequencerController to have updated the tempo to the new value immediately";

    EXPECT_EQ( 12, AudioEngine::time_sig_beat_amount )
        << "expected SequencerController to have updated the time signature immediately";

    EXPECT_EQ( 8, AudioEngine::time_sig_beat_unit )
        << "expected SequencerController to have updated the time signature immediately";

    delete controller;
}

TEST( SequencerController, SetVolume )
{
    SequencerController* controller = new SequencerController();

    float volume        = randomFloat( 0.1f, 1.0f );
    float scaledVolume  = VolumeUtil::toLog( volume );
    AudioEngine::volume = randomFloat( 0.0f, 0.05f );

    controller->setVolume( volume );

    EXPECT_EQ( scaledVolume, AudioEngine::volume )
        << "expected SequencerController to have updated the engine output volume against a logarithmic scale";

    delete controller;
}

TEST( SequencerController, SetPlaying )
{
    SequencerController* controller = new SequencerController();

    ASSERT_FALSE( Sequencer::playing )
        << "expected Sequencer not to be playing during controller construction";

    controller->setPlaying( true );

    ASSERT_TRUE( Sequencer::playing )
        << "expected Sequencer to be playing";

    controller->setPlaying( false );

    ASSERT_FALSE( Sequencer::playing )
        << "expected Sequencer not to be playing";

    delete controller;
}

TEST( SequencerController, SetLoopRange )
{
    SequencerController* controller = new SequencerController();

    int startPos = randomInt( 0, 5512 );
    int endPos   = randomInt( startPos + 1, startPos + 44100 );
    int steps    = randomInt( 2, 16 );

    // ensure current max step is unequal to the target position

    AudioEngine::max_step_position = 0;
    int currentBufferPosition      = randomInt( startPos, endPos );
    AudioEngine::bufferPosition    = currentBufferPosition;

    // test 1. test whether min/max buffer positions are updated

    controller->setLoopRange( startPos, endPos, steps );

    EXPECT_EQ( startPos, AudioEngine::min_buffer_position )
        << "expected minimum buffer position to have been updated";

    EXPECT_EQ( endPos, AudioEngine::max_buffer_position )
        << "expected maximum buffer position to have been updated";

    EXPECT_EQ( steps - 1, AudioEngine::max_step_position )
        << "expected maximum step position to have been updated";

    EXPECT_EQ( currentBufferPosition, AudioEngine::bufferPosition )
        << "expected buffer position NOT to have been updated (was within range)";

    // test 2. test whether buffer position is sanitized

    AudioEngine::bufferPosition = startPos - 1; // force current position to be below range
    controller->setLoopRange( startPos, endPos, steps );

    EXPECT_EQ( AudioEngine::min_buffer_position, AudioEngine::bufferPosition )
        << "expected buffer position to have been sanitized as it was out of range";

    AudioEngine::bufferPosition = endPos + 1; // force current position to be above range
    controller->setLoopRange( startPos, endPos, steps );

    EXPECT_EQ( AudioEngine::min_buffer_position, AudioEngine::bufferPosition )
        << "expected buffer position to have been sanitized as it was out of range";

    // test 3. test whether step position is sanitized

    AudioEngine::stepPosition = -1; // force current position to be below range
    controller->setLoopRange( startPos, endPos, steps );

    EXPECT_EQ( AudioEngine::min_step_position, AudioEngine::stepPosition )
        << "expected step position to have been sanitized as it was out of range";

    AudioEngine::bufferPosition = steps + 1; // force current position to be above range
    controller->setLoopRange( startPos, endPos, steps );

    EXPECT_EQ( AudioEngine::min_step_position, AudioEngine::stepPosition )
        << "expected step position to have been sanitized as it was out of range";

    delete controller;
}

TEST( SequencerController, GetStepPosition )
{
    SequencerController* controller = new SequencerController();

    AudioEngine::stepPosition = randomInt( 1, 16 );

    EXPECT_EQ( AudioEngine::stepPosition, controller->getStepPosition() )
        << "expected SequencerController to have returned the current step position";

    delete controller;
}

TEST( SequencerController, GetSetBufferPosition )
{
    SequencerController* controller = new SequencerController();

    int minBufferPos = randomInt( 0, 5512 );
    int maxBufferPos = randomInt( minBufferPos + 1, minBufferPos + 44100 );

    AudioEngine::min_buffer_position = minBufferPos;
    AudioEngine::max_buffer_position = maxBufferPos;
    AudioEngine::bufferPosition      = randomInt( minBufferPos, maxBufferPos );

    // test 1. test getter

    EXPECT_EQ( AudioEngine::bufferPosition, controller->getBufferPosition() )
        << "expected SequencerController to return the buffer position";

    // test 2. test setter

    int newPosition = randomInt( minBufferPos, maxBufferPos );
    controller->setBufferPosition( newPosition );

    EXPECT_EQ( newPosition, controller->getBufferPosition() )
        << "expected SequencerController to have updated the buffer position";

    // test 3. sanitation of existing out-of-range position

    newPosition = minBufferPos - 1; // force current position to be below range

    controller->setBufferPosition( newPosition );

    EXPECT_EQ( minBufferPos, controller->getBufferPosition() )
        << "expected SequencerController to have sanitized the current buffer position";

    newPosition = maxBufferPos + 1; // force current position to be above range

    controller->setBufferPosition( newPosition );

    EXPECT_EQ( minBufferPos, controller->getBufferPosition() )
        << "expected SequencerController to have sanitized the current buffer position";

    delete controller;
}

TEST( SequencerController, GetSamplesPerBeat )
{
    SequencerController* controller = new SequencerController();

    AudioEngine::samples_per_beat = randomInt( 512, 8192 );

    EXPECT_EQ( AudioEngine::samples_per_beat, controller->getSamplesPerBeat() )
        << "expected SequencerController to have returned samples per beat";

    delete controller;
}

TEST( SequencerController, GetSamplesPerStep )
{
    SequencerController* controller = new SequencerController();

    AudioEngine::samples_per_step = randomInt( 512, 8192 );

    EXPECT_EQ( AudioEngine::samples_per_step, controller->getSamplesPerStep() )
        << "expected SequencerController to have returned samples per step";

    delete controller;
}

TEST( SequencerController, GetSamplesPerBar )
{
    SequencerController* controller = new SequencerController();

    AudioEngine::samples_per_bar = randomInt( 512, 8192 );

    EXPECT_EQ( AudioEngine::samples_per_bar, controller->getSamplesPerBar() )
        << "expected SequencerController to have returned samples per bar";

    delete controller;
}

TEST( SequencerController, GetTimeSignatureBeatAmount )
{
    SequencerController* controller = new SequencerController();

    int tsBA = randomInt( 2, 8 );
    AudioEngine::time_sig_beat_amount = tsBA;

    EXPECT_EQ( tsBA, AudioEngine::time_sig_beat_amount )
        << "expected to have retrieved the correct time signature";

    delete controller;
}

TEST( SequencerController, GetTimeSignatureBeatUnit )
{
    SequencerController* controller = new SequencerController();

    int tsBU = randomInt( 4, 12 );
    AudioEngine::time_sig_beat_unit = tsBU;

    EXPECT_EQ( tsBU, AudioEngine::time_sig_beat_unit )
        << "expected to have retrieved the correct time signature";

    delete controller;
}

TEST( SequencerController, UpdateStepsPerBar )
{
    SequencerController* controller = new SequencerController();

    int steps = randomInt( 1, 12 );

    AudioEngine::stepPosition = steps - 1;
    int originalStepPosition  = AudioEngine::stepPosition;

    controller->updateStepsPerBar( steps );

    EXPECT_EQ( steps - 1, AudioEngine::max_step_position )
        << "expected maximum step position to have been updated";

    EXPECT_EQ( originalStepPosition, AudioEngine::stepPosition )
        << "expected current step position to not have been sanitized (was within range)";

    steps = randomInt( 1, 16 );
    AudioEngine::stepPosition = steps * 2;

    controller->updateStepsPerBar( steps );

    EXPECT_EQ( AudioEngine::stepPosition, AudioEngine::min_step_position )
        << "expected current step position to have been sanitized (was out of range)";

    delete controller;
}

TEST( SequencerController, UpdateMeasures )
{
    SequencerController* controller = new SequencerController();

    // setup engine with initial values

    controller->updateStepsPerBar( 2 );

    AudioEngine::amount_of_bars      = 1;
    AudioEngine::samples_per_bar     = 88200;
    AudioEngine::max_step_position   = 1;
    AudioEngine::max_buffer_position = AudioEngine::samples_per_bar - 1;

    // update values

    int measureAmount = randomInt( 2, 10 );
    int newSteps = randomInt( 4, 16 );

    controller->updateMeasures( measureAmount, newSteps );

    EXPECT_EQ( measureAmount, AudioEngine::amount_of_bars )
        << "expected amount of bars to have been updated";

    EXPECT_EQ(( newSteps * measureAmount ) - 1, AudioEngine::max_step_position )
        << "expected maximum step position to have been updated";

    EXPECT_EQ(( AudioEngine::samples_per_bar * measureAmount ) - 1, AudioEngine::max_buffer_position )
        << "expected maximum buffer position to have been updated";

    delete controller;
}

TEST( SequencerController, Rewind )
{
    SequencerController* controller = new SequencerController();

    int minPosition                  = randomInt( 0, 44100 );
    int maxPosition                  = randomInt( 88200, 192000 );
    AudioEngine::min_buffer_position = minPosition;
    AudioEngine::max_buffer_position = maxPosition;

    controller->setBufferPosition( randomInt( AudioEngine::min_buffer_position + 1,
                                              AudioEngine::max_buffer_position - 1 ));

    controller->rewind();

    EXPECT_EQ( minPosition, AudioEngine::min_buffer_position )
        << "expected engine to have 'rewound'";

    delete controller;
}

TEST( SequencerController, SetNotificationMarker )
{
    SequencerController* controller = new SequencerController();

    AudioEngine::marked_buffer_position = -1;

    int marker = randomInt( 1024, 8192 );
    controller->setNotificationMarker( marker );

    EXPECT_EQ( marker, AudioEngine::marked_buffer_position )
        << "expected marked buffer position to have been updated";

    delete controller;
}
