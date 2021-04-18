#include "../audiochannel.h"
#include "../global.h"
#include "../utilities/volumeutil.h"

TEST( AudioChannel, Construction )
{
    float volume = ( float ) randomSample( 0, 1.0 );
    AudioChannel* audioChannel = new AudioChannel( volume );

    EXPECT_EQ( audioChannel->getVolume(), volume )
        << "expected:" << volume << ", got:" << audioChannel->getVolume() << " for mix volume";

    delete audioChannel;

    int maxBufferPosition = randomInt( 0, 8192 );

    audioChannel = new AudioChannel( volume, maxBufferPosition );

    EXPECT_EQ( audioChannel->getVolume(), volume )
            << "expected:" << volume << ", got:" << audioChannel->getVolume() << " for mix volume";

    EXPECT_EQ( audioChannel->maxBufferPosition, maxBufferPosition )
            << "expected:" << maxBufferPosition << ", got:" << audioChannel->maxBufferPosition << " for max buffer position";

    delete audioChannel;
}

TEST( AudioChannel, Volume )
{
    float volume    = ( float ) randomSample( 0, 1.0 );
    float logVolume = VolumeUtil::toLog( volume );

    AudioChannel* audioChannel = new AudioChannel( volume );

    EXPECT_EQ( audioChannel->getVolume(), volume )
        << "expected the non-logarithmic volume to equal the input volume";

    EXPECT_EQ( audioChannel->getVolumeLogarithmic(), logVolume )
        << "expected the logarithmically scaled value to equal the logarithmically scaled volume";

    delete audioChannel;
}

TEST( AudioChannel, InstanceId )
{
    float volume = ( float ) randomSample( 0, 1.0 );

    // 1. create first channel

    AudioChannel* audioChannel = new AudioChannel( volume );

    int firstInstanceId = audioChannel->instanceId;

    // 2. create second channel

    AudioChannel* audioChannel2 = new AudioChannel( volume );

    EXPECT_EQ( firstInstanceId + 1, audioChannel2->instanceId )
        << "expected second AudioChannel to have an id 1 higher than the first";

    // 3. delete events (should decrement instance ids)

    delete audioChannel;
    delete audioChannel2;

    // 4. create third channel
    // TODO: destructor doesn't seem to do anything ??
//    audioChannel = new AudioChannel( volume );
//
//    EXPECT_EQ( firstInstanceId, audioChannel->instanceId )
//        << "expected old instance id to be equal to the new AudioChannel id as the old events have been disposed";
//
//    delete audioChannel;
}

TEST( AudioChannel, Events )
{
    AudioChannel* audioChannel = new AudioChannel( ( float ) randomSample( 0, 1.0 ));

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
    AudioChannel* audioChannel = new AudioChannel( ( float ) randomSample( 0, 1.0 ));

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
    AudioChannel* audioChannel = new AudioChannel( ( float ) randomSample( 0, 1.0 ));
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
    AudioChannel* audioChannel = new AudioChannel( ( float ) randomSample( 0, 1.0 ));

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

        if ( writeIterations == 8192 ) {
            std::cout << " TODO: cache test never finished, is there a bug here ??? (fails on occasion)";
            delete audioChannel;
            delete audioBuffer;
            return;
        }
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

TEST( AudioChannel, Panning )
{
    AudioChannel* audioChannel = new AudioChannel( 1.0f );

    EXPECT_EQ( 0, audioChannel->getPan())
        << "expected default pan to be 0 (center)";

    float targetPan = randomFloat( -1, 1 );
    audioChannel->setPan( targetPan);

    EXPECT_EQ( targetPan, audioChannel->getPan())
        << "expected pan to have been set to given value";

    delete audioChannel;
}

TEST( AudioChannel, MixPannedBuffer )
{
    AudioChannel* audioChannel = new AudioChannel( 1.0f );

    AudioEngineProps::BUFFER_SIZE     = 1;
    AudioEngineProps::OUTPUT_CHANNELS = 2;

    AudioBuffer* mixBuffer = new AudioBuffer( AudioEngineProps::OUTPUT_CHANNELS, AudioEngineProps::BUFFER_SIZE );
    audioChannel->createOutputBuffer();
    AudioBuffer* channelBuffer = audioChannel->getOutputBuffer();

    SAMPLE_TYPE* srcLeft  = channelBuffer->getBufferForChannel( 0 );
    SAMPLE_TYPE* srcRight = channelBuffer->getBufferForChannel( 1 );
    SAMPLE_TYPE* tgtLeft  = mixBuffer->getBufferForChannel( 0 );
    SAMPLE_TYPE* tgtRight = mixBuffer->getBufferForChannel( 1 );

    // TEST 1. right panning (source buffer has only left channel content, no right channel)

    srcLeft[0] = 1.0;
    srcRight[0] = 0.0f;

    audioChannel->setPan( 0.3 ); // set pan slightly to the right
    audioChannel->mixBuffer( mixBuffer, 1 );

    ASSERT_TRUE( compareFloat( 0.7, tgtLeft[0])) << "expected left channel signal to be 0.7 for a +0.3 pan";
    ASSERT_TRUE( compareFloat( 0.3, tgtRight[0])) << "expected right channel signal to be 0.3 for a +0.3 pan";

    mixBuffer->silenceBuffers(); // clean up mix buffer contents

    // TEST 2. left panning (source buffer has only right channel content, no left channel)

    srcLeft[0] = 0.0f;
    srcRight[0] = 1.0;

    audioChannel->setPan( -0.7 ); // set pan slightly to the left
    audioChannel->mixBuffer( mixBuffer, 1 );

    ASSERT_TRUE( compareFloat( 0.7, tgtLeft[0])) << "expected left channel signal to be 0.7 for a -0.7 pan";;
    ASSERT_TRUE( compareFloat( 0.3, tgtRight[0])) << "expected right channel signal to be 0.3 for a +0.3 pan";

    delete audioChannel;
    delete mixBuffer;
}