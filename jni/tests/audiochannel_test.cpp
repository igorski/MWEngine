#include "../audiochannel.h"
#include "../global.h"

TEST( AudioChannel, Construction )
{
    float mixVolume = ( float ) randomSample( 0, MAX_PHASE );
    AudioChannel* audioChannel = new AudioChannel( mixVolume );

    EXPECT_EQ( audioChannel->mixVolume, mixVolume )
        << "expected:" << mixVolume << ", got:" << audioChannel->mixVolume << " for mix volume";

    delete audioChannel;

    int maxBufferPosition = randomInt( 0, 8192 );

    audioChannel = new AudioChannel( mixVolume, maxBufferPosition );

    EXPECT_EQ( audioChannel->mixVolume, mixVolume )
            << "expected:" << mixVolume << ", got:" << audioChannel->mixVolume << " for mix volume";

    EXPECT_EQ( audioChannel->maxBufferPosition, maxBufferPosition )
            << "expected:" << maxBufferPosition << ", got:" << audioChannel->maxBufferPosition << " for max buffer position";

    delete audioChannel;
}

TEST( AudioChannel, Events )
{
    AudioChannel* audioChannel = new AudioChannel( ( float ) randomSample( 0, MAX_PHASE ));

    EXPECT_EQ( audioChannel->audioEvents.size(), 0 )
        << "expected AudioChannel events vector to be empty upon construction";

    BaseAudioEvent* audioEvent = randomAudioEvent();
    audioChannel->addEvent( audioEvent );

    EXPECT_EQ( audioChannel->audioEvents.size(), 1 )
        << "expected AudioChannel events vector to hold 1 event after addition";

    audioChannel->reset();

    EXPECT_EQ( audioChannel->audioEvents.size(), 0 )
        << "expected AudioChannel events vector to be empty after reset";

    delete audioChannel;
    deleteAudioEvent( audioEvent );
}

TEST( AudioChannel, LiveEvents )
{
    AudioChannel* audioChannel = new AudioChannel( ( float ) randomSample( 0, MAX_PHASE ));

    ASSERT_FALSE( audioChannel->hasLiveEvents )
        << "expected AudioChannel to contain no live events upon construction";

    EXPECT_EQ( audioChannel->liveEvents.size(), 0 )
        << "expected AudioChannel liveEvents vector to be empty upon construction";

    BaseAudioEvent* audioEvent = randomAudioEvent();
    audioChannel->addLiveEvent( audioEvent );

    ASSERT_TRUE( audioChannel->hasLiveEvents )
        << "expected AudioChannel to contain live events after addition";

    EXPECT_EQ( audioChannel->liveEvents.size(), 1 )
        << "expected AudioChannel liveEvents vector to hold 1 event after addition";

    audioChannel->reset();

    ASSERT_FALSE( audioChannel->hasLiveEvents )
        << "expected AudioChannel to contain no live events after reset";

    EXPECT_EQ( audioChannel->liveEvents.size(), 0 )
        << "expected AudioChannel liveEvents vector to be empty after reset";

    delete audioChannel;
    deleteAudioEvent( audioEvent );
}

TEST( AudioChannel, OutputBuffer )
{
    AudioChannel* audioChannel = new AudioChannel( ( float ) randomSample( 0, MAX_PHASE ));
    AudioBuffer* outputBuffer  = audioChannel->getOutputBuffer();

    // ensure created buffer matches engine properties

    EXPECT_EQ( outputBuffer->bufferSize, AudioEngineProps::BUFFER_SIZE )
        << "expected:" << AudioEngineProps::BUFFER_SIZE << ", got:" << outputBuffer->bufferSize << " for output buffer size";

    EXPECT_EQ( outputBuffer->amountOfChannels, AudioEngineProps::OUTPUT_CHANNELS )
        << "expected:" << AudioEngineProps::OUTPUT_CHANNELS << ", got:" << outputBuffer->amountOfChannels << " for output channel amount";

    // update engine properties

    int oldBufferSize = AudioEngineProps::BUFFER_SIZE;

    while ( AudioEngineProps::BUFFER_SIZE == oldBufferSize )
        AudioEngineProps::BUFFER_SIZE = randomInt( 256, 8192 );

    // ensure engine properties have altered

    ASSERT_FALSE( AudioEngineProps::BUFFER_SIZE == oldBufferSize );

    // update output buffer (is invoked by AudioEngine during application lifetime)

    audioChannel->createOutputBuffer();

    // ensure created buffer matches new engine properties

    EXPECT_EQ( outputBuffer->bufferSize, AudioEngineProps::BUFFER_SIZE )
        << "expected:" << AudioEngineProps::BUFFER_SIZE << ", got:" << outputBuffer->bufferSize << " for output buffer size";

    EXPECT_EQ( outputBuffer->amountOfChannels, AudioEngineProps::OUTPUT_CHANNELS )
        << "expected:" << AudioEngineProps::OUTPUT_CHANNELS << ", got:" << outputBuffer->amountOfChannels << " for output channel amount";

    delete audioChannel;
}

TEST( AudioChannel, Caching )
{
    AudioChannel* audioChannel = new AudioChannel( ( float ) randomSample( 0, MAX_PHASE ));

    ASSERT_FALSE( audioChannel->canCache() )
        << "expected caching to be disabled by default on construction";

    int cacheBufferSize  = randomInt( 512, 8192 );
    int cacheStartOffset = randomInt( 0, cacheBufferSize / 2 );
    int cacheEndOffset   = randomInt( cacheStartOffset + 1, cacheBufferSize - 1 );

    audioChannel->canCache( true, cacheBufferSize, cacheStartOffset, cacheEndOffset );

    ASSERT_TRUE( audioChannel->canCache() )
        << "expected caching to be enabled after cache invocation";

    ASSERT_TRUE( audioChannel->isCaching )
        << "expected caching state to be true after cache invocation";

    ASSERT_FALSE( audioChannel->hasCache )
        << "expected caching complete state to be false after cache invocation";

    // create a randomly filled AudioBuffer to write into the cache

    AudioBuffer* audioBuffer = new AudioBuffer( AudioEngineProps::OUTPUT_CHANNELS, randomInt( 0, 512 ));
    fillAudioBuffer( audioBuffer );

    // write something into the cache until it is completely filled
    int writeIterations = 0;
    while ( !audioChannel->hasCache ) {
        audioChannel->writeCache( audioBuffer, 0 );
        ++writeIterations;
    }

    ASSERT_TRUE( audioChannel->hasCache )
        << "expected caching complete state to be true after filling of the entire cache buffer";

    ASSERT_FALSE( audioChannel->isCaching )
        << "expected caching state to be false after filling of the entire cache buffer";

    // ensure all has been written
    int readOffset = cacheStartOffset;

    for ( int i = 0; i < writeIterations; ++i )
    {
        // create temporary buffer to receive cache contents
        AudioBuffer* tempBuffer = new AudioBuffer( audioBuffer->amountOfChannels, audioBuffer->bufferSize );
        audioChannel->readCachedBuffer( tempBuffer, readOffset );

        // read one buffer size at a time as it has been written one buffer size at a time
        readOffset += audioBuffer->bufferSize;

        if ( readOffset < cacheEndOffset )
        {
            for ( int c = 0, ca = audioBuffer->amountOfChannels; c < ca; ++c )
            {
                SAMPLE_TYPE* srcBuffer   = tempBuffer->getBufferForChannel( c );
                SAMPLE_TYPE* cacheBuffer = tempBuffer->getBufferForChannel( c );

                for ( int j = 0, l = audioBuffer->bufferSize; j < l; ++j )
                    EXPECT_EQ( srcBuffer[ j ], cacheBuffer[ j ] ) << " expected cached contents to equal source contents";
            }
        }
        delete tempBuffer;
    }

    // check flushing of the cached buffer

    audioChannel->clearCachedBuffer();

    ASSERT_TRUE( audioChannel->canCache() )
        << "expected caching to be enabled after clearing of the cached buffer";

    ASSERT_TRUE( audioChannel->isCaching )
        << "expected caching state to be true after clearing of the cached buffer";

    ASSERT_FALSE( audioChannel->hasCache )
        << "expected caching complete state to be false after clearing of the cached buffer";

    delete audioChannel;
    delete audioBuffer;
}
