/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2022 Igor Zinken - https://www.igorski.nl
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
#include <cstdio>
#include <definitions/notifications.h>
#include <messaging/notifier.h>
#include <utilities/stringutility.h>

namespace MWEngine {

std::string DiskWriter::outputFile;
std::string DiskWriter::tempDirectory;
std::vector<DiskWriter::writtenFile> DiskWriter::outputFiles;
std::vector<ResizableAudioBuffer*> DiskWriter::cachedBuffers{ nullptr, nullptr };

int DiskWriter::currentBufferIndex = 0;
int DiskWriter::recordingChunkSize = 0;
int DiskWriter::outputWriterIndex  = 0;
int DiskWriter::savedSnippets      = 0;

int DiskWriter::recordingChannelAmount = AudioEngineProps::OUTPUT_CHANNELS;
bool DiskWriter::prepared = false;

/* public methods */

void DiskWriter::prepare( std::string outputFilename, int chunkSize, int amountOfChannels )
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

bool DiskWriter::updateSnippetProgress( bool force, bool broadcast )
{
    if ( !force && !isSnippetBufferFull()) {
        return false;
    }

    // when bouncing, this is done synchronously (as engine is not writing output to hardware), otherwise
    // broadcast that a snippet has recorded in full and can be written onto storage

    AudioBuffer* tempBuffer;

    if ( AudioEngine::recordingState.bouncing ) {
        writeBufferToFile( currentBufferIndex, false );
    } else {
        // when recording input and output in sync, we need to prepare the buffer to make up
        // for the correction at the size of the latency
        if ( AudioEngine::recordingState.correctLatency ) {
            tempBuffer = new AudioBuffer( AudioEngineProps::OUTPUT_CHANNELS, AudioEngine::recordingState.latency );
            auto cachedBuffer = getCachedBuffer( currentBufferIndex );
            outputWriterIndex = cachedBuffer->bufferSize - AudioEngine::recordingState.latency; // resizes on prepareSnippet()
            tempBuffer->mergeBuffers( cachedBuffer, outputWriterIndex, 0, MAX_VOLUME );
        }
        if ( broadcast ) {
            Notifier::broadcast( Notifications::RECORDED_SNIPPET_READY, currentBufferIndex );
        }
    }
    prepareSnippet();

    if ( AudioEngine::recordingState.correctLatency ) {
        appendBuffer( tempBuffer ); // write final chunk of last output into the newly prepare snippet
        delete tempBuffer;
    }
    return true;
}

bool DiskWriter::finish()
{
    if ( !prepared ) {
        return false;
    }

    int bufferIndex = currentBufferIndex;
    updateSnippetProgress( true, false );    // finish current snippet
    writeBufferToFile( bufferIndex, false ); // render current snippet to temp file

    // flush all temporary buffers now recording has finished
    // and all snippets have been written onto disk

    for ( int i = 0; i < 2; ++i ) {
        flushOutput( i );
    }

    if ( outputFiles.empty() ) {
        return false;
    }

    // calculate total data size of concatenated wave files

    size_t totalBufferSize = 0;

    for ( const auto & file : outputFiles ) {
        totalBufferSize += file.size;
    }

    // create a stream for writing the output file to

    std::ofstream waveStream = WaveWriter::createWAVStream(
        outputFile.c_str(), totalBufferSize,
        AudioEngineProps::SAMPLE_RATE, recordingChannelAmount
    );

    // concatenate the wave files into a single output file

    while ( ! outputFiles.empty() )
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

void DiskWriter::appendBuffer( AudioBuffer* aBuffer )
{
    if ( !prepared ) {
        return;
    }
    int bufferSize    = aBuffer->bufferSize;
    int channelAmount = aBuffer->amountOfChannels;

    ResizableAudioBuffer* cachedBuffer = getCachedBuffer( currentBufferIndex );

    if ( cachedBuffer == nullptr ) {
        cachedBuffer = generateOutputBuffer( currentBufferIndex, channelAmount );
    }
    cachedBuffer->mergeBuffers( aBuffer, 0, outputWriterIndex, MAX_VOLUME );
    outputWriterIndex += bufferSize;
}

void DiskWriter::appendBuffer( const float* aBuffer, int aBufferSize, int amountOfChannels )
{
    if ( !prepared ) {
        return;
    }
    ResizableAudioBuffer* cachedBuffer = getCachedBuffer( currentBufferIndex );

    if ( cachedBuffer == nullptr ) {
        cachedBuffer = generateOutputBuffer( currentBufferIndex, amountOfChannels );
    }
    int i, c, ci;

    // write samples into cache buffers

    for ( i = 0, c = 0; i < aBufferSize; ++i, ++outputWriterIndex, c += amountOfChannels )
    {
        if ( outputWriterIndex == recordingChunkSize ) {
            updateSnippetProgress( true, true );
            cachedBuffer = getCachedBuffer( currentBufferIndex );
        }
        for ( ci = 0; ci < amountOfChannels; ++ci ) {
            cachedBuffer->getBufferForChannel( ci )[ outputWriterIndex ] = aBuffer[ c + ci ];
        }
    }
}

void DiskWriter::mixInputBuffer( AudioBuffer* inputBuffer, int bufferSize, int amountOfChannels, int writeOffset )
{
    auto cachedBuffer  = getCachedBuffer( currentBufferIndex );
    int maxWriteOffset = cachedBuffer->bufferSize - 1;
    // mix in at last written position (minus buffer size as output is already written) plus given writeOffset
    int wo = ( outputWriterIndex - bufferSize ) + writeOffset;

    for ( int c = 0; c < amountOfChannels; ++c ) {
        auto sourceBuffer = inputBuffer->getBufferForChannel( c );
        auto targetBuffer = cachedBuffer->getBufferForChannel( c );
        for ( int i = 0; i < bufferSize; ++i ) {
            int destOffset = wo + i;
            if ( destOffset < 0 ) {
                continue;
            }
            if ( destOffset > maxWriteOffset ) {
                break;
            }
            targetBuffer[ destOffset ] = capSampleSafe( targetBuffer[ destOffset ] + sourceBuffer[ i ] );
        }
    }
}

void DiskWriter::writeBufferToFile( int bufferIndex, bool broadcastUpdate )
{
    if ( !prepared ) {
        return;
    }
    ResizableAudioBuffer* cachedBuffer = getCachedBuffer( bufferIndex );

    if ( cachedBuffer == nullptr ) {
        return;
    }
    int sampleRate = ( int ) AudioEngineProps::SAMPLE_RATE;

    // create output file name
    std::string snippetFileName = std::string(
            tempDirectory
    ).append( "rec_snippet_" + TO_STRING( savedSnippets ) + ".WAV" );

    size_t writtenWAVSize = WaveWriter::bufferToWAV( snippetFileName, cachedBuffer, sampleRate );

    flushOutput( bufferIndex ); // free allocated buffer memory

    // store output file in vector
    writtenFile file;
    file.path = snippetFileName;
    file.size = writtenWAVSize;
    outputFiles.push_back( file );

    // broadcast update
    if ( broadcastUpdate ) {
        Notifier::broadcast( Notifications::RECORDED_SNIPPET_SAVED, savedSnippets );
    }
    ++savedSnippets;
}

/* private methods */

void DiskWriter::prepareSnippet()
{
    // resize the currently active buffer to reflect its total written size
    ResizableAudioBuffer* currentBuffer = getCachedBuffer( currentBufferIndex );
    if ( currentBuffer != nullptr ) {
        currentBuffer->resize( outputWriterIndex );
    }

    // swap the currently active buffer index
    currentBufferIndex = ( currentBufferIndex == 0 ) ? 1 : 0;

    // generate output buffer for the next snippets index
    generateOutputBuffer( currentBufferIndex, recordingChannelAmount );
}

ResizableAudioBuffer* DiskWriter::getCachedBuffer( int bufferIndex )
{
    return cachedBuffers.at(( unsigned long ) bufferIndex );
}

ResizableAudioBuffer* DiskWriter::generateOutputBuffer( int bufferIndex, int amountOfChannels )
{
    flushOutput( bufferIndex ); // free previous contents when existing

    if (( bufferIndex + 1 ) > cachedBuffers.size()) {
        cachedBuffers.resize(( unsigned long ) ( bufferIndex ) + 1 );
    }
    auto* out = new ResizableAudioBuffer( amountOfChannels, recordingChunkSize );

    cachedBuffers.at(( unsigned long ) bufferIndex ) = out;
    outputWriterIndex = 0;

    return out;
}

bool DiskWriter::isSnippetBufferFull()
{
    return outputWriterIndex >= recordingChunkSize;
}

void DiskWriter::flushOutput( int bufferIndex )
{
    ResizableAudioBuffer* cachedBuffer = getCachedBuffer( bufferIndex );

    if ( cachedBuffer != nullptr ) {
        delete cachedBuffer;
    }
    cachedBuffers.at(( unsigned long ) bufferIndex ) = nullptr;
}

} // E.O namespace MWEngine
