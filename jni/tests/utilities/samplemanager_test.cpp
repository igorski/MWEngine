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

    SampleManager::setSample( id, buffer );

    ASSERT_TRUE( SampleManager::hasSample( id ))
        << "expected Sample to be found as it has been registered";

    SampleManager::removeSample( id, true );

    // buffer deleted by SampleManager.removeSample()
}

TEST( SampleManager, GetSample )
{
    std::string id = "foo";
    AudioBuffer* buffer = new AudioBuffer( 1, 10 );

    SampleManager::setSample( id, buffer );

    ASSERT_TRUE( SampleManager::getSample( id ) == buffer)
        << "expected registered AudioBuffer to have been returned";

    SampleManager::removeSample( id, true );

    // buffer deleted by SampleManager.removeSample()
}

TEST( SampleManager, RemoveSample )
{
    std::string id = "foo";
    AudioBuffer* buffer = new AudioBuffer( 1, 10 );

    SampleManager::setSample( id, buffer );

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

    SampleManager::setSample( id, buffer );

    ASSERT_TRUE( SampleManager::hasSample( id ))
        << "expected Sample to be found as it has been registered";

    SampleManager::removeSample( id, false );

    ASSERT_FALSE( SampleManager::hasSample( id ))
        << "expected no Sample to be found as it has been removed";

    ASSERT_FALSE( buffer == 0 ) << "expected AudioBuffer not to have been deleted";

    delete buffer;
}

TEST( SampleManager, GetSampleLength )
{
    std::string id = "foo";
    AudioBuffer* buffer = new AudioBuffer( 1, 10 );

    SampleManager::setSample( id, buffer );

    EXPECT_EQ( buffer->bufferSize, SampleManager::getSampleLength( id ))
        << "expected SampleManager to return the correct bufferSize for the registered  sample";

    SampleManager::removeSample( id, true );

    // buffer deleted by SampleManager.removeSample()
}

TEST( SampleManager, Flush )
{
    AudioBuffer* buffer1 = new AudioBuffer( 1, 10 );
    AudioBuffer* buffer2 = new AudioBuffer( 1, 10 );

    SampleManager::setSample( "foo", buffer1 );
    SampleManager::setSample( "bar", buffer2 );

    SampleManager::flushSamples();

    ASSERT_FALSE( SampleManager::hasSample( "foo" ))
        << "expected no Sample to be found as they should have been flushed";

    ASSERT_FALSE( SampleManager::hasSample( "bar" ))
        << "expected no Sample to be found as they should have been flushed";

    // buffers deleted by SampleManager.flushSamples()
}
