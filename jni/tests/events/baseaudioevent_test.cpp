#include "../../events/baseaudioevent.h"
#include "../../instruments/baseinstrument.h"

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
    int sampleEnd    = sampleStart + sampleLength;

    audioEvent->setSampleStart ( sampleStart );
    audioEvent->setSampleLength( sampleLength );

    AudioBuffer* buffer = fillAudioBuffer( new AudioBuffer( randomInt( 1, 4 ), sampleLength ));
    audioEvent->setBuffer( buffer, true );

    float volume = randomFloat();
    audioEvent->setVolume( volume );

    std::cout << " ss: " << sampleStart << " se: " << sampleEnd << " sl: " << sampleLength << " ch: " << buffer->amountOfChannels;

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
                    << "expected mixed sample at " << i << " to be equal the calculated expected sample at read offset " << r;
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
    loopOffset    = maxBufferPos - bufferPos;

    // pre calculate at which buffer iterator the looping will commence
    // loopStartIteratorPosition describes at which sequencer position the loop starts
    // loopStartWritePointer describes at which position in the targetBuffer the loop is written to
    // loopStartReadPointer describes at which position the samples from the source audioEvent will be read when loop starts

    int loopStartIteratorPosition = bufferPos + buffersToWrite;
    int loopStartWritePointer     = ( loopStartIteratorPosition - maxBufferPos ) + 1;
    int loopStartReadPointer      = minBufferPos + ( loopStartWritePointer - loopOffset );

    expectContent = ( bufferPos >= sampleStart && bufferPos <= sampleEnd ) ||
                    (( bufferPos + buffersToWrite ) >= sampleStart && ( bufferPos + buffersToWrite ) <= sampleEnd ) ||
                    ( loopStartIteratorPosition > maxBufferPos && ( loopStartReadPointer >= sampleStart && loopStartReadPointer <= sampleEnd ));

    audioEvent->mixBuffer( targetBuffer, bufferPos, minBufferPos, maxBufferPos, loopStarted, loopOffset, false );

    std::cout << " expected content: " << expectContent << " for buffer size: " << buffersToWrite;
    std::cout << " min: " << minBufferPos << " max: " << maxBufferPos << " cur: " << bufferPos << " loop offset: " << loopOffset;

    std::cout << "\nSOURCE\n";
    std::cout << "\ndumping AudioBuffer with " << audioEvent->getBuffer()->amountOfChannels << " channels of "
            << audioEvent->getBuffer()->bufferSize << " samples in size\n";

        for ( int c = 0; c < audioEvent->getBuffer()->amountOfChannels; ++c )
        {
            std::cout << "---------\n";
            std::cout << "CHANNEL " << c << ":\n";
            std::cout << "---------\n";

            SAMPLE_TYPE* buffer = audioEvent->getBuffer()->getBufferForChannel( c );

            for ( int i = 0; i < audioEvent->getBuffer()->bufferSize; ++i )
                std::cout << i << ":" << ( buffer[ i ] * volume ) << "\n";
        }
    std::cout << "\nTARGET\n";
    dumpBufferContents( targetBuffer );

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

                if ( i >= loopOffset ) {
                std::cout << "i:" << i << " larger than:" << loopOffset << "\n";
                    r = minBufferPos + ( i - loopOffset );
                    }

                if ( r >= sampleStart && r <= sampleEnd )
                {
                    r -= sampleStart; // substract audioEvent start position
                    std::cout << "r:!" << r << "!!! for iterator:" << i << "\n";
                    expectedSample = sourceBuffer[ r ] * volume;
                }
                SAMPLE_TYPE sample = buffer[ i ];

                EXPECT_EQ( expectedSample, sample )
                    << "expected mixed sample at " << i << " to be equal the calculated expected sample at read offset " << r;
            }
        }
    }
    else {
        ASSERT_FALSE( bufferHasContent( targetBuffer ))
            << "expected output buffer to contain no content after mixing for an out-of-range buffer position";
    }
    deleteAudioEvent( audioEvent );
    delete targetBuffer;
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

    deleteAudioEvent( audioEvent );
}
