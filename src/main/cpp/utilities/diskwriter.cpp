/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2019 Igor Zinken - http://www.igorski.nl
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

namespace MWEngine {
namespace DiskWriter
{
    std::string               outputFile;
    std::string               tempDirectory;
    std::vector<writtenFile>  outputFiles;
    std::vector<AudioBuffer*> cachedBuffers{ nullptr, nullptr };

    int recordingChunkSize     = 0;
    int outputWriterIndex      = 0;
    int currentBufferIndex     = 0;
    int savedSnippets          = 0;
    int recordingChannelAmount = AudioEngineProps::OUTPUT_CHANNELS;

    bool prepared = false;

    void prepare( std::string outputFilename, int chunkSize, int amountOfChannels )
    {
        outputFile = outputFilename;

        std::string path   = std::string( outputFilename );
        std::size_t dirPos = path.find_last_of( "/" );
        tempDirectory      = path.substr( 0, dirPos ) + "/";

        recordingChunkSize     = chunkSize;
        recordingChannelAmount = amountOfChannels;
        savedSnippets          = 0;

        outputFiles.clear();
        prepareSnippet();

        prepared = true;
    }

    void prepareSnippet()
    {
        // swap the currently active buffer
        currentBufferIndex = ( currentBufferIndex == 0 ) ? 1 : 0;
        generateOutputBuffer( currentBufferIndex, recordingChannelAmount );
    }

    bool finish()
    {
        if ( !prepared )
            return false;

        // flush all temporary buffers as recording has finished
        // and snippets have been written onto disk

        for ( int i = 0; i < 2; ++i )
            flushOutput( i );

        if ( outputFiles.size() == 0 )
            return false;

        // calculate total data size of concatenated wave files

        size_t totalBufferSize = 0;

        for ( std::size_t i = 0; i < outputFiles.size(); ++i )
            totalBufferSize += outputFiles.at( i ).size;

        // create a stream for writing the output file to

        std::ofstream waveStream = WaveWriter::createWAVStream(
            outputFile.c_str(), totalBufferSize,
            AudioEngineProps::SAMPLE_RATE, recordingChannelAmount
        );

        // concatenate the wave files into a single output file

        while ( outputFiles.size() > 0 )
        {
            writtenFile file = outputFiles.at( 0 );

            // read WAV data from file

            AudioBuffer* tempBuffer = WaveReader::fileToBuffer( file.path ).buffer;

            // delete the source WAV file so we immediately free storage space for
            // writing the buffer contents into the single file output stream,
            // note we assume that no errors have occurred with file writing
            // as the original recording is now gone!! then again, if it has errors
            // than the original recording isn't worth keeping anyways... ;)

            remove( file.path.c_str() );

            if ( tempBuffer != nullptr ) {

                // convert data to a temporary PCM buffer
                // TODO: can we just extract the existing PCM data without this
                // back-and-forth conversion?
                INT16* outputBuffer = WaveWriter::bufferToPCM( tempBuffer );

                delete tempBuffer; // free memory allocated to read WAV file

                // write PCM buffer into the output stream

                WaveWriter::appendBufferToStream( waveStream, outputBuffer, file.size );

                delete[] outputBuffer; // free memory allocated to the temporary PCM buffer
            }

            // remove from vector, iterate onto next

            outputFiles.erase( outputFiles.begin() );
        }
        waveStream.close();

        prepared = false;

        return true;
    }

    /**
     * allocates a new buffer for writing the next snippet
     */
    AudioBuffer* generateOutputBuffer( int bufferIndex, int amountOfChannels )
    {
        flushOutput( bufferIndex ); // free previous contents when existing

        if (( bufferIndex + 1 ) > cachedBuffers.size())
            cachedBuffers.resize(( unsigned long )( bufferIndex + 1 ));

        AudioBuffer* out = new AudioBuffer( amountOfChannels, recordingChunkSize );

        cachedBuffers.at(( unsigned long ) bufferIndex ) = out;
        outputWriterIndex = 0;

        return out;
    }

    /**
     * appends an AudioBuffer into the current snippets output buffer
     */
    void appendBuffer( AudioBuffer* aBuffer )
    {
        if ( !prepared )
            return;

        int bufferSize    = aBuffer->bufferSize;
        int channelAmount = aBuffer->amountOfChannels;

        AudioBuffer* cachedBuffer = getCachedBuffer( currentBufferIndex );

        if ( cachedBuffer == nullptr )
            cachedBuffer = generateOutputBuffer( currentBufferIndex, channelAmount );

        cachedBuffer->mergeBuffers( aBuffer, 0, outputWriterIndex, 1.0f );
        outputWriterIndex += bufferSize;
    }

    /**
     * append the actual output buffer from the engine
     * into the current snippets output buffer
     */
    void appendBuffer( float* aBuffer, int aBufferSize, int amountOfChannels )
    {
        if ( !prepared )
            return;

        AudioBuffer* cachedBuffer = getCachedBuffer( currentBufferIndex );

        if ( cachedBuffer == nullptr )
            cachedBuffer = generateOutputBuffer( currentBufferIndex, amountOfChannels );

        int i, c, ci;

        // write samples into cache buffers

        for ( i = 0, c = 0; i < aBufferSize; ++i, ++outputWriterIndex, c += amountOfChannels )
        {
            if ( outputWriterIndex == recordingChunkSize )
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
        return outputWriterIndex >= recordingChunkSize;
    }

    /**
     * write the contents of the write buffer into
     * an output file, this will only write content
     * up until the point if was written to in case
     * the buffer wasn't full yet
     */
    void writeBufferToFile( int bufferIndex, bool broadcastUpdate )
    {
        if ( !prepared )
            return;

        AudioBuffer* cachedBuffer = getCachedBuffer( bufferIndex );

        if ( cachedBuffer == nullptr )
            return;

        int sampleRate = AudioEngineProps::SAMPLE_RATE;

        // create output file name
        std::string outputFile = std::string(
            tempDirectory.c_str()
        ).append( "rec_snippet_" + SSTR( savedSnippets ) + ".WAV" );

        int bufferSize        = recordingChunkSize;
        size_t writtenWAVSize = 0;

        // recorded less than maximum available in buffer ? cut silence
        // by writing recording into temporary buffers

        if ( outputWriterIndex < bufferSize )
        {
            bufferSize = outputWriterIndex;

            AudioBuffer* tempBuffer = new AudioBuffer( recordingChannelAmount, bufferSize );

            for ( int i = 0; i < bufferSize; ++i )
            {
                for ( int c = 0; c < recordingChannelAmount; ++c )
                    tempBuffer->getBufferForChannel( c )[ i ] = cachedBuffer->getBufferForChannel( c )[ i ];
            }
            writtenWAVSize = WaveWriter::bufferToWAV( outputFile, tempBuffer, sampleRate );

            // free memory allocated by temporary buffer

            delete tempBuffer;
        }
        else
        {
            writtenWAVSize = WaveWriter::bufferToWAV( outputFile, cachedBuffer, sampleRate );
        }

        flushOutput( bufferIndex ); // free allocated buffer memory

        // store output file in vector
        writtenFile file;
        file.path = outputFile;
        file.size = writtenWAVSize;
        outputFiles.push_back( file );

        // broadcast update
        if ( broadcastUpdate )
            Notifier::broadcast( Notifications::RECORDED_SNIPPET_SAVED, savedSnippets );

        ++savedSnippets;
    }

    /* internal methods */

    AudioBuffer* getCachedBuffer( int bufferIndex )
    {
        return cachedBuffers.at(( unsigned long ) bufferIndex );
    }

    void flushOutput( int bufferIndex )
    {
        AudioBuffer* cachedBuffer = getCachedBuffer( bufferIndex );

        if ( cachedBuffer != nullptr )
            delete cachedBuffer;

        cachedBuffers.at(( unsigned long ) bufferIndex ) = nullptr;
    }
}

} // E.O namespace MWEngine
