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

    event->setEventStart ( sampleStart );
    event->setEventEnd   ( sampleEnd );
    event->setEventLength( sampleLength );
    event->addToSequencer();

    // increase tempo by given factor

    float factor = 2.0f;

    AudioEngine::tempo *= factor;

    // invoke updateEvents() (would have been executed by the Sequencer when running)

    instrument->updateEvents();

    int expectedStart = ( int )( sampleStart / factor );

    EXPECT_EQ( expectedStart, event->getEventStart() )
        << "expected event start offset to have updated after tempo change and invocation of updateEvents()";

    EXPECT_EQ( expectedStart + ( sampleLength - 1 ), event->getEventEnd() )
        << "expected event end offset to have updated after tempo change and invocation of updateEvents()";

    EXPECT_EQ( sampleLength, event->getEventLength() )
        << "expected event length not to have updated after tempo change and invocation of updateEvents()";

    // decrease tempo again by given factor

    factor = 0.5f;  // restores to original
    AudioEngine::tempo *= factor;

    instrument->updateEvents();

    EXPECT_EQ( sampleStart, event->getEventStart() )
        << "expected event start offset to have updated after tempo change and invocation of updateEvents()";

    EXPECT_EQ(( sampleEnd - 1 ), event->getEventEnd() )
        << "expected event end offset to have updated after tempo change and invocation of updateEvents()";

    EXPECT_EQ( sampleLength, event->getEventLength() )
        << "expected event length not to have updated after tempo change and invocation of updateEvents()";

    delete event;
    delete instrument;
}
