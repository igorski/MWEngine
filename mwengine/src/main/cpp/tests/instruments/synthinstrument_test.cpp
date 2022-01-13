#include "../../instruments/synthinstrument.h"
#include "../../events/basesynthevent.h"
#include "../../audioengine.h"

TEST( SynthInstrument, UpdateEvents )
{
    // test for tempo change updates
    // this differs from the BaseInstrument as the SynthInstrument invocation of
    // the method will also adjust the event length values to match the sequencer tempo

    AudioEngine::tempo = 120.F;

    SynthInstrument* instrument = new SynthInstrument();
    BaseSynthEvent* event       = new BaseSynthEvent( 440.F, instrument );

    instrument->adsr->setReleaseTime( 0.F ); // unset release as it increases note tails (and thus end beyond duration)
    event->isSequenced  = true;

    int eventStart  = 1000;
    int eventLength = 500;
    int eventEnd    = eventStart + ( eventLength - 1 );

    event->setEventStart ( eventStart );
    event->setEventEnd   ( eventEnd );
    event->setEventLength( eventLength );
    event->addToSequencer();

    // increase tempo by given factor

    float factor = 0.5F; // previous 120 BPM / desired new 240 BPM

    // invoke updateEvents() (would have been executed by the Sequencer when running)

    instrument->updateEvents( factor );

    int expectedStart  = ( int )( eventStart * factor );
    int expectedLength = ( int )( eventLength * factor );
    int expectedEnd    = expectedStart + ( expectedLength - 1 );

    EXPECT_EQ( expectedStart, event->getEventStart() )
        << "expected event start offset to have updated after tempo change and invocation of updateEvents()";

    EXPECT_EQ( expectedEnd, event->getEventEnd() )
        << "expected event end offset to have updated after tempo change and invocation of updateEvents()";

    EXPECT_EQ( expectedLength, event->getEventLength() )
        << "expected event length to have updated after tempo change and invocation of updateEvents()";

    // decrease tempo again by given factor

    factor = 2.F; // restores to original (previous 240 BPM / desired new 120 BPM)

    instrument->updateEvents( factor );

    EXPECT_EQ( eventStart, event->getEventStart() )
        << "expected event start offset to have updated after tempo change and invocation of updateEvents()";

    EXPECT_EQ( eventEnd, event->getEventEnd() )
        << "expected event end offset to have updated after tempo change and invocation of updateEvents()";

    EXPECT_EQ( eventLength, event->getEventLength() )
        << "expected event length not to have updated after tempo change and invocation of updateEvents()";

    delete event;
    delete instrument;
}
