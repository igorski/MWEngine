#include "../../events/baseaudioevent.h"
#include "../../instruments/baseinstrument.h"
#include "../../audioengine.h"

TEST( BaseAudioEvent, GettersSettersVolume )
{
    BaseAudioEvent* audioEvent = new BaseAudioEvent();

    EXPECT_EQ( audioEvent->getVolume(), MAX_PHASE )
        << "expected default volume to be equal to the maximum phase value";

    float volume = randomFloat();

    audioEvent->setVolume( volume );

    EXPECT_EQ( audioEvent->getVolume(), volume )
        << "expected volume to be equal to the set value";

    deleteAudioEvent( audioEvent );
}

TEST( BaseAudioEvent, EnabledState )
{
    BaseAudioEvent* audioEvent = new BaseAudioEvent();

    ASSERT_TRUE( audioEvent->isEnabled() )
        << "expected audio event to be enabled after construction";

    audioEvent->setEnabled( false );

    ASSERT_FALSE( audioEvent->isEnabled() )
        << "expected audio event to be disabled after disabling";

    audioEvent->setEnabled( true );

    ASSERT_TRUE( audioEvent->isEnabled() )
        << "expected audio event to be enabled after enabling";

    deleteAudioEvent( audioEvent );
}

TEST( BaseAudioEvent, LoopeableState )
{
    BaseAudioEvent* audioEvent = new BaseAudioEvent();

    ASSERT_FALSE( audioEvent->isLoopeable() )
        << "expected audio event not to be loopeable after construction";

    audioEvent->setLoopeable( true );

    ASSERT_TRUE( audioEvent->isLoopeable() )
        << "expected audio event to be loopeable after enabling loop";

    audioEvent->setLoopeable( false );

    ASSERT_FALSE( audioEvent->isLoopeable() )
        << "expected audio event not to be loopeable after disabling loop";

    deleteAudioEvent( audioEvent );
}

TEST( BaseAudioEvent, LockedState )
{
    BaseAudioEvent* audioEvent = new BaseAudioEvent();

    ASSERT_FALSE( audioEvent->isLocked() )
        << "expected audio event to be unlocked after construction";

    audioEvent->lock();

    ASSERT_TRUE( audioEvent->isLocked() )
        << "expected audio event to be locked after locking";

    audioEvent->unlock();

    ASSERT_FALSE( audioEvent->isLocked() )
        << "expected audio event to be unlocked after unlocking";

    deleteAudioEvent( audioEvent );
}

TEST( BaseAudioEvent, DeletableState )
{
    BaseAudioEvent* audioEvent = new BaseAudioEvent();

    ASSERT_FALSE( audioEvent->isDeletable() )
        << "expected audio event not to be deletable after construction";

    audioEvent->setDeletable( true );

    ASSERT_TRUE( audioEvent->isDeletable() )
        << "expected audio event to be deletable after flagging it as such";

    deleteAudioEvent( audioEvent );
}

TEST( BaseAudioEvent, AddRemoveSequencer )
{
    BaseInstrument* instrument = new BaseInstrument();
    BaseAudioEvent* audioEvent = new BaseAudioEvent( instrument );

    // expect AudioEvent not be in any of the event queues of the instrument after construction

    bool found = false;
    for ( int i = 0; i < instrument->getEvents()->size(); ++i )
    {
        if ( instrument->getEvents()->at( i ) == audioEvent )
            found = true;
    }

    ASSERT_FALSE( found )
        << "expected event not to be present in the event list after construction";

    found = false;
    for ( int i = 0; i < instrument->getLiveEvents()->size(); ++i )
    {
        if ( instrument->getLiveEvents()->at( i ) == audioEvent )
            found = true;
    }

    ASSERT_FALSE( found )
        << "expected event not to be present in the live event list after construction";

    // 1. add the event to the sequencer

    audioEvent->addToSequencer();

    // expect AudioEvent to be in the sequenced event list, not the live list

    found = false;
    for ( int i = 0; i < instrument->getEvents()->size(); ++i )
    {
        if ( instrument->getEvents()->at( i ) == audioEvent )
            found = true;
    }

    ASSERT_TRUE( found )
        << "expected event to be present in the event list after addition";

    found = false;
    for ( int i = 0; i < instrument->getLiveEvents()->size(); ++i )
    {
        if ( instrument->getLiveEvents()->at( i ) == audioEvent )
            found = true;
    }

    ASSERT_FALSE( found )
        << "expected event not to be present in the live event list after addition";

    // 2. remove the event from the sequencer

    audioEvent->removeFromSequencer();

    // expect AudioEvent not to be in the sequenced event list anymore

    found = false;
    for ( int i = 0; i < instrument->getEvents()->size(); ++i )
    {
        if ( instrument->getEvents()->at( i ) == audioEvent )
            found = true;
    }

    ASSERT_FALSE( found )
        << "expected event not to be present in the event list after removal";

    // 3. add live event to the sequencer

    audioEvent->isSequenced = false;
    audioEvent->addToSequencer();

    // expect AudioEvent to be in the live event list, not the sequenced list

    found = false;
    for ( int i = 0; i < instrument->getEvents()->size(); ++i )
    {
        if ( instrument->getEvents()->at( i ) == audioEvent )
            found = true;
    }

    ASSERT_FALSE( found )
        << "expected live event not to be present in the sequenced event list after addition";

    found = false;
    for ( int i = 0; i < instrument->getLiveEvents()->size(); ++i )
    {
        if ( instrument->getLiveEvents()->at( i ) == audioEvent )
            found = true;
    }

    ASSERT_TRUE( found )
        << "expected live event to be present in the live event list after addition";

    // 4. remove live event from sequencer

    audioEvent->removeFromSequencer();

    // expect AudioEvent not be in any of the event queues of the instrument after removal

    found = false;
    for ( int i = 0; i < instrument->getEvents()->size(); ++i )
    {
        if ( instrument->getEvents()->at( i ) == audioEvent )
            found = true;
    }

    ASSERT_FALSE( found )
        << "expected event not to be present in the event list after removal";

    found = false;
    for ( int i = 0; i < instrument->getLiveEvents()->size(); ++i )
    {
        if ( instrument->getLiveEvents()->at( i ) == audioEvent )
            found = true;
    }

    ASSERT_FALSE( found )
        << "expected event not to be present in the live event list after removal";

    delete audioEvent;
    delete instrument;
}

TEST( BaseAudioEvent, SampleProperties )
{
    BaseAudioEvent* audioEvent = new BaseAudioEvent();

    int sampleLength = randomInt( 512, 8192 );
    int sampleStart  = randomInt( 0, sampleLength / 2 );
    int expectedEnd  = sampleStart + ( sampleLength - 1 );

    audioEvent->setSampleStart ( sampleStart );
    audioEvent->setSampleLength( sampleLength );

    EXPECT_EQ( sampleStart,  audioEvent->getSampleStart() );
    EXPECT_EQ( expectedEnd,  audioEvent->getSampleEnd() );
    EXPECT_EQ( sampleLength, audioEvent->getSampleLength() );

    // test auto sanitation of properties

    audioEvent->setSampleEnd( expectedEnd * 2 );
    EXPECT_EQ( expectedEnd, audioEvent->getSampleEnd() )
        << "expected sample end not to exceed the range set by the sample start and length properties";

    sampleLength /= 2;
    audioEvent->setSampleLength( sampleLength );
    expectedEnd = sampleStart + ( sampleLength - 1 );

    EXPECT_EQ( expectedEnd, audioEvent->getSampleEnd() )
        << "expected sample end not to exceed the range set by the sample start and updated length properties";

    // test non sanitation of properties for loopeable events

    audioEvent->setLoopeable( true );

    expectedEnd *= 2;
    audioEvent->setSampleEnd( expectedEnd );

    EXPECT_EQ( expectedEnd, audioEvent->getSampleEnd() )
        << "expected sample end to exceed the range set by the sample start and length properties for loopeable event";

    sampleLength /= 2;
    audioEvent->setSampleLength( sampleLength );

    EXPECT_EQ( expectedEnd, audioEvent->getSampleEnd() )
        << "expected sample end to exceed the range set by the sample start and updated length properties for loopeable event";

    deleteAudioEvent( audioEvent );
}

TEST( BaseAudioEvent, PositionEvent )
{
    BaseAudioEvent* audioEvent   = new BaseAudioEvent();
    AudioEngine::samples_per_bar = randomInt( 11025, 88200 );

    int sampleLength = randomInt( 24, 8192 );
    audioEvent->setSampleLength( sampleLength );

    int startMeasure = randomInt( 0, 15 );
    int subdivisions = randomInt( 4, 128 );
    int offset       = randomInt( 0, 64 );

    audioEvent->positionEvent( startMeasure, subdivisions, offset );

    int expectedSampleStart = ( startMeasure * AudioEngine::samples_per_bar ) +
                              ( offset * AudioEngine::samples_per_bar / subdivisions );
    int expectedSampleEnd   = expectedSampleStart + sampleLength - 1;

    EXPECT_EQ( expectedSampleStart, audioEvent->getSampleStart() );
    EXPECT_EQ( expectedSampleEnd,   audioEvent->getSampleEnd() );

    deleteAudioEvent( audioEvent );
}

TEST( BaseAudioEvent, Buffers )
{
    BaseAudioEvent* audioEvent = new BaseAudioEvent();
    AudioBuffer* buffer        = fillAudioBuffer( randomAudioBuffer() );

    audioEvent->setBuffer( buffer, true );

    ASSERT_TRUE( buffer == audioEvent->getBuffer() )
        << "expected AudioEvent to return set buffer";

    deleteAudioEvent( audioEvent );
}

TEST( BaseAudioEvent, MixBuffer )
{
    BaseAudioEvent* audioEvent = new BaseAudioEvent();

    int sampleLength = randomInt( 8, 24 );
    int sampleStart  = randomInt( 0, ( int )( sampleLength / 2 ));

    audioEvent->setSampleStart ( sampleStart );
    audioEvent->setSampleLength( sampleLength );

    int sampleEnd = audioEvent->getSampleEnd();

    AudioBuffer* buffer = fillAudioBuffer( new AudioBuffer( randomInt( 1, 4 ), sampleLength ));
    audioEvent->setBuffer( buffer, true );

    float volume = randomFloat();
    audioEvent->setVolume( volume );

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
    delete targetBuffer;
    delete buffer;
}

TEST( BaseAudioEvent, MixBufferLoopeableEvent )
{
    BaseAudioEvent* audioEvent = new BaseAudioEvent();

    int sourceSize            = 16;
    AudioBuffer* sourceBuffer = new AudioBuffer( 1, sourceSize );
    SAMPLE_TYPE* rawBuffer    = sourceBuffer->getBufferForChannel( 0 );
    fillAudioBuffer( sourceBuffer );

    audioEvent->setBuffer( sourceBuffer, false );
    audioEvent->setLoopeable( true );
    audioEvent->setSampleLength( 16 * 4 ); // thus will loop 4 times
    audioEvent->positionEvent ( 0, 16, 0 );

    // create an output buffer at a size smaller than the source buffer length

    int outputSize = ( int )(( double ) sourceSize * .4 );
    AudioBuffer* targetBuffer = new AudioBuffer( sourceBuffer->amountOfChannels, outputSize );

    int minBufferPos = audioEvent->getSampleStart();
    int bufferPos    = minBufferPos;
    int maxBufferPos = audioEvent->getSampleEnd();

    // test the seamless mixing over multiple iterations

    for ( ; bufferPos < maxBufferPos; bufferPos += outputSize )
    {
        // mix buffer contents

        targetBuffer->silenceBuffers();
        bool loopStarted = bufferPos + ( outputSize - 1 ) > maxBufferPos;
        int loopOffset   = ( maxBufferPos - bufferPos ) + 1;
        audioEvent->mixBuffer( targetBuffer, bufferPos, minBufferPos, maxBufferPos, loopStarted, loopOffset, false );

        // assert results

        SAMPLE_TYPE* mixedBuffer = targetBuffer->getBufferForChannel( 0 );

        for ( int i = 0; i < outputSize; ++i )
        {
            int compareOffset = ( bufferPos + i ) % sourceSize;

            EXPECT_EQ( rawBuffer[ compareOffset ], mixedBuffer[ i ] )
                << "expected mixed buffer contents to equal the source contents at mixed offset " << i << " for source offset " << compareOffset;
        }
    }

    delete targetBuffer;
    delete sourceBuffer;
    delete audioEvent;
}

TEST( BaseAudioEvent, Instrument )
{
    BaseAudioEvent* audioEvent = new BaseAudioEvent();

    ASSERT_TRUE( 0 == audioEvent->getInstrument() )
        << "expected BaseAudioEvent not to have an Instrument during construction";

    BaseInstrument* instrument = new BaseInstrument();
    audioEvent->setInstrument( instrument );

    ASSERT_TRUE( instrument == audioEvent->getInstrument() )
        << "expected AudioEvent to return the set Instrument";

    delete audioEvent;
    delete instrument;
}
