#include "../../events/baseaudioevent.h"
#include "../../utilities/bufferutility.h"
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

    delete audioEvent;
}

TEST( BaseAudioEvent, PlayStop )
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

    // 1. activate play-state

    audioEvent->play();

    // expect event to be present in live events list

    found = false;
    for ( int i = 0; i < instrument->getEvents()->size(); ++i )
    {
        if ( instrument->getEvents()->at( i ) == audioEvent )
            found = true;
    }

    ASSERT_FALSE( found )
        << "expected event not to be present in the sequenced event list after invocation of play()";

    found = false;
    for ( int i = 0; i < instrument->getLiveEvents()->size(); ++i )
    {
        if ( instrument->getLiveEvents()->at( i ) == audioEvent )
            found = true;
    }

    ASSERT_TRUE( found )
        << "expected event to be present in the live event list after invocation of play()";

    ASSERT_TRUE( audioEvent->isEnabled() )
        << "expected SampleEvent to be enabled after invocation of play()";

    // 2. deactivate play-state

    audioEvent->stop();

    // expect event not be in the event lists anymore

    found = false;
    for ( int i = 0; i < instrument->getEvents()->size(); ++i )
    {
        if ( instrument->getEvents()->at( i ) == audioEvent )
            found = true;
    }

    ASSERT_FALSE( found )
        << "expected event not to be present in the event list after invocation of stop()";

    found = false;
    for ( int i = 0; i < instrument->getLiveEvents()->size(); ++i )
    {
        if ( instrument->getLiveEvents()->at( i ) == audioEvent )
            found = true;
    }

    ASSERT_FALSE( found )
        << "expected event not to be present in the live event list after invocation of stop()";

    delete audioEvent;
    delete instrument;
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

    delete audioEvent;
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

    delete audioEvent;
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

    delete audioEvent;
}

TEST( BaseAudioEvent, DeletableState )
{
    BaseAudioEvent* audioEvent = new BaseAudioEvent();

    ASSERT_FALSE( audioEvent->isDeletable() )
        << "expected audio event not to be deletable after construction";

    audioEvent->setDeletable( true );

    ASSERT_TRUE( audioEvent->isDeletable() )
        << "expected audio event to be deletable after flagging it as such";

    delete audioEvent;
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

TEST( BaseAudioEvent, PositionInSamples )
{
    BaseAudioEvent* audioEvent = new BaseAudioEvent();

    int eventLength = randomInt( 512, 8192 );
    int eventStart  = randomInt( 0, eventLength / 2 );
    int expectedEnd = eventStart + ( eventLength - 1 );

    audioEvent->setEventStart ( eventStart );
    audioEvent->setEventLength( eventLength );

    EXPECT_EQ( eventStart, audioEvent->getEventStart() )
        << "expected eventStart to match the set position";

    EXPECT_EQ( expectedEnd, audioEvent->getEventEnd() )
        << "expected eventEnd to match the implied end set by start + length";

    EXPECT_EQ( eventLength, audioEvent->getEventLength() )
        << "expected eventLength to match the set length";

    // test whether values in seconds have updated accordingly

    int SAMPLE_RATE = 44100;
    float expectedStartPosition = BufferUtility::bufferToSeconds( eventStart, SAMPLE_RATE );
    float expectedEndPosition   = BufferUtility::bufferToSeconds( expectedEnd, SAMPLE_RATE );
    float expectedDuration      = expectedEndPosition - expectedStartPosition;

    EXPECT_FLOAT_EQ( expectedStartPosition, audioEvent->getStartPosition() );
    EXPECT_FLOAT_EQ( expectedEndPosition,   audioEvent->getEndPosition() );
    EXPECT_FLOAT_EQ( expectedDuration,      audioEvent->getDuration() );

    // test auto sanitation of properties

    audioEvent->setEventEnd( expectedEnd * 2 );
    EXPECT_EQ( expectedEnd, audioEvent->getEventEnd() )
        << "expected event end not to exceed the range set by the event start and length properties";

    eventLength /= 2;
    audioEvent->setEventLength( eventLength );
    expectedEnd = eventStart + ( eventLength - 1 );

    EXPECT_EQ( expectedEnd, audioEvent->getEventEnd() )
        << "expected event end not to exceed the range set by the event start and updated length properties";

    // test non sanitation of properties for loopeable events

    audioEvent->setLoopeable( true );

    expectedEnd *= 2;
    audioEvent->setEventEnd( expectedEnd );

    EXPECT_EQ( expectedEnd, audioEvent->getEventEnd() )
        << "expected event end to exceed the range set by the event start and length properties for loopeable event";

    eventLength /= 2;
    audioEvent->setEventLength( eventLength );

    EXPECT_EQ( expectedEnd, audioEvent->getEventEnd() )
        << "expected event end to exceed the range set by the event start and updated length properties for loopeable event";

    delete audioEvent;
}

TEST( BaseAudioEvent, PositionInSeconds )
{
    BaseAudioEvent* audioEvent = new BaseAudioEvent();

    float startPosition = randomFloat( 0, 10 );
    float endPosition   = startPosition + randomFloat( 0, 10 );

    int SAMPLE_RATE = 44100;

    float expectedDuration  = endPosition - startPosition;
    int expectedEventStart  = BufferUtility::secondsToBuffer( startPosition, SAMPLE_RATE );
    int expectedEventEnd    = BufferUtility::secondsToBuffer( endPosition, SAMPLE_RATE );
    int expectedEventLength = ( expectedEventEnd - expectedEventStart ) - 1;
    audioEvent->setStartPosition( startPosition );

    EXPECT_FLOAT_EQ( startPosition, audioEvent->getStartPosition() );
    EXPECT_FLOAT_EQ( startPosition, audioEvent->getEndPosition() )
        << "expected end position to equal start position (hasn't been explicitly set yet)";
    EXPECT_FLOAT_EQ( 0, audioEvent->getDuration())
        << "expected zero duration (duration nor end haven't been explicitly set yet)";
    EXPECT_EQ( 0, audioEvent->getEventLength())
        << "expected zero event length (duration nor end haven't been explicitly set yet)";
    EXPECT_EQ( expectedEventStart, audioEvent->getEventStart() )
        << "expected event start to have been updated after setting start position";

    audioEvent->setEndPosition( endPosition );

    EXPECT_FLOAT_EQ( startPosition, audioEvent->getStartPosition() );
    EXPECT_FLOAT_EQ( endPosition, audioEvent->getEndPosition() );
    EXPECT_FLOAT_EQ( expectedDuration, audioEvent->getDuration() );
    EXPECT_EQ( expectedEventEnd, audioEvent->getEventEnd())
        << "expected event end to have been updated after setting end position";
    EXPECT_EQ( expectedEventLength, audioEvent->getEventLength())
        << "expected event length to have been updated after setting end position";

    expectedDuration /= 2;
    float expectedEndPosition = startPosition + expectedDuration;
    audioEvent->setDuration( expectedDuration );

    // there may be a tiny loss in floating point precision so we round the resolution

    EXPECT_FLOAT_EQ(
        floatRounding( expectedDuration, 4 ),
        floatRounding( audioEvent->getDuration(), 4 )
    );

    EXPECT_FLOAT_EQ(
        floatRounding( expectedEndPosition, 4 ),
        floatRounding( audioEvent->getEndPosition(), 4 )
    ) << "expected end position to have corrected after updating of duration";

    delete audioEvent;
}

TEST( BaseAudioEvent, PositionEvent )
{
    BaseAudioEvent* audioEvent   = new BaseAudioEvent();
    AudioEngine::samples_per_bar = randomInt( 11025, 88200 );

    int eventLength = randomInt( 24, 8192 );
    audioEvent->setEventLength( eventLength );

    int startMeasure = randomInt( 0, 15 );
    int subdivisions = randomInt( 4, 128 );
    int offset       = randomInt( 0, 64 );

    audioEvent->positionEvent( startMeasure, subdivisions, offset );

    int expectedSampleStart = ( startMeasure * AudioEngine::samples_per_bar ) +
                              ( offset * AudioEngine::samples_per_bar / subdivisions );
    int expectedSampleEnd   = expectedSampleStart + eventLength - 1;

    EXPECT_EQ( expectedSampleStart, audioEvent->getEventStart() )
        << "expected sampleStart in buffer samples to match the translated expectation";

    EXPECT_EQ( expectedSampleEnd, audioEvent->getEventEnd() )
        << "expected sampleEnd in buffer samples to match the translated expectation";

    delete audioEvent;
}

TEST( BaseAudioEvent, Buffers )
{
    BaseAudioEvent* audioEvent = new BaseAudioEvent();

    ASSERT_FALSE( audioEvent->hasBuffer() )
        << "expected event not to contain an AudioBuffer after construction";

    AudioBuffer* buffer = fillAudioBuffer( randomAudioBuffer() );

    audioEvent->setBuffer( buffer, true );

    ASSERT_TRUE( audioEvent->hasBuffer() )
        << "expected event to contain an AudioBuffer after setter";

    ASSERT_TRUE( buffer == audioEvent->getBuffer() )
        << "expected AudioEvent to return set buffer";

    delete audioEvent;
}

TEST( BaseAudioEvent, MixBuffer )
{
    BaseAudioEvent* audioEvent = new BaseAudioEvent();

    int eventLength = randomInt( 8, 24 );
    int eventStart  = randomInt( 0, ( int )( eventLength / 2 ));

    audioEvent->setEventStart ( eventStart );
    audioEvent->setEventLength( eventLength );

    int eventEnd = audioEvent->getEventEnd();

    AudioBuffer* buffer = fillAudioBuffer( new AudioBuffer( randomInt( 1, 4 ), eventLength ));
    audioEvent->setBuffer( buffer, true );

    float volume = randomFloat();
    audioEvent->setVolume( volume );

    //std::cout << " ss: " << eventStart << " se: " << eventEnd << " sl: " << eventLength << " ch: " << buffer->amountOfChannels;

    // create a temporary buffer to write output in, ensure it is smaller than the event buffer
    AudioBuffer* targetBuffer = new AudioBuffer( buffer->amountOfChannels, randomInt( 2, 4 ));
    int buffersToWrite        = targetBuffer->bufferSize;

    ASSERT_FALSE( bufferHasContent( targetBuffer ))
        << "expected target buffer to be silent after creation, but it has content";

    // test 1. mix without loopable range

    int maxBufferPos = eventLength * 2; // use a "loop range" larger than the size of the events length
    int minBufferPos = randomInt( 0, maxBufferPos / 2 );
    int bufferPos    = randomInt( minBufferPos, maxBufferPos - 1 );
    bool loopStarted = false;
    int loopOffset   = 0;

    // if the random bufferPosition wasn't within the events eventStart and eventEnd range, we expect no content

    bool expectContent = ( bufferPos >= eventStart && bufferPos <= eventEnd ) ||
                         (( bufferPos + buffersToWrite ) >= eventStart && ( bufferPos + buffersToWrite ) <= eventEnd );

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

                if ( r >= eventStart && r <= eventEnd )
                {
                    r -= eventStart; // substract audioEvent start position
                    expectedSample = sourceBuffer[ r ] * volume;
                }
                else {
                    expectedSample = 0.0;
                }
                SAMPLE_TYPE event = buffer[ i ];

                EXPECT_EQ( expectedSample, event )
                    << "expected mixed event at " << i << " to be equal to the calculated expected event at read offset " << r;
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
    // amountOfLoopedWrites is the amount of events written in the loop
    // loopStartReadPointer describes at which position the events from the source audioEvent will be read when loop starts
    // loopStartReadPointerEnd describes the last position the events from the source audioEvent will be read for the amount of loop writes

    int loopStartIteratorPosition = maxBufferPos + 1;
    int loopStartWritePointer     = loopOffset;
    int loopStartReadPointer      = minBufferPos;
    int amountOfLoopedWrites      = ( bufferPos + buffersToWrite ) - loopStartIteratorPosition;
    int loopStartReadPointerEnd   = ( loopStartReadPointer + amountOfLoopedWrites ) - 1;

    expectContent = ( bufferPos >= eventStart && bufferPos <= eventEnd ) ||
                    (( bufferPos + buffersToWrite ) >= eventStart && ( bufferPos + buffersToWrite ) <= eventEnd ) ||
                    ( loopStartIteratorPosition > maxBufferPos && (
                        ( loopStartReadPointer >= eventStart && loopStartReadPointer <= eventEnd ) ||
                        ( loopStartReadPointerEnd >= eventStart && loopStartReadPointerEnd <= eventEnd )));

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

                if ( r >= eventStart && r <= eventEnd )
                {
                    r -= eventStart; // substract audioEvent start position
                    expectedSample = sourceBuffer[ r ] * volume;
                }
                SAMPLE_TYPE event = buffer[ i ];

                EXPECT_EQ( expectedSample, event )
                    << "expected mixed event at " << i << " to be equal to the calculated expected event at read "
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
    audioEvent->setEventLength( 16 * 4 ); // thus will loop 4 times
    audioEvent->positionEvent ( 0, 16, 0 );

    // create an output buffer at a size smaller than the source buffer length

    int outputSize = ( int )(( double ) sourceSize * .4 );
    AudioBuffer* targetBuffer = new AudioBuffer( sourceBuffer->amountOfChannels, outputSize );

    int minBufferPos = audioEvent->getEventStart();
    int bufferPos    = minBufferPos;
    int maxBufferPos = audioEvent->getEventEnd();

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
