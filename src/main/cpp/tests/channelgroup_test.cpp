#include "../channelgroup.h"
#include "../audiochannel.h"
#include "../global.h"

TEST( ChannelGroup, Construction )
{
    ChannelGroup* channelGroup = new ChannelGroup();

    EXPECT_TRUE( channelGroup->getProcessingChain() != nullptr ) << "expected processingchain upon construction";

    delete channelGroup;
}

TEST( ChannelGroup, AdditionAndRemovalOfChannels )
{
    AudioChannel* channel1 = new AudioChannel( 1.0F );
    AudioChannel* channel2 = new AudioChannel( 1.0F );

    ChannelGroup* channelGroup = new ChannelGroup();

    EXPECT_FALSE( channelGroup->containsAudioChannel( channel1 )) << "expected false as channel was not added to group";
    EXPECT_FALSE( channelGroup->containsAudioChannel( channel2 )) << "expected false as channel was not added to group";

    channelGroup->addAudioChannel( channel1 );

    EXPECT_TRUE( channelGroup->containsAudioChannel( channel1 )) << "expected true as channel was added to group";
    EXPECT_FALSE( channelGroup->containsAudioChannel( channel2 )) << "expected false as channel was not added to group";

    channelGroup->addAudioChannel( channel2 );

    EXPECT_TRUE( channelGroup->containsAudioChannel( channel1 )) << "expected true as channel was added to group";
    EXPECT_TRUE( channelGroup->containsAudioChannel( channel2 )) << "expected true as channel was added to group";

    channelGroup->removeAudioChannel( channel1 );

    EXPECT_FALSE( channelGroup->containsAudioChannel( channel1 )) << "expected false as channel was removed from group";
    EXPECT_TRUE( channelGroup->containsAudioChannel( channel2 )) << "expected true as channel was not removed from group";

    channelGroup->removeAudioChannel( channel2 );

    EXPECT_FALSE( channelGroup->containsAudioChannel( channel1 )) << "expected false as channel was removed from group";
    EXPECT_FALSE( channelGroup->containsAudioChannel( channel2 )) << "expected false as channel was removed from group";

    delete channelGroup;
    delete channel1;
    delete channel2;
}