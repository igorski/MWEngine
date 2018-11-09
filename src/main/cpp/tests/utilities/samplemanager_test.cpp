#include "../../utilities/samplemanager.h"
#include "../../audiobuffer.h"

TEST( SampleManager, EmptyByDefault )
{
    std::string id = "foo";

    ASSERT_FALSE( SampleManager::hasSample( id ))
        << "expected no Sample to be found as it hasn't been registered yet";
}

TEST( SampleManager, SetSample )
{
    std::string id = "foo";
    AudioBuffer* buffer = new AudioBuffer( 1, 10 );

    SampleManager::setSample( id, buffer, AudioEngineProps::SAMPLE_RATE );

    ASSERT_TRUE( SampleManager::hasSample( id ))
        << "expected Sample to be found as it has been registered";

    SampleManager::removeSample( id, true );

    // buffer deleted by SampleManager.removeSample()
}

TEST( SampleManager, GetSample )
{
    std::string id = "foo";
    AudioBuffer* buffer = new AudioBuffer( 1, 10 );

    SampleManager::setSample( id, buffer, AudioEngineProps::SAMPLE_RATE );

    ASSERT_TRUE( SampleManager::getSample( id ) == buffer)
        << "expected registered AudioBuffer to have been returned";

    SampleManager::removeSample( id, true );

    // buffer deleted by SampleManager.removeSample()
}

TEST( SampleManager, RemoveSample )
{
    std::string id = "foo";
    AudioBuffer* buffer = new AudioBuffer( 1, 10 );

    SampleManager::setSample( id, buffer, AudioEngineProps::SAMPLE_RATE );

    ASSERT_TRUE( SampleManager::hasSample( id ))
        << "expected Sample to be found as it has been registered";

    SampleManager::removeSample( id, true );

    ASSERT_FALSE( SampleManager::hasSample( id ))
        << "expected no Sample to be found as it has been removed";

    // buffer deleted by SampleManager.removeSample()
}

TEST( SampleManager, RemoveSampleWithoutFree )
{
    std::string id = "foo";
    AudioBuffer* buffer = new AudioBuffer( 1, 10 );

    SampleManager::setSample( id, buffer, AudioEngineProps::SAMPLE_RATE );

    ASSERT_TRUE( SampleManager::hasSample( id ))
        << "expected Sample to be found as it has been registered";

    SampleManager::removeSample( id, false );

    ASSERT_FALSE( SampleManager::hasSample( id ))
        << "expected no Sample to be found as it has been removed";

    ASSERT_FALSE( buffer == nullptr ) << "expected AudioBuffer not to have been deleted";

    delete buffer;
}

TEST( SampleManager, GetSampleLength )
{
    std::string id = "foo";
    AudioBuffer* buffer = new AudioBuffer( 1, 10 );

    SampleManager::setSample( id, buffer, AudioEngineProps::SAMPLE_RATE );

    EXPECT_EQ( buffer->bufferSize, SampleManager::getSampleLength( id ))
        << "expected SampleManager to return the correct bufferSize for the registered sample";

    SampleManager::removeSample( id, true );

    // buffer deleted by SampleManager.removeSample()
}

TEST( SampleManager, GetSampleRateForSample )
{
    std::string id = "foo";
    AudioBuffer* buffer = new AudioBuffer( 1, 10 );

    // ensure sample rate doesn't match engine
    int sampleRate = AudioEngineProps::SAMPLE_RATE / 2;

    SampleManager::setSample( id, buffer, sampleRate );

    EXPECT_EQ( sampleRate, SampleManager::getSampleRateForSample( id ))
        << "expected SampleManager to return the correct sample rate for the registered sample";

    SampleManager::removeSample( id, true );

    // buffer deleted by SampleManager.removeSample()
}

TEST( SampleManager, Flush )
{
    AudioBuffer* buffer1 = new AudioBuffer( 1, 10 );
    AudioBuffer* buffer2 = new AudioBuffer( 1, 10 );

    SampleManager::setSample( "foo", buffer1, AudioEngineProps::SAMPLE_RATE );
    SampleManager::setSample( "bar", buffer2, AudioEngineProps::SAMPLE_RATE );

    SampleManager::flushSamples();

    ASSERT_FALSE( SampleManager::hasSample( "foo" ))
        << "expected no Sample to be found as they should have been flushed";

    ASSERT_FALSE( SampleManager::hasSample( "bar" ))
        << "expected no Sample to be found as they should have been flushed";

    // buffers deleted by SampleManager.flushSamples()
}
