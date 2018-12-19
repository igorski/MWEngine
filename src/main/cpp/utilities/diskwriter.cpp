/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2018 Igor Zinken - http://www.igorski.nl
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
#include "wavereader.h"
#include "utils.h"
#include <stdio.h>
#include <definitions/notifications.h>
#include <messaging/notifier.h>

namespace DiskWriter
{
    std::string              outputFile;
    std::string              tempDirectory;
    std::vector<writtenFile> outputFiles;
    unsigned long            tempChunkSize     = 0;
    unsigned long            outputWriterIndex = 0;
    AudioBuffer*             cachedBuffer      = nullptr;

    void prepare( std::string outputFilename, int chunkSize, int amountOfChannels )
    {
        outputFile = outputFilename;

        std::string path   = std::string( outputFilename );
        std::size_t dirPos = path.find_last_of( "/" );
        tempDirectory      = path.substr( 0, dirPos ) + "/";

        tempChunkSize = chunkSize;

        outputFiles.clear();
        generateOutputBuffer( amountOfChannels );
    }

    bool finish()
    {
        if ( outputFiles.size() == 0 )
            return false;

        // calculate total data size of concatenated wave files

        size_t totalBufferSize = 0;

        for ( std::size_t i = 0; i < outputFiles.size(); ++i )
            totalBufferSize += outputFiles.at( i ).size;

        // create a stream for writing the output file to

        std::ofstream waveStream = WaveWriter::createWAVStream(
            outputFile.c_str(), totalBufferSize,
            AudioEngineProps::SAMPLE_RATE, AudioEngineProps::OUTPUT_CHANNELS
        );

        // concatenate the wave files into a single output file

        while ( outputFiles.size() > 0 )
        {
            writtenFile file = outputFiles.at( 0 );

            // read WAV data from file

            AudioBuffer* tempBuffer = WaveReader::fileToBuffer( file.path ).buffer;

            if ( tempBuffer != nullptr ) {

                // convert data to a temporary PCM buffer
                // TODO: can we just extract the existing PCM data without this
                // back-and-forth conversion?
                short int* outputBuffer = WaveWriter::bufferToPCM( tempBuffer );

                delete tempBuffer; // free memory allocated to read WAV file

                // write PCM buffer into the output stream

                WaveWriter::appendBufferToStream( waveStream, outputBuffer, file.size );

                delete[] outputBuffer; // free memory allocated to the temporary PCM buffer
            }

            // delete the source WAV file

            remove( file.path.c_str() );

            // remove from vector, iterate onto next

            outputFiles.erase( outputFiles.begin() );
        }
        waveStream.close();

        return true;
    }

    /**
     * allocates a new buffer for the next write iteration
     */
    void generateOutputBuffer( int amountOfChannels )
    {
        flushOutput(); // free previous contents
        cachedBuffer = new AudioBuffer( amountOfChannels, tempChunkSize );

        outputWriterIndex = 0;
    }

    /**
     * append an AudioBuffer into the write buffer
     */
    void appendBuffer( AudioBuffer* aBuffer )
    {
        int bufferSize    = aBuffer->bufferSize;
        int channelAmount = aBuffer->amountOfChannels;

        if ( cachedBuffer == nullptr )
            generateOutputBuffer( channelAmount );

        cachedBuffer->mergeBuffers( aBuffer, 0, outputWriterIndex, 1.0 );
        outputWriterIndex += bufferSize;
    }

    /**
     * append the actual output buffer from the engine
     * into the write buffer
     */
    void appendBuffer( float* aBuffer, int aBufferSize, int amountOfChannels )
    {
        if ( cachedBuffer == nullptr )
            generateOutputBuffer( amountOfChannels );

        int i, c, ci;

        // write samples into cache buffers

        for ( i = 0, c = 0; i < aBufferSize; ++i, ++outputWriterIndex, c += amountOfChannels )
        {
            if ( outputWriterIndex == tempChunkSize )
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
        return outputWriterIndex >= tempChunkSize;
    }

    /**
     * flush the contents of the write buffer
     */
    void flushOutput()
    {
        if ( cachedBuffer != nullptr )
            delete cachedBuffer;

        cachedBuffer      = nullptr;
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
        if ( cachedBuffer == nullptr )
            return;

        // create output file name
        std::string outputFile = std::string(
            tempDirectory.c_str()
        ).append( "rec_snippet_" + SSTR( AudioEngine::recordingFileId ) + ".WAV" );

        int bufferSize        = tempChunkSize;
        size_t writtenWAVSize = 0;

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
            writtenWAVSize = WaveWriter::bufferToWAV( outputFile, tempBuffer, aSampleRate );

            // free memory allocated by temporary buffer

            delete tempBuffer;
        }
        else
        {
            writtenWAVSize = WaveWriter::bufferToWAV( outputFile, cachedBuffer, aSampleRate );
        }

        flushOutput(); // free allocated buffer memory

        // store output file in vector
        writtenFile file;
        file.path = outputFile;
        file.size = writtenWAVSize;
        outputFiles.push_back( file );

        // broadcast update, pass buffer identifier to identify last recording
        if ( broadcastUpdate )
            Notifier::broadcast( Notifications::RECORDING_STATE_UPDATED, AudioEngine::recordingFileId );
    }
}
