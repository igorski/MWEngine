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

TEST( AudioBuffer, IsSilent )
{
    AudioBuffer* audioBuffer = randomAudioBuffer();

    ASSERT_TRUE( audioBuffer->isSilent()) << "expected empty buffer to be recognized as silent";

    // fill buffer with content
    fillAudioBuffer( audioBuffer );
    ASSERT_FALSE( audioBuffer->isSilent()) << "expected filled buffer to not be recognized as silent";

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
            EXPECT_EQ( buffer[ i ], 0.0 ) << "expected silent sample";
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

TEST( AudioBuffer, MergeEqualLengthBuffers )
{
    AudioBuffer* bufferToMergeWidth = fillAudioBuffer( randomAudioBuffer() );
    int bufferSize            = bufferToMergeWidth->bufferSize;
    AudioBuffer* bufferSource = fillAudioBuffer( new AudioBuffer( bufferToMergeWidth->amountOfChannels, bufferSize ));
    AudioBuffer* mergedBuffer = bufferSource->clone(); // a clone from the buffer source

    // randomize read / write offsets so this test can cover a range of scenarios

    int read                = randomInt( 0, bufferToMergeWidth->bufferSize - 2 );
    int write               = randomInt( 0, bufferSource->bufferSize - 1 );
    float volume            = randomFloat();
    bufferToMergeWidth->loopeable = randomBool(); // use random loopeable setting;

    int writtenSamples = mergedBuffer->mergeBuffers( bufferToMergeWidth, read, write, volume );

    int expectedWriteAmount = mergedBuffer->bufferSize - read;

    // if the input buffer is loopeable, we expect to write as many samples are in the output buffer
    if ( bufferToMergeWidth->loopeable )
        expectedWriteAmount = mergedBuffer->bufferSize - write;

    if (( expectedWriteAmount + write ) >= bufferSize )
        expectedWriteAmount = bufferSize - write;

    EXPECT_EQ( writtenSamples, expectedWriteAmount )
        << "expected:" << expectedWriteAmount << " written buffers, got " << writtenSamples << " instead.";

    for ( int c = 0, ca = mergedBuffer->amountOfChannels; c < ca ; ++c )
    {
        // it is possible the AudioBuffer has been merged with a buffer that had less channels
        bool mergedChannel = c < bufferToMergeWidth->amountOfChannels;

        SAMPLE_TYPE* buffer        = mergedBuffer->getBufferForChannel( c );
        SAMPLE_TYPE* sourceBuffer1 = mergedChannel ? bufferToMergeWidth->getBufferForChannel( c ) : 0;
        SAMPLE_TYPE* sourceBuffer2 = bufferSource->getBufferForChannel( c );

        for ( int i = 0, l = bufferToMergeWidth->bufferSize, r = read; i < l; ++i )
        {
            if ( mergedChannel && i >= write )
            {
                if ( r >= l )
                {
                    if ( bufferToMergeWidth->loopeable  )
                        r = 0;
                    else
                        break;
                }
                SAMPLE_TYPE compareSample = sourceBuffer2[ i ] + ( sourceBuffer1[ r ] * volume );
                EXPECT_EQ( compareSample, buffer[ i ] )
                    << "expected:" << compareSample << ", got:" << buffer[ i ] << " for merged buffer range at write pos " << i;

                ++r;
            }
            else {
                // outside of the merge range, we expect the buffer values to
                // equal that of the source of the clone buffer
                EXPECT_EQ( sourceBuffer2[ i ], buffer[ i ] )
                    << "expected:" << sourceBuffer2[ i ] << " for non merged buffer range at write pos " << i;
            }
        }
    }

    delete bufferToMergeWidth;
    delete bufferSource;
    delete mergedBuffer;
}

TEST( AudioBuffer, MergeInequalLengthBuffers )
{
    // create two buffers

    int channelAmount = randomInt( 1, 5 );
    int buffer1size   = 128;
    int buffer2size   = buffer1size / 2;

    AudioBuffer* audioBuffer1 = new AudioBuffer( channelAmount, buffer1size );
    AudioBuffer* audioBuffer2 = fillAudioBuffer( new AudioBuffer( channelAmount, buffer2size ));

    // merge properties

    int   read              = 0;
    int   write             = 0;
    float volume            = randomFloat();
    int expectedWriteAmount = buffer2size;

    // TEST 1 : merging of a smaller buffer into a large buffer, no looping

    int writtenSamples = audioBuffer1->mergeBuffers( audioBuffer2, read, write, volume );

    EXPECT_EQ( writtenSamples, expectedWriteAmount )
        << "expected:" << expectedWriteAmount << " written buffers, got " << writtenSamples << " instead.";

    for ( int c = 0, ca = audioBuffer1->amountOfChannels; c < ca ; ++c )
    {
        SAMPLE_TYPE* buffer       = audioBuffer1->getBufferForChannel( c );
        SAMPLE_TYPE* sourceBuffer = audioBuffer2->getBufferForChannel( c );

        for ( int i = 0, l = buffer1size; i < l; ++i )
        {
            if ( i >= buffer2size )
            {
                // expect no samples as the iterator has extended the merge range
                EXPECT_EQ( 0.0, buffer[ i ] )
                    << "expected no sample, got:" << buffer[ i ] << " for merged buffer range at write pos " << i;
            }
            else {
                SAMPLE_TYPE compareSample = sourceBuffer[ i ] * volume;
                EXPECT_EQ( compareSample, buffer[ i ] )
                    << "expected " << compareSample << ", got:" << buffer[ i ] << " for merged buffer range at write pos " << i;
            }
        }
    }

    // TEST 2 : merging of a smaller buffer into a large buffer but with looping

    audioBuffer1->silenceBuffers();
    audioBuffer2->loopeable = true;
    expectedWriteAmount     = buffer1size;
    writtenSamples = audioBuffer1->mergeBuffers( audioBuffer2, read, write, volume );

    EXPECT_EQ( writtenSamples, expectedWriteAmount )
        << "expected:" << expectedWriteAmount << " written buffers, got " << writtenSamples << " instead.";

    for ( int c = 0, ca = audioBuffer1->amountOfChannels; c < ca ; ++c )
    {
        SAMPLE_TYPE* buffer       = audioBuffer1->getBufferForChannel( c );
        SAMPLE_TYPE* sourceBuffer = audioBuffer2->getBufferForChannel( c );

        for ( int i = 0, loopOffset = 0, l = buffer1size; i < l; ++i )
        {
            if ( i >= buffer2size )
            {
                // when reading beyond the size of the source buffer, we expect
                // the next sample to be read from the beginning of the source buffer
                SAMPLE_TYPE compareSample = sourceBuffer[ loopOffset ] * volume;
                EXPECT_EQ( compareSample, buffer[ i ] )
                    << "expected " << compareSample << ", got:" << buffer[ i ] << " for merged buffer range "
                    << "at write pos " << i << " and read pos " << loopOffset << " after expecting looped writing";

                ++loopOffset;
            }
            else {
                SAMPLE_TYPE compareSample = sourceBuffer[ i ] * volume;
                EXPECT_EQ( compareSample, buffer[ i ] )
                    << "expected " << compareSample << ", got:" << buffer[ i ] << " for merged buffer range at write pos " << i;
            }
        }
    }
    delete audioBuffer1;
    delete audioBuffer2;
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
