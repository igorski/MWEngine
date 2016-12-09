#include "../../instruments/sampledinstrument.h"
#include "../../events/sampleevent.h"

TEST( SampledInstrument, UpdateEvents )
{
    // test for tempo change updates
    // this differs from the BaseInstrument as the SampledInstrument
    // should not adjust the sampleEnd and sampleLength values as
    // the sampleLength cannot exceed the actual SampleEvents bufferSize

    AudioEngine::tempo = 120.0f;

    BaseInstrument* instrument = new SampledInstrument();
    BaseAudioEvent* event      = new SampleEvent( instrument );

    int sampleStart  = 1000;
    int sampleLength = 500;
    int sampleEnd    = sampleStart + sampleLength;

    event->setSampleStart ( sampleStart );
    event->setSampleEnd   ( sampleEnd );
    event->setSampleLength( sampleLength );
    event->addToSequencer();

    // increase tempo by given factor

    float factor = 2.0f;

    AudioEngine::tempo *= factor;

    // invoke updateEvents() (would have been executed by the Sequencer when running)

    instrument->updateEvents();

    EXPECT_EQ(( int )( sampleStart / factor ), event->getSampleStart() )
        << "expected event start offset to have updated after tempo change and invocation of updateEvents()";

    EXPECT_EQ(( sampleEnd - 1 ), event->getSampleEnd() )
        << "expected event end offset not to have updated after tempo change and invocation of updateEvents()";

    EXPECT_EQ( sampleLength, event->getSampleLength() )
        << "expected event length not to have updated after tempo change and invocation of updateEvents()";

    // decrease tempo again by given factor

    factor = 0.5f;  // restores to original
    AudioEngine::tempo *= factor;

    instrument->updateEvents();

    EXPECT_EQ( sampleStart, event->getSampleStart() )
        << "expected event start offset to have updated after tempo change and invocation of updateEvents()";

    EXPECT_EQ(( sampleEnd - 1 ), event->getSampleEnd() )
        << "expected event end offset not to have updated after tempo change and invocation of updateEvents()";

    EXPECT_EQ( sampleLength, event->getSampleLength() )
        << "expected event length not to have updated after tempo change and invocation of updateEvents()";

    delete event;
    delete instrument;
}
