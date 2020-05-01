#include <channelgroup.h>
#include <audioengine.h>
#include <audiochannel.h>
#include <global.h>
#include <utilities/volumeutil.h>

TEST( ChannelGroup, Construction )
{
    ChannelGroup* channelGroup = new ChannelGroup();

    EXPECT_TRUE( channelGroup->getProcessingChain() != nullptr ) << "expected processingchain upon construction";
    EXPECT_EQ( channelGroup->getVolume(), 1.F ) << "expected volume to be set at the maximum level when unspecified in the constructor";

    delete channelGroup;

    float volume = ( float ) randomSample( 0, 1.F );
    channelGroup = new ChannelGroup( volume );

    EXPECT_EQ( channelGroup->getVolume(), volume ) << "expected volume to be equal to the value specified in the constructor";

    delete channelGroup;
}

TEST( ChannelGroup, Destruction )
{
    ChannelGroup* channelGroup = new ChannelGroup();

    auto it = std::find( AudioEngine::groups.begin(), AudioEngine::groups.end(), channelGroup );
    ASSERT_TRUE( it == AudioEngine::groups.end() ) << "expected channel groups not to be registered at start of test";

    AudioEngine::addChannelGroup( channelGroup );

    it = std::find( AudioEngine::groups.begin(), AudioEngine::groups.end(), channelGroup );
    ASSERT_FALSE( it == AudioEngine::groups.end() ) << "expected channel groups to be registered in engine";

    delete channelGroup;

    it = std::find( AudioEngine::groups.begin(), AudioEngine::groups.end(), channelGroup );
    ASSERT_TRUE( it == AudioEngine::groups.end() ) << "expected destructor to have unregistered channel group from engine";
}

TEST( ChannelGroup, AdditionAndRemovalOfChannels )
{
    AudioChannel* channel1 = new AudioChannel( 1.F );
    AudioChannel* channel2 = new AudioChannel( 1.F );

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

TEST( ChannelGroup, CustomVolume )
{
    float volume    = ( float ) randomSample( 0, 1.0 );
    float logVolume = VolumeUtil::toLog( volume );

    ChannelGroup* channelGroup = new ChannelGroup( volume );

    EXPECT_EQ( channelGroup->getVolume(), volume )
    << "expected the non-logarithmic volume to equal the input volume";

    EXPECT_EQ( channelGroup->getVolumeLogarithmic(), logVolume )
    << "expected the logarithmically scaled value to equal the logarithmically scaled volume";

    delete channelGroup;
}