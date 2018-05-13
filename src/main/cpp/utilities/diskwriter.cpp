/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2016 Igor Zinken - http://www.igorski.nl
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
#include "diskwriter.h"
#include "audioengine.h"
#include "wavewriter.h"
#include "utils.h"
#include <definitions/notifications.h>
#include <messaging/notifier.h>

namespace DiskWriter
{
    std::string outputDirectory;
    unsigned long outputBufferSize  = 0;
    unsigned long outputWriterIndex = 0;
    AudioBuffer*  cachedBuffer      = 0;

    /**
     * prepare a new iteration of recording
     */
    void prepare( std::string aOutputDir, int aBufferSize, int amountOfChannels )
    {
        outputDirectory  = aOutputDir;
        outputBufferSize = aBufferSize;

        generateOutputBuffer( amountOfChannels );
    }

    /**
     * allocates a new buffer for the next write iterations
     */
    void generateOutputBuffer( int amountOfChannels )
    {
        flushOutput(); // free previous contents
        cachedBuffer = new AudioBuffer( amountOfChannels, outputBufferSize );

        outputWriterIndex = 0;
    }

    /**
     * append an AudioBuffer into the write buffer
     */
    void appendBuffer( AudioBuffer* aBuffer )
    {
        int bufferSize    = aBuffer->bufferSize;
        int channelAmount = aBuffer->amountOfChannels;

        if ( cachedBuffer == 0 )
            generateOutputBuffer( channelAmount );

        cachedBuffer->mergeBuffers( aBuffer, 0, outputWriterIndex, MAX_PHASE );
        outputWriterIndex += bufferSize;
    }

    /**
     * append the actual output buffer from the engine
     * into the write buffer
     */
    void appendBuffer( float* aBuffer, int aBufferSize, int amountOfChannels )
    {
        if ( cachedBuffer == 0 )
            generateOutputBuffer( amountOfChannels );

        int i, c, ci;

        // write samples into cache buffers

        for ( i = 0, c = 0; i < aBufferSize; ++i, ++outputWriterIndex, c += amountOfChannels )
        {
            if ( outputWriterIndex == outputBufferSize )
                return;

            for ( ci = 0; ci < amountOfChannels; ++ci )
                cachedBuffer->getBufferForChannel( ci )[ outputWriterIndex ] = aBuffer[ c + ci ];
        }
    }

    /**
     * checks whether the current write buffer is full
     */
    bool bufferFull()
    {
        return outputWriterIndex >= outputBufferSize;
    }

    /**
     * flush the contents of the write buffer
     */
    void flushOutput()
    {
        if ( cachedBuffer != 0 )
            delete cachedBuffer;

        cachedBuffer      = 0;
        outputWriterIndex = 0;
    }

    /**
     * write the contents of the write buffer into
     * an output file, this will only write content
     * up until the point if was written to in case
     * the buffer wasn't full yet
     */
    void writeBufferToFile( int aSampleRate, int aNumChannels, bool broadcastUpdate )
    {
        // quick assertion
        if ( cachedBuffer == 0 )
            return;

        // copy string contents for appending of filename
        std::string outputFile = std::string( outputDirectory.c_str());

        int bufferSize = outputBufferSize;

        // recorded less than maximum available in buffer ? cut silence
        // by writing recording into temporary buffers

        if ( outputWriterIndex < bufferSize )
        {
            bufferSize = outputWriterIndex;

            AudioBuffer* tempBuffer = new AudioBuffer( aNumChannels, bufferSize );

            for ( int i = 0; i < bufferSize; ++i )
            {
                for ( int c = 0; c < aNumChannels; ++c )
                    tempBuffer->getBufferForChannel( c )[ i ] = cachedBuffer->getBufferForChannel( c )[ i ];
            }
            WaveWriter::bufferToFile( outputFile.append( SSTR( AudioEngine::recordingFileId )),
                                      tempBuffer, aSampleRate );

            // free memory allocated by temporary buffer

            delete tempBuffer;
        }
        else
        {
            WaveWriter::bufferToFile( outputFile.append( SSTR( AudioEngine::recordingFileId )),
                                      cachedBuffer, aSampleRate );
        }

        flushOutput(); // free memory

        // broadcast update, pass buffer identifier to identify last recording
        if ( broadcastUpdate )
            Notifier::broadcast( Notifications::RECORDING_STATE_UPDATED, AudioEngine::recordingFileId );
    }
}
