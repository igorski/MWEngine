#include "../../events/basesynthevent.h"
#include "../../instruments/synthinstrument.h"

TEST( BaseSynthEvent, InstanceId )
{
    SynthInstrument* instrument = new SynthInstrument();

    // 1. create first event

    BaseSynthEvent* audioEvent  = new BaseSynthEvent( 440.f, instrument );

    int firstInstanceId = audioEvent->instanceId;

    // 2. create second event

    BaseSynthEvent* audioEvent2 = new BaseSynthEvent( 330.f, instrument );

    EXPECT_EQ( firstInstanceId + 1, audioEvent2->instanceId )
        << "expected second BaseSynthEvent to have an id 1 higher than the first";

    delete audioEvent;
    delete audioEvent2;
    delete instrument;
}

TEST( BaseSynthEvent, GettersSettersFrequency )
{
    float frequency             = 440.f;
    SynthInstrument* instrument = new SynthInstrument();
    BaseSynthEvent* audioEvent  = new BaseSynthEvent( frequency, instrument );

    audioEvent->setFrequency( frequency );

    EXPECT_EQ( frequency, audioEvent->getFrequency() )
        << "expected frequency to be equal to the value passed in the setter method";

    frequency = 660.f;

    audioEvent->setFrequency( frequency, true );

    EXPECT_EQ( frequency, audioEvent->getBaseFrequency() )
        << "expected base frequency to be equal to the value passed in the setter method";

    delete audioEvent;
    delete instrument;
}

TEST( BaseSynthEvent, Envelopes )
{
    float frequency             = randomFloat() * 4000.f;
    SynthInstrument* instrument = new SynthInstrument();

    instrument->adsr->setAttackTime( 0.0 );
    BaseSynthEvent* audioEvent  = new BaseSynthEvent( frequency, instrument );

    EXPECT_FLOAT_EQ( audioEvent->cachedProps.releaseLevel, instrument->adsr->getSustainLevel() )
        << "expected events release level by default to equal the ADSR sustain level";

    EXPECT_FLOAT_EQ( audioEvent->cachedProps.envelope, 1.0 )
        << "expected events envelope to be 1.0 after construction for a 0 attack ADSR";

    EXPECT_EQ( audioEvent->cachedProps.envelopeOffset, 0 )
        << "expected events envelope offset to be 0 after construction";

    delete audioEvent;

    instrument->adsr->setAttackTime( 1.0f );
    audioEvent = new BaseSynthEvent( frequency, instrument );

    EXPECT_FLOAT_EQ( audioEvent->cachedProps.envelope, 0.0 )
        << "expected events envelope to be 0.0 after construction for a positive attack ADSR";

    delete audioEvent;
    delete instrument;
}

TEST( BaseSynthEvent, OscillatorPhases )
{
    float frequency             = randomFloat() * 4000.f;
    int amountOfOscillators     = randomInt( 2, 8 ); // max of eight (see BaseSynthEvent::init() !!)
    SynthInstrument* instrument = new SynthInstrument();
    instrument->setOscillatorAmount( amountOfOscillators );
    BaseSynthEvent* audioEvent  = new BaseSynthEvent( frequency, instrument );

    for ( int i = 0; i < amountOfOscillators; ++i )
    {
        SAMPLE_TYPE phase = randomFloat();

        ASSERT_TRUE( 0 == audioEvent->getPhaseForOscillator( i ))
            << "expected oscillator phase to be 0 prior to first setter invocation";

        audioEvent->setPhaseForOscillator( i, phase );

        EXPECT_EQ( phase, audioEvent->getPhaseForOscillator( i ))
            << "expected oscillator phase to equal the value passed in the setter method";
    }
    delete audioEvent;
    delete instrument;
}

TEST( BaseSynthEvent, SequencedEvent )
{
    float frequency             = randomFloat() * 4000.f;
    SynthInstrument* instrument = new SynthInstrument();
    int position                = randomInt( 0, 15 );
    float duration              = randomFloat() * 16.f;
    BaseSynthEvent* audioEvent  = new BaseSynthEvent( frequency, position, duration, instrument );

    ASSERT_TRUE( audioEvent->isSequenced )
        << "expected BaseSynthEvent to be sequenced for this constructor";

    EXPECT_EQ( position, audioEvent->getSequencePosition() )
        << "expected position to equal the value passed in the constructor";

    EXPECT_EQ( duration, audioEvent->getSequenceDuration() )
        << "expected length to equal the value passed in the constructor";

    delete audioEvent;
    delete instrument;
}

TEST( BaseSynthEvent, SetSequencePosition )
{
    SynthInstrument* instrument = new SynthInstrument();
    BaseSynthEvent* audioEvent  = new BaseSynthEvent( randomFloat(), 0, 1.0f, instrument );

    int position = randomInt( 1, 32 );

    audioEvent->setSequencePosition( position );

    EXPECT_EQ( position, audioEvent->getSequencePosition() )
        << "expected position to equal the value passed in the setter";

    EXPECT_EQ( position * AudioEngine::samples_per_step, audioEvent->getEventStart() )
        << "expected event start position in samples to have been calculated from the given position in the setter";

    delete audioEvent;
    delete instrument;
}

TEST( BaseSynthEvent, SetSequenceDuration )
{
    SynthInstrument* instrument = new SynthInstrument();
    BaseSynthEvent* audioEvent  = new BaseSynthEvent( randomFloat(), 0, 1.0f, instrument );

    float duration = randomFloat() * 32;

    audioEvent->setSequenceDuration( duration );

    EXPECT_EQ( duration, audioEvent->getSequenceDuration() )
        << "expected duration to equal the value passed in the setter";

    EXPECT_EQ( ( int ) ( duration * AudioEngine::samples_per_step ), audioEvent->getEventLength() )
        << "expected event length in samples to have been calculated from the given length in the setter";

    delete audioEvent;
    delete instrument;
}

TEST( BaseSynthEvent, LiveEvent )
{
    float frequency             = randomFloat() * 4000.f;
    SynthInstrument* instrument = new SynthInstrument();
    BaseSynthEvent* audioEvent  = new BaseSynthEvent( frequency, instrument );

    ASSERT_FALSE( audioEvent->isSequenced )
        << "expected BaseSynthEvent not be sequenced for this constructor";

    delete audioEvent;
    delete instrument;
}

// test overridden mix buffer method

TEST( BaseSynthEvent, MixBuffer )
{
    float frequency             = randomFloat() * 4000.f;
    SynthInstrument* instrument = new SynthInstrument();
    BaseSynthEvent* audioEvent  = new BaseSynthEvent( frequency, instrument );

    int sampleLength = randomInt( 8, 24 );
    audioEvent->setEventLength( sampleLength );
    audioEvent->positionEvent( randomInt( 0, 1 ), 16, randomInt( 0, 16 ));
    int sampleStart = audioEvent->getEventStart();

    int sampleEnd = audioEvent->getEventEnd();

    float volume = randomFloat();
    audioEvent->setVolume( volume );

    AudioBuffer* buffer = new AudioBuffer( AudioEngineProps::OUTPUT_CHANNELS, sampleLength );

    // synthesize contents into buffer

    instrument->synthesizer->render( buffer, audioEvent );

    //std::cout << " ss: " << sampleStart << " se: " << sampleEnd << " sl: " << sampleLength << " ch: " << buffer->amountOfChannels;

    // create a temporary buffer to write output in, ensure it is smaller than the event buffer
    AudioBuffer* targetBuffer = new AudioBuffer( buffer->amountOfChannels, randomInt( 2, 4 ));
    int buffersToWrite        = targetBuffer->bufferSize;

    ASSERT_FALSE( bufferHasContent( targetBuffer ))
        << "expected target buffer to be silent after creation, but it has content";

    // test 1. mix without loopable range

    int maxBufferPos = sampleLength * 2; // use a "loop range" larger than the size of the events length
    int minBufferPos = randomInt( 0, maxBufferPos / 2 );
    int bufferPos    = randomInt( minBufferPos, maxBufferPos - 1 );
    bool loopStarted = false;
    int loopOffset   = 0;

    // if the random bufferPosition wasn't within the events sampleStart and sampleEnd range, we expect no content

    bool expectContent = ( bufferPos >= sampleStart && bufferPos <= sampleEnd ) ||
                         (( bufferPos + buffersToWrite ) >= sampleStart && ( bufferPos + buffersToWrite ) <= sampleEnd );

    //std::cout << " expected content: " << expectContent << " for buffer size: " << buffersToWrite;
    //std::cout << " min: " << minBufferPos << " max: " << maxBufferPos << " cur: " << bufferPos;

    audioEvent->mixBuffer( targetBuffer, bufferPos, minBufferPos, maxBufferPos, loopStarted, loopOffset, false );

    // validate buffer contents after mixing

    if ( expectContent )
    {
        for ( int c = 0, ca = targetBuffer->amountOfChannels; c < ca; ++c )
        {
            SAMPLE_TYPE* buffer       = targetBuffer->getBufferForChannel( c );
            SAMPLE_TYPE* sourceBuffer = audioEvent->getBuffer()->getBufferForChannel( c );
            SAMPLE_TYPE expectedSample;

            for ( int i = 0; i < buffersToWrite; ++i )
            {
                int r = i + bufferPos; // read pointer for the source buffer

                if ( r >= maxBufferPos && !loopStarted )
                    r -= ( maxBufferPos - minBufferPos );

                if ( r >= sampleStart && r <= sampleEnd )
                {
                    r -= sampleStart; // substract audioEvent start position
                    expectedSample = sourceBuffer[ r ] * volume;
                }
                else {
                    expectedSample = 0.0;
                }
                SAMPLE_TYPE sample = buffer[ i ];

                EXPECT_EQ( expectedSample, sample )
                    << "expected mixed sample at " << i << " to be equal to the calculated expected sample at read offset " << r;
            }
        }

    }
    else {
        ASSERT_FALSE( bufferHasContent( targetBuffer ))
            << "expected target buffer to contain no content after mixing for an out-of-range buffer position";
    }

    // test 2. mixing within a loopable range (implying sequencer is starting a loop)

    targetBuffer->silenceBuffers();

    ASSERT_FALSE( bufferHasContent( targetBuffer ))
        << "expected target buffer to be silent after silencing, but it still has content";

    bufferPos     = randomInt( minBufferPos, maxBufferPos - 1 );
    loopStarted   = true;
    loopOffset    = ( maxBufferPos - bufferPos ) + 1;

    // pre calculate at which buffer iterator the looping will commence
    // loopStartIteratorPosition describes at which sequencer position the loop starts
    // loopStartWritePointer describes at which position in the targetBuffer the loop is written to
    // amountOfLoopedWrites is the amount of samples written in the loop
    // loopStartReadPointer describes at which position the samples from the source audioEvent will be read when loop starts
    // loopStartReadPointerEnd describes the last position the samples from the source audioEvent will be read for the amount of loop writes

    int loopStartIteratorPosition = maxBufferPos + 1;
    int loopStartWritePointer     = loopOffset;
    int loopStartReadPointer      = minBufferPos;
    int amountOfLoopedWrites      = ( bufferPos + buffersToWrite ) - loopStartIteratorPosition;
    int loopStartReadPointerEnd   = ( loopStartReadPointer + amountOfLoopedWrites ) - 1;

    // render into comparison buffer

    buffer->silenceBuffers();
    instrument->synthesizer->render( buffer, audioEvent );

    expectContent = ( bufferPos >= sampleStart && bufferPos <= sampleEnd ) ||
                    (( bufferPos + buffersToWrite ) >= sampleStart && ( bufferPos + buffersToWrite ) <= sampleEnd ) ||
                    ( loopStartIteratorPosition > maxBufferPos && (
                        ( loopStartReadPointer >= sampleStart && loopStartReadPointer <= sampleEnd ) ||
                        ( loopStartReadPointerEnd >= sampleStart && loopStartReadPointerEnd <= sampleEnd )));

    audioEvent->mixBuffer( targetBuffer, bufferPos, minBufferPos, maxBufferPos, loopStarted, loopOffset, false );

    //std::cout << " expected content: " << expectContent << " for buffer size: " << buffersToWrite;
    //std::cout << " min: " << minBufferPos << " max: " << maxBufferPos << " cur: " << bufferPos << " loop offset: " << loopOffset;

    if ( expectContent )
    {
        for ( int c = 0, ca = targetBuffer->amountOfChannels; c < ca; ++c )
        {
            SAMPLE_TYPE* buffer       = targetBuffer->getBufferForChannel( c );
            SAMPLE_TYPE* sourceBuffer = audioEvent->getBuffer()->getBufferForChannel( c );

            for ( int i = 0; i < buffersToWrite; ++i )
            {
                SAMPLE_TYPE expectedSample = 0.0;

                int r = i + bufferPos; // read pointer for the source buffer

                if ( i >= loopOffset )
                    r = minBufferPos + ( i - loopOffset );

                if ( r >= sampleStart && r <= sampleEnd )
                {
                    r -= sampleStart; // substract audioEvent start position
                    expectedSample = sourceBuffer[ r ] * volume;
                }
                SAMPLE_TYPE sample = buffer[ i ];

                EXPECT_EQ( expectedSample, sample )
                    << "expected mixed sample at " << i << " to be equal to the calculated expected sample at read "
                    << "offset " << r << " ( sanitized from " << ( i + bufferPos ) << " )";
            }
        }
    }
    else {
        ASSERT_FALSE( bufferHasContent( targetBuffer ))
            << "expected output buffer to contain no content after mixing for an out-of-range buffer position";
    }
    delete audioEvent;
    delete instrument;
    delete targetBuffer;
    delete buffer;
}

// test overridden lock methods

TEST( BaseSynthEvent, LockedState )
{
    BaseSynthEvent* audioEvent = new BaseSynthEvent();

    ASSERT_FALSE( audioEvent->isLocked() )
        << "expected audio event to be unlocked after construction";

    audioEvent->lock();

    ASSERT_TRUE( audioEvent->isLocked() )
        << "expected audio event to be locked after locking";

    audioEvent->unlock();

    ASSERT_FALSE( audioEvent->isLocked() )
        << "expected audio event to be unlocked after unlocking";

    delete audioEvent;
}

// test overridden play/stop states

TEST( BaseSynthEvent, Play )
{
    SynthInstrument* instrument = new SynthInstrument();
    BaseSynthEvent* audioEvent  = new BaseSynthEvent( 440.0f, instrument );

    ASSERT_FALSE( audioEvent->released )
        << "expected synth event not be released after construction";

    audioEvent->released = true;
    audioEvent->enqueueRemoval( true );
    audioEvent->cachedProps.envelope       = 1.0;
    audioEvent->cachedProps.envelopeOffset = 1000;

    audioEvent->play();

    ASSERT_FALSE( audioEvent->released )
        << "expected synth event not be released after invocation of play";

    ASSERT_FALSE( audioEvent->isEnqueuedForRemoval() )
        << "expected synth event to have unset its removable flag after invocation of play";

    EXPECT_EQ( audioEvent->cachedProps.envelopeOffset, 0 )
        << "expected synth events cached envelope offset to have been reset";

    EXPECT_EQ( audioEvent->cachedProps.envelope, 0.0 )
        << "expected synth events cached envelope offset to have been reset";

    EXPECT_EQ( audioEvent->lastWriteIndex, 0 )
        << "expected synth events last write index to have been reset";

    delete audioEvent;
    delete instrument;
}

TEST( BaseSynthEvent, StopAndRemove )
{
    float frequency                = randomFloat() * 4000.f;
    SynthInstrument* instrument    = new SynthInstrument();

    // apply a positive release as we don't want immediate silence/removal of the event
    instrument->adsr->setReleaseTime( 1.0f );

    BaseSynthEvent* liveEvent      = new BaseSynthEvent( frequency, instrument );
    BaseSynthEvent* sequencedEvent = new BaseSynthEvent( frequency, 0, 512, instrument );

    ASSERT_FALSE( liveEvent->isEnqueuedForRemoval() ) << "expected event not be enqueued for removal upon construction";
    ASSERT_FALSE( sequencedEvent->isEnqueuedForRemoval() ) << "expected sequenced event not be enqueued for removal upon construction";

    // start playing events
    liveEvent->play();
    sequencedEvent->play();

    // and stop them directly
    liveEvent->stop();
    sequencedEvent->stop();

    ASSERT_TRUE( liveEvent->shouldEnqueueRemoval() )
        << "expected live event to be set for queued removal upon invocation of stop()";
    ASSERT_FALSE( liveEvent->isEnqueuedForRemoval() )
        << "expected live event not to be immediately queued for removal due to positive release phase";
    ASSERT_FALSE( sequencedEvent->isEnqueuedForRemoval() )
        << "expected sequenced event not be immediately queued for removal upon invocation of stop()";

    delete liveEvent;
    delete sequencedEvent;
    delete instrument;
}

TEST( BaseSynthEvent, Stop )
{
    SynthInstrument* instrument = new SynthInstrument();
    BaseSynthEvent* audioEvent  = new BaseSynthEvent( 440.0f, instrument );

    ASSERT_FALSE( audioEvent->released )
        << "expected synth event not be released after construction";

    audioEvent->play();
    audioEvent->stop();

    ASSERT_TRUE( audioEvent->released )
        << "expected synth event to be released after invocation of stop";

    delete audioEvent;
    delete instrument;
}

TEST( BaseSynthEvent, StopLiveEvent )
{
    SynthInstrument* instrument = new SynthInstrument();
    BaseSynthEvent* audioEvent  = new BaseSynthEvent( 440.0f, instrument );
    audioEvent->isSequenced     = false;

    instrument->adsr->setAttackTime( 1.0f );
    instrument->adsr->setDecayTime ( 2.0f );
    int expectedOffset = instrument->adsr->getReleaseStartOffset();

    audioEvent->cachedProps.envelopeOffset = 0;
    audioEvent->cachedProps.envelope       = 0.5;

    // start the event
    audioEvent->play();
    // and stop it
    audioEvent->stop();

    EXPECT_EQ( audioEvent->cachedProps.envelopeOffset, expectedOffset )
        << "expected synth event envelope offset to be at the start of the release envelopes offset";

    EXPECT_EQ( audioEvent->cachedProps.releaseLevel, audioEvent->cachedProps.envelope )
        << "expected events release level to equal the last envelope level";

    delete audioEvent;
    delete instrument;
}

// test overridden event duration state

TEST( BaseSynthEvent, GetEventEnd )
{
    float frequency             = randomFloat() * 4000.f;
    SynthInstrument* instrument = new SynthInstrument();
    BaseSynthEvent* audioEvent  = new BaseSynthEvent( frequency, instrument );

    instrument->adsr->setReleaseTime( 0.0 );

    // set a non-existing release envelope
    instrument->adsr->setReleaseTime( 0 );

    int eventStart  = 0;
    int eventLength = 88200;
    int expectedEnd = eventStart + ( eventLength - 1 );

    audioEvent->setEventStart ( eventStart );
    audioEvent->setEventLength( eventLength );

    EXPECT_EQ( audioEvent->getEventEnd(), expectedEnd )
        << "expected event end to equal the expectation of its total duration without a release envelope";

    // set a positive release envelope
    instrument->adsr->setReleaseTime( 0.5 );

    // add release duration (in samples) to expected end
    expectedEnd += instrument->adsr->getReleaseDuration();

    EXPECT_EQ( audioEvent->getEventEnd(), expectedEnd )
        << "expected event end to include the positive release duration";

    delete audioEvent;
    delete instrument;
}

TEST( BaseSynthEvent, RepositionToTempoChange )
{
    SynthInstrument* instrument = new SynthInstrument();
    instrument->adsr->setReleaseTime( 0.F ); // ensure we have no release envelope extending the event end!

    BaseSynthEvent* event = new BaseSynthEvent( 440.F, instrument );
    event->isSequenced    = true;

    // sequenced SynthEvents determine their position and length via constructor relative
    // to the sequence. For this test we want to verify directly at the sample level.

    int eventStart  = 1000;
    int eventLength = 500;
    int eventEnd    = eventStart + ( eventLength - 1 );
    
    event->setEventStart ( eventStart );
    event->setEventLength( eventLength );
    event->addToSequencer();

    EXPECT_EQ( eventEnd, event->getEventEnd() )
        << "expected the event end to match when no release envelope is present in the synthesizers ADSR module";
    
    // adjust tempo by given factor
    
    float factor = 0.5F; // increases speed
    
    event->repositionToTempoChange( factor );
    
    EXPECT_EQ(( int )( eventStart * factor ), event->getEventStart() )
        << "expected event start offset to have updated after tempo change";

    EXPECT_EQ(( int )( eventLength * factor ), event->getEventLength() )
        << "expected event length to have updated after tempo change (contrary to non-synth events)";

    EXPECT_EQ(( int )( eventEnd * factor ), event->getEventEnd() )
        << "expected event end offset to have updated after tempo change";

    // adjust tempo again by given factor
    
    factor = 2.0F;  // decreases speed (back to original)

    event->repositionToTempoChange( factor );
    
    EXPECT_EQ( eventStart, event->getEventStart() )
        << "expected event start offset to have updated after tempo change";
    
    EXPECT_EQ( eventEnd, event->getEventEnd() )
        << "expected event end offset to have updated after tempo change";
    
    EXPECT_EQ( eventLength, event->getEventLength() )
        << "expected event length to have updated after tempo change";
    
    delete instrument;
    delete event;
}
