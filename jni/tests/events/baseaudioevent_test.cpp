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
    int sampleEnd    = sampleStart + sampleLength;

    audioEvent->setSampleStart ( sampleStart );
    audioEvent->setSampleEnd   ( sampleEnd );
    audioEvent->setSampleLength( sampleLength );

    EXPECT_EQ( sampleStart,  audioEvent->getSampleStart() );
    EXPECT_EQ( sampleEnd,    audioEvent->getSampleEnd() );
    EXPECT_EQ( sampleLength, audioEvent->getSampleLength() );

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

    int sampleLength = randomInt( 64, 256 );
    int sampleStart  = randomInt( 0, ( int )( sampleLength / 2 ));
    int sampleEnd    = sampleStart + sampleLength;

    audioEvent->setSampleStart ( sampleStart );
    audioEvent->setSampleEnd   ( sampleEnd );
    audioEvent->setSampleLength( sampleLength );

    AudioBuffer* buffer = fillAudioBuffer( new AudioBuffer( randomInt( 1, 4 ), sampleLength ));
    audioEvent->setBuffer( buffer, true );

    std::cout << " ss:" << sampleStart << " se:" << sampleEnd << " sl:" << sampleLength << "ch:" << buffer->amountOfChannels;

    // create a temporary buffer to write output in, ensure it is smaller than the event buffer
    AudioBuffer* tempBuffer = new AudioBuffer( buffer->amountOfChannels, randomInt( 24, 128 ));

    ASSERT_FALSE( bufferHasContent( tempBuffer ))
        << "expected temporary buffer to be silent after creation, but it has content";

    // test 1. mix without loopable range

    int maxBufferPos = sampleLength * 2; // use a "loop range" larger than the size of the events length
    int minBufferPos = randomInt( 0, maxBufferPos / 2 );
    int bufferPos    = randomInt( minBufferPos, maxBufferPos - 1 );
    bool loopStarted = false;
    int loopOffset   = 0;
    float volume     = audioEvent->getVolume();

    // if the random bufferPosition wasn't within the events sampleStart and sampleEnd range, we expect no content

    bool expectContent = ( bufferPos >= sampleStart && bufferPos < sampleEnd );

    std::cout << "expected content:" << expectContent;
    std::cout << "min: " << minBufferPos << "max: " << maxBufferPos << " cur:" << bufferPos;

    audioEvent->mixBuffer( tempBuffer, bufferPos, minBufferPos, maxBufferPos, loopStarted, loopOffset, false );

    if ( expectContent )
    {
        for ( int c = 0, ca = tempBuffer->amountOfChannels; c < ca; ++c )
        {
            SAMPLE_TYPE* buffer       = tempBuffer->getBufferForChannel( c );
            SAMPLE_TYPE* sourceBuffer = audioEvent->getBuffer()->getBufferForChannel( c );

            for ( int i = 0, r = bufferPos; i < tempBuffer->bufferSize; ++i, ++r )
            {
                if ( r >= maxBufferPos && !loopStarted )
                    r -= ( maxBufferPos - minBufferPos );

                SAMPLE_TYPE sample         = buffer[ i ];
                SAMPLE_TYPE expectedSample = sourceBuffer[ r ] * volume;

                EXPECT_EQ( expectedSample, sample )
                    << "expected mixed sample to be equal to the source sample multiplied by the event volume";
            }
        }

    }
    else {
        ASSERT_FALSE( bufferHasContent( tempBuffer ))
            << "expected temporary buffer to contain no content after mixing for an out-of-range buffer position";
    }

    // test 2. mixing within a loopable range (could imply sequencer is starting a loop)

    tempBuffer->silenceBuffers();

    ASSERT_FALSE( bufferHasContent( tempBuffer ))
        << "expected temporary buffer to be silent after silencing, but it has content";

    bufferPos     = randomInt( minBufferPos, maxBufferPos - 1 );
    expectContent = ( bufferPos >= sampleStart && bufferPos < sampleEnd );
    loopStarted   = true;
    loopOffset    = maxBufferPos - bufferPos;

    audioEvent->mixBuffer( tempBuffer, bufferPos, minBufferPos, maxBufferPos, loopStarted, loopOffset, false );

    if ( expectContent )
    {
        ASSERT_TRUE( bufferHasContent( tempBuffer ))
            << "expected temporary buffer to contain content after mixing, but it didn't";
    }
    else {
        ASSERT_FALSE( bufferHasContent( tempBuffer ))
            << "expected temporary buffer to contain no content after mixing for an out-of-range buffer position";
    }

    deleteAudioEvent( audioEvent );
    delete tempBuffer;
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
