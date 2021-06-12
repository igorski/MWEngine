#include "../resizable_audiobuffer.h"

TEST( ResizableAudioBuffer, Construction )
{
    int amountOfChannels     = randomInt( 1, 5 );
    int bufferSize           = randomInt( 64, 8192 );
    ResizableAudioBuffer* audioBuffer = new ResizableAudioBuffer( amountOfChannels, bufferSize );

    EXPECT_EQ( audioBuffer->bufferSize, bufferSize )
        << "expected:" << bufferSize << ", got:" << audioBuffer->bufferSize << " for buffer size";

    EXPECT_EQ( audioBuffer->amountOfChannels, amountOfChannels )
        << "expected:" << amountOfChannels << ", got:" << audioBuffer->amountOfChannels << " channels";

    delete audioBuffer;
}

TEST( ResizableAudioBuffer, ResizeReadWriteOffsets )
{
    // in test we verify whether adjusting the size after construction
    // updates the expected read/write behaviour for the new range (by using the silenceBuffers method)

    int orgSize = 10;
    ResizableAudioBuffer* audioBuffer = new ResizableAudioBuffer( 1, orgSize );

    // fill buffer with audio
    SAMPLE_TYPE* buffer = audioBuffer->getBufferForChannel( 0 );

    for ( int i = 0; i < orgSize; ++i ) {
        buffer[ i ] = 1.0;
    }

    // shrink buffer size
    audioBuffer->resize( 5 );
    // silence buffers
    audioBuffer->silenceBuffers();

    for ( int i = 0; i < orgSize; ++i ) {
        SAMPLE_TYPE sample = buffer[ i ];

        if ( i < 5 ) {
            EXPECT_EQ( buffer[ i ], 0.0 ) << "expected sample within shrunk range to have been silenced";
        } else {
            EXPECT_EQ( buffer[ i ], 1.0 ) << "expected sample outside of shrunk range to remain unaffected";
        }
    }
    delete audioBuffer;
}
