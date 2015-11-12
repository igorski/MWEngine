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
    delete audioEvent;
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
    delete audioEvent;
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
