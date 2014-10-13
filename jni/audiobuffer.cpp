/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2014 Igor Zinken - http://www.igorski.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "audiobuffer.h"
#include <algorithm>

AudioBuffer::AudioBuffer( int aAmountOfChannels, int aBufferSize )
{
    loopeable        = false;
    amountOfChannels = aAmountOfChannels;
    bufferSize       = aBufferSize;
    _buffers         = new std::vector<SAMPLE_TYPE*>();//[ amountOfChannels ];

    for ( int i = 0; i < amountOfChannels; ++i )
    {
        SAMPLE_TYPE* buffer = new SAMPLE_TYPE[ bufferSize ];

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

SAMPLE_TYPE* AudioBuffer::getBufferForChannel( int aChannelNum )
{
    return _buffers->at( aChannelNum );
}

int AudioBuffer::mergeBuffers( AudioBuffer* aBuffer, int aReadOffset, int aWriteOffset, float aMixVolume )
{
    if ( aBuffer == 0 ) return 0;

    int sourceLength   = aBuffer->bufferSize;
    int writeLength    = sourceLength;
    int writtenSamples = 0;

    // keep writes within the bounds of this buffer

    if (( aWriteOffset + writeLength ) > bufferSize )
        writeLength = bufferSize - aWriteOffset;

    for ( int i = 0; i < amountOfChannels; ++i )
    {
        SAMPLE_TYPE* srcBuffer    = aBuffer->getBufferForChannel( i );
        SAMPLE_TYPE* targetBuffer = getBufferForChannel( i );

        for ( int j = aWriteOffset, l = aWriteOffset + writeLength, r = aReadOffset; j < l; ++j, ++r )
        {
            if ( r >= sourceLength )
            {
                if ( loopeable )
                    r = 0;
                else
                    break;
            }
            targetBuffer[ j ] += ( srcBuffer[ r ] * aMixVolume );
            ++writtenSamples;
        }
    }
    // return the amount of samples written (per buffer)
    return writtenSamples;
}

/**
 * fills the buffers with silence
 * clearing their previous contents
 */
void AudioBuffer::silenceBuffers()
{
    for ( int i = 0; i < amountOfChannels; ++i )
    {
        SAMPLE_TYPE* buffer = getBufferForChannel( i );

        for ( int j = 0; j < bufferSize; ++j )
            buffer[ j ] = 0.0;
    }
}

void AudioBuffer::adjustBufferVolumes( SAMPLE_TYPE amp )
{
    for ( int i = 0; i < amountOfChannels; ++i )
    {
        SAMPLE_TYPE* buffer = getBufferForChannel( i );

        for ( int j = 0; j < bufferSize; ++j )
            buffer[ j ] *= amp;
    }
}

/**
 * copy contents of the mono (first) buffer
 * onto the remaining buffers
 */
void AudioBuffer::applyMonoSource()
{
    if ( amountOfChannels == 1 )
        return;

    SAMPLE_TYPE* monoBuffer = getBufferForChannel( 0 );

    for ( int i = 1; i < amountOfChannels; ++i )
    {
        SAMPLE_TYPE* targetBuffer = getBufferForChannel( i );
        memcpy( targetBuffer, monoBuffer, bufferSize * sizeof( SAMPLE_TYPE ));
    }
}

AudioBuffer* AudioBuffer::clone()
{
    AudioBuffer* output = new AudioBuffer( amountOfChannels, bufferSize );

    for ( int i = 0; i < amountOfChannels; ++i )
    {
        SAMPLE_TYPE* sourceBuffer = getBufferForChannel( i );
        SAMPLE_TYPE* targetBuffer = output->getBufferForChannel( i );

        memcpy( targetBuffer, sourceBuffer, bufferSize * sizeof( SAMPLE_TYPE ));
    }
    return output;
}
