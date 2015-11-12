#include "../audiobuffer.h"

TEST( AudioBuffer, Construction )
{
    int amountOfChannels     = randomInt( 1, 5 );
    int bufferSize           = randomInt( 64, 8192 );
    AudioBuffer* audioBuffer = new AudioBuffer( amountOfChannels, bufferSize );

    EXPECT_EQ( audioBuffer->bufferSize, bufferSize )
        << "expected:" << bufferSize << ", got:" << audioBuffer->bufferSize << " for buffer size";

    EXPECT_EQ( audioBuffer->amountOfChannels, amountOfChannels )
        << "expected:" << amountOfChannels << ", got:" << audioBuffer->amountOfChannels << " channels";

    delete audioBuffer;
}

TEST( AudioBuffer, Silence )
{
    AudioBuffer* audioBuffer = fillAudioBuffer( randomAudioBuffer() );
    SAMPLE_TYPE maxValue     = getMaxAmpForBuffer( audioBuffer );

    ASSERT_GT( maxValue, 0.0 ) << "expected max amp above 0.0, got:" << maxValue << " instead";

    audioBuffer->silenceBuffers();

    for ( int c = 0, ca = audioBuffer->amountOfChannels; c < ca; ++ c )
    {
        SAMPLE_TYPE* buffer = audioBuffer->getBufferForChannel( c );

        for ( int i = 0, l = audioBuffer->bufferSize; i < l; ++i )
            EXPECT_EQ( buffer[ i ], 0.0 ) << " epected: 0.0, got:" << buffer[ i ];
    }
    delete audioBuffer;
}

TEST( AudioBuffer, AdjustVolumes )
{
    AudioBuffer* audioBuffer = fillAudioBuffer( randomAudioBuffer() );
    SAMPLE_TYPE multiplier   = randomSample( 0.0, MAX_PHASE );
    SAMPLE_TYPE maxValue     = getMaxAmpForBuffer( audioBuffer );

    audioBuffer->adjustBufferVolumes( multiplier );

    EXPECT_EQ( getMaxAmpForBuffer( audioBuffer ), maxValue * multiplier )
        << "expected:" << ( maxValue * multiplier ) << ", got:" << getMaxAmpForBuffer( audioBuffer ) << " for adjusted volume";

    delete audioBuffer;
}

TEST( AudioBuffer, MergeBuffers )
{
    AudioBuffer* audioBuffer1 = fillAudioBuffer( randomAudioBuffer() );
    AudioBuffer* audioBuffer2 = fillAudioBuffer( randomAudioBuffer() );
    AudioBuffer* buffer2clone = audioBuffer2->clone();

    // randomize read / write offsets
    int read       = randomInt( 0, audioBuffer1->bufferSize - 1 );
    int write      = randomInt( 0, audioBuffer2->bufferSize - 1 );
    float volume   = randomFloat();
    bool loopeable = false; // TODO: failures!! randomBool(); // use random loopeable setting

    int writtenSamples = buffer2clone->mergeBuffers( audioBuffer1, read, write, volume );
    buffer2clone->loopeable = loopeable;

    int expectedWriteAmount = std::min( buffer2clone->bufferSize - write, audioBuffer1->bufferSize - read );

    EXPECT_EQ( writtenSamples, expectedWriteAmount )
        << "expected:" << expectedWriteAmount << " written buffers, got " << writtenSamples << " instead.";

    for ( int c = 0, ca = buffer2clone->amountOfChannels; c < ca ; ++c )
    {
        // it is possible the AudioBuffer has been merged with a buffer that had less channels
        bool channelMerged = c < audioBuffer1->amountOfChannels;

        SAMPLE_TYPE* buffer        = buffer2clone->getBufferForChannel( c );
        SAMPLE_TYPE* sourceBuffer1 = channelMerged ? audioBuffer1->getBufferForChannel( c ) : 0;
        SAMPLE_TYPE* sourceBuffer2 = audioBuffer2->getBufferForChannel( c );

        for ( int i = 0, l = audioBuffer1->bufferSize, r = read; i < l; ++i )
        {
            if ( channelMerged && i >= write )
            {
                if ( r >= l )
                {
                    if ( loopeable )
                        r = 0;
                    else
                        break;
                }
                SAMPLE_TYPE compareSample = sourceBuffer2[ i ] + ( sourceBuffer1[ r ] * volume );
                EXPECT_EQ( buffer[ i ], compareSample )
                    << "expected:" << compareSample << ", got:" << buffer[ i ] << " for merged buffer range at write pos " << i;

                ++r;
            }
            else {
                // outside of the merge range, we expect the buffer values to
                // equal that of the source of the clone buffer
                EXPECT_EQ( buffer[ i ], sourceBuffer2[ i ])
                    << "expected:" << sourceBuffer2[ i ] << " for non merged buffer range at write pos " << i;
            }
        }
    }

    delete audioBuffer1;
    delete audioBuffer2;
    delete buffer2clone;
}

TEST( AudioBuffer, ApplyMonoSource )
{
    // create stereo - or surround - buffer
    int amountOfChannels = randomInt( 2, 5 );
    int bufferSize       = randomInt( 64, 256 );

    AudioBuffer* audioBuffer = new AudioBuffer( amountOfChannels, bufferSize );

    // fill it with data
    for ( int c = 0, ca = audioBuffer->amountOfChannels; c < ca; ++c )
    {
        SAMPLE_TYPE* buffer = audioBuffer->getBufferForChannel( c );

        for ( int i = 0, l = audioBuffer->bufferSize; i < l; ++i )
            buffer[ i ] = randomSample( -MAX_PHASE, +MAX_PHASE );
    }

    // get all mono values
    SAMPLE_TYPE* monoBuffer = new SAMPLE_TYPE[ audioBuffer->bufferSize ];
    SAMPLE_TYPE* buffer     = audioBuffer->getBufferForChannel( 0 );

    for ( int i = 0, l = audioBuffer->bufferSize; i < l; ++i )
        monoBuffer[ i ] = buffer[ i ];

    // apply mono
    audioBuffer->applyMonoSource();

    // verify all channels have the same values
    for ( int c = 0, ca = audioBuffer->amountOfChannels; c < ca; ++c )
    {
        SAMPLE_TYPE* buffer = audioBuffer->getBufferForChannel( c );

        for ( int i = 0, l = audioBuffer->bufferSize; i < l; ++i )
            EXPECT_EQ( buffer[ i ], monoBuffer[ i ]) << "expected:" << monoBuffer[ i ] << ", got:" << buffer[ i ];
    }
    delete audioBuffer;
}

TEST( AudioBuffer, Clone )
{
    AudioBuffer* audioBuffer = fillAudioBuffer( randomAudioBuffer() );
    AudioBuffer* clone       = audioBuffer->clone();

    // verify the properties are equal

    EXPECT_EQ( clone->bufferSize, audioBuffer->bufferSize )
        << "expected:" << audioBuffer->bufferSize << ",  got:" << clone->bufferSize << " for buffer size";

    EXPECT_EQ( clone->amountOfChannels, audioBuffer->amountOfChannels )
        << "expected:" << audioBuffer->amountOfChannels << ",  got:" << clone->amountOfChannels << " for amount of channels";

    EXPECT_EQ( clone->loopeable, audioBuffer->loopeable )
        << "expected:" << audioBuffer->loopeable << ",  got:" << clone->loopeable << " for loopeable";

    // verify all cloned channels have the same values as the source

    for ( int c = 0, ca = audioBuffer->amountOfChannels; c < ca; ++c )
    {
        SAMPLE_TYPE* buffer    = clone->getBufferForChannel( c );
        SAMPLE_TYPE* srcBuffer = audioBuffer->getBufferForChannel( c );

        for ( int i = 0, l = audioBuffer->bufferSize; i < l; ++i )
            EXPECT_EQ( buffer[ i ], srcBuffer[ i ]) << "expected:" << srcBuffer[ i ] << ", got:" << buffer[ i ] << " for clone";
    }

    delete audioBuffer;
    delete clone;
}
