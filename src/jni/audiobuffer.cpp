#include "audiobuffer.h"
#include <algorithm>

AudioBuffer::AudioBuffer( int aAmountOfChannels, int aBufferSize )
{
    loopeable        = false;
    amountOfChannels = aAmountOfChannels;
    bufferSize       = aBufferSize;
    _buffers         = new std::vector<float*>();//[ amountOfChannels ];

    for ( int i = 0; i < amountOfChannels; ++i )
    {
        float* buffer = new float[ bufferSize ];

        for ( int j = 0; j < bufferSize; ++j )
            buffer[ j ] = 0.0;

        _buffers->push_back( buffer );
        //(*_buffers)[ i ] = buffer;
    }
}

AudioBuffer::~AudioBuffer()
{
    while ( !_buffers->empty())
    {
        delete _buffers->back(), _buffers->pop_back();
    }
}

/* public methods */

float* AudioBuffer::getBufferForChannel( int aChannelNum )
{
    return _buffers->at( aChannelNum );
}

int AudioBuffer::mergeBuffers( AudioBuffer* aBuffer, int aReadOffset, int aWriteOffset )
{
    int sourceLength   = aBuffer->bufferSize;
    int writeLength    = sourceLength;
    int writtenSamples = 0;

    // keep writes within the bounds of this buffer

    if (( aWriteOffset + writeLength ) > bufferSize )
        writeLength = bufferSize - aWriteOffset;

    for ( int i = 0; i < amountOfChannels; ++i )
    {
        float* srcBuffer    = aBuffer->getBufferForChannel( i );
        float* targetBuffer = getBufferForChannel( i );

        for ( int j = aWriteOffset, l = aWriteOffset + writeLength, r = aReadOffset; j < l; ++j, ++r )
        {
            if ( r >= sourceLength )
            {
                if ( loopeable )
                    r = 0;
                else
                    break;
            }
            targetBuffer[ j ] += srcBuffer[ r ];
            ++writtenSamples;
        }
    }
    // return the amount of samples written (per buffer)
    return writtenSamples;
}

/**
 * fills the buffers with silence
 * overriding their previous contents
 */
void AudioBuffer::silenceBuffers()
{
    for ( int i = 0; i < amountOfChannels; ++i )
    {
        float* buffer = getBufferForChannel( i );

        for ( int j = 0; j < bufferSize; ++j )
            buffer[ j ] = 0.0;
    }
}

/**
 * copy contents of the mono (first) buffer
 * onto the remaining buffers
 */
void AudioBuffer::applyMonoSource()
{
    float* monoBuffer = getBufferForChannel( 0 );

    for ( int i = 1; i < amountOfChannels; ++i )
    {
        float* targetBuffer = getBufferForChannel( i );
        memcpy( targetBuffer, monoBuffer, bufferSize * sizeof( float ));
    }
}

AudioBuffer* AudioBuffer::clone()
{
    AudioBuffer* output = new AudioBuffer( amountOfChannels, bufferSize );

    for ( int i = 0; i < amountOfChannels; ++i )
    {
        float* sourceBuffer = getBufferForChannel( i );
        float* targetBuffer = output->getBufferForChannel( i );

        memcpy( targetBuffer, sourceBuffer, bufferSize * sizeof( float ));
    }
    return output;
}
