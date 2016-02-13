#include "../../events/basesynthevent.h"
#include "../../instruments/synthinstrument.h"

TEST( BaseSynthEvent, InstanceId )
{
    float frequency             = randomFloat() * 4000.f;
    SynthInstrument* instrument = new SynthInstrument();

    // 1. create first event

    BaseSynthEvent* audioEvent  = new BaseSynthEvent( frequency, instrument );

    int firstInstanceId = audioEvent->instanceId;

    // 2. create second event

    BaseSynthEvent* audioEvent2 = new BaseSynthEvent( frequency, instrument );

    EXPECT_EQ( firstInstanceId + 1, audioEvent2->instanceId )
        << "expected second BaseSynthEvent to have an id 1 higher than the first";

    // 3. delete events (should decrement instance ids)

    deleteAudioEvent( audioEvent );
    deleteAudioEvent( audioEvent2 );

    // 4. create third event
    // TODO: destructor doesn't seem to do anything ??
//    audioEvent = new BaseSynthEvent( frequency, instrument );
//
//    EXPECT_EQ( firstInstanceId, audioEvent->instanceId )
//        << "expected old instance id to be equal to the new BaseAudioEvents id as the old events have been disposed";
//
//    deleteAudioEvent( audioEvent );
}

TEST( BaseSynthEvent, GettersSetters )
{
    float frequency             = randomFloat() * 4000.f;
    SynthInstrument* instrument = new SynthInstrument();
    BaseSynthEvent* audioEvent  = new BaseSynthEvent( frequency, instrument );

    audioEvent->setFrequency( frequency );

    EXPECT_EQ( frequency, audioEvent->getFrequency() )
        << "expected frequency to be equal to the value passed in the setter method";

    frequency = randomFloat() * 4000.f;

    audioEvent->setFrequency( frequency, true );

    EXPECT_EQ( frequency, audioEvent->getBaseFrequency() )
        << "expected base frequency to be equal to the value passed in the setter method";

    deleteAudioEvent( audioEvent );
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
    deleteAudioEvent( audioEvent );
}

TEST( BaseSynthEvent, SequencedEvent )
{
    float frequency             = randomFloat() * 4000.f;
    SynthInstrument* instrument = new SynthInstrument();
    int position                = randomInt( 0, 15 );
    float length                = randomFloat() * 16.f;
    BaseSynthEvent* audioEvent  = new BaseSynthEvent( frequency, position, length, instrument );

    ASSERT_TRUE( audioEvent->isSequenced )
        << "expected BaseSynthEvent to be sequenced for this constructor";

    EXPECT_EQ( position, audioEvent->position )
        << "expected position to equal the value passed in the constructor";

    EXPECT_EQ( length, audioEvent->length )
        << "expected length to equal the value passed in the constructor";

    deleteAudioEvent( audioEvent );
}

TEST( BaseSynthEvent, PropertyInvalidation )
{
    float frequency             = randomFloat() * 4000.f;
    SynthInstrument* instrument = new SynthInstrument();
    int position                = randomInt( 0, 15 );
    float length                = randomFloat() * 16.f;
    BaseSynthEvent* audioEvent  = new BaseSynthEvent( frequency, position, length, instrument );

    position = randomInt( 0, 15 );
    length   = randomFloat() * 16;
    SynthInstrument* newInstrument = new SynthInstrument();

    audioEvent->invalidateProperties( position, length, newInstrument );

    EXPECT_EQ( position, audioEvent->position )
        << "expected position to equal the value passed in the invalidation method";

    EXPECT_EQ( length, audioEvent->length )
        << "expected length to equal the value passed in the invalidation method";

    ASSERT_FALSE( instrument == audioEvent->getInstrument() )
        << "expected the original instrument to have been replaced by the invalidation method";

    ASSERT_TRUE( newInstrument == audioEvent->getInstrument() )
       << "expected the newly set instrument to have been returned";

    deleteAudioEvent( audioEvent );
}

TEST( BaseSynthEvent, LiveEvent )
{
    float frequency             = randomFloat() * 4000.f;
    SynthInstrument* instrument = new SynthInstrument();
    BaseSynthEvent* audioEvent  = new BaseSynthEvent( frequency, instrument );

    ASSERT_FALSE( audioEvent->isSequenced )
        << "expected BaseSynthEvent not be sequenced for this constructor";

    deleteAudioEvent( audioEvent );
}

// test overridden mix buffer method

TEST( BaseSynthEvent, MixBuffer )
{
    float frequency             = randomFloat() * 4000.f;
    SynthInstrument* instrument = new SynthInstrument();
    BaseSynthEvent* audioEvent  = new BaseSynthEvent( frequency, instrument );

    int sampleLength = randomInt( 8, 24 );
    audioEvent->setSampleLength( sampleLength );
    audioEvent->positionEvent( randomInt( 0, 1 ), 16, randomInt( 0, 16 ));
    int sampleStart = audioEvent->getSampleStart();

    int sampleEnd = audioEvent->getSampleEnd();

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
    deleteAudioEvent( audioEvent );
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
