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
#include "bufferutility.h"
#include "debug.h"
#include <cstdio>
#include <definitions/notifications.h>
#include <messaging/notifier.h>
#include <utilities/stringutility.h>
#include <resizable_audiobuffer.h>

namespace MWEngine {

std::string DiskWriter::outputFile;
std::string DiskWriter::tempDirectory;
std::vector<DiskWriter::writtenFile> DiskWriter::outputFiles;
std::vector<ResizableBufferGroup*> DiskWriter::cachedBuffers{ nullptr, nullptr };

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

    // when full duplex recording is active, half the chunkSize to keep memory usage (more or less) equal
    int channelChunkSize = isFullDuplex() ? ceil( chunkSize / 2 ) : chunkSize;

    recordingChunkSize     = channelChunkSize;
    recordingChannelAmount = amountOfChannels;
    savedSnippets          = 0;
    outputWriterIndex      = 0;

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

    if ( AudioEngine::recordingState.bouncing ) {
        writeBufferToFile( currentBufferIndex, false );
    } else if ( broadcast ) {
        Notifier::broadcast( Notifications::RECORDED_SNIPPET_READY, currentBufferIndex );
    }
    prepareSnippet();

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
        auto file = outputFiles.at( 0 );

        // read WAV data from file

        AudioBuffer* tempBuffer = nullptr;

        if ( isFullDuplex() )
        {
            auto serializedBuffer = WaveReader::fileToBuffer( file.path ).buffer;

            if ( serializedBuffer != nullptr ) {

                // always calculate length from buffer (instead of taking recordingChunkSize)
                // as the buffer might not have recorded for a full recordingChunkSize in length!
                int serializedChunkSize = ceil( serializedBuffer->bufferSize / 2 );
                int nextSerializedChunkSize = 0;

                tempBuffer = new AudioBuffer( serializedBuffer->amountOfChannels, serializedChunkSize );

                // to correct the recorded input channel with the output channel we subtract the specified
                // latency in samples, this means we also need to read a little from the next available buffer

                AudioBuffer* nextSerializedBuffer = nullptr;
                if ( outputFiles.size() > 1 ) {
                    nextSerializedBuffer = WaveReader::fileToBuffer( outputFiles.at( 1 ).path ).buffer;
                    nextSerializedChunkSize = nextSerializedBuffer != nullptr ? ceil( nextSerializedBuffer->bufferSize / 2 ) : 0;
                }
                int nextInputReadOffset = serializedChunkSize - AudioEngine::recordingState.latency;

                for ( int c = 0, cl = serializedBuffer->amountOfChannels; c < cl; ++c ) {
                    auto outputBuffer = tempBuffer->getBufferForChannel( c );
                    auto sampleBuffer = serializedBuffer->getBufferForChannel( c );
                    auto nextSampleBuffer = nextSerializedChunkSize != 0 ? nextSerializedBuffer->getBufferForChannel( c ) : nullptr;

                    int inputOffset; // offset at which we can get the appropriate input sample, corrected for the latency

                    for ( int i = 0; i < serializedChunkSize; ++i ) {
                        SAMPLE_TYPE outputSample = sampleBuffer[ i ];
                        SAMPLE_TYPE inputSample  = SILENCE;

                        // note inputOffset starts at recordingChunkSize (== half the size of each file) as
                        // the input samples were serialized after the output samples

                        if ( nextInputReadOffset < 0 ) {
                            // this shouldn't occur
                        } else if ( i < nextInputReadOffset ) {
                            inputOffset = serializedChunkSize + ( i + AudioEngine::recordingState.latency );
                            inputSample = sampleBuffer[ inputOffset ];
                        } else if ( nextSampleBuffer != nullptr ) {
                            // read the amount of samples of the latency from the next buffer, when existing
                            inputOffset = nextSerializedChunkSize + ( i - nextInputReadOffset );
                            inputSample = nextSampleBuffer[ inputOffset ];
                        }

                        // mix in the offset corrected input sample with the output sample

                        outputBuffer[ i ] = capSampleSafe( outputSample + inputSample );
                    }
                }
                delete serializedBuffer;
                delete nextSerializedBuffer;
            }
        } else {
            tempBuffer = WaveReader::fileToBuffer( file.path ).buffer;
        }

        // delete the source WAV file so we immediately free storage space for
        // writing the buffer contents into the single file output stream,
        // note we assume that no errors have occurred with file writing
        // as the original recording is now gone!! then again, if it has errors
        // then the original recording isn't worth keeping anyways... :/

        remove( file.path.c_str() );

        if ( tempBuffer != nullptr ) {

            // convert data to a temporary PCM buffer
            // TODO: can we just extract the existing PCM data without this
            // back-and-forth conversion?
            INT16* outputBuffer = WaveWriter::bufferToPCM( tempBuffer );

            delete tempBuffer; // free memory allocated to read WAV file
            tempBuffer = nullptr;

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

    auto cachedBuffer = getCachedOutputBuffer( currentBufferIndex )->getBufferAtIndex( 0 );

    if ( cachedBuffer == nullptr ) {
        cachedBuffer = generateOutputBuffer( currentBufferIndex, channelAmount )->getBufferAtIndex( 0 );
    }
    cachedBuffer->mergeBuffers( aBuffer, 0, outputWriterIndex, MAX_VOLUME );
    outputWriterIndex += bufferSize;
}

void DiskWriter::appendBuffer( const float* outputBuffer, int bufferSize, int amountOfChannels )
{
    if ( !prepared ) {
        return;
    }
    auto cachedBuffer = getCachedOutputBuffer( currentBufferIndex )->getBufferAtIndex( 0 );

    if ( cachedBuffer == nullptr ) {
        cachedBuffer = generateOutputBuffer( currentBufferIndex, amountOfChannels )->getBufferAtIndex( 0 );
    }
    int i, c, ci;

    // write samples into cache buffers

    for ( i = 0, c = 0; i < bufferSize; ++i, ++outputWriterIndex, c += amountOfChannels )
    {
        if ( outputWriterIndex == recordingChunkSize ) {
            updateSnippetProgress( true, true );
            cachedBuffer = getCachedOutputBuffer( currentBufferIndex )->getBufferAtIndex( 0 );
        }
        for ( ci = 0; ci < amountOfChannels; ++ci ) {
            cachedBuffer->getBufferForChannel( ci )[ outputWriterIndex ] = outputBuffer[ c + ci ];
        }
    }
}

void DiskWriter::appendDuplexBuffers( const float* outputBuffer, AudioBuffer* inputBuffer, int outputBufferSize, int amountOfChannels, int latencyInSamples )
{
    if ( !prepared ) {
        return;
    }
    if ( outputBufferSize != inputBuffer->bufferSize ) {
        Debug::log( "cannot appendDuplexBuffers when input- and outputBuffer are of unequal size!" );
        return;
    }

    auto cachedOutputBuffer = getCachedOutputBuffer( currentBufferIndex )->getBufferAtIndex( 0 );
    if ( cachedOutputBuffer == nullptr ) {
        cachedOutputBuffer = generateOutputBuffer( currentBufferIndex, amountOfChannels )->getBufferAtIndex( 0 );
    }
    auto cachedInputBuffer = getCachedOutputBuffer( currentBufferIndex )->getBufferAtIndex( 1 );

    int i, c, ci;

    // write samples into cache buffers

    for ( i = 0, c = 0; i < outputBufferSize; ++i, ++outputWriterIndex, c += amountOfChannels )
    {
        if ( outputWriterIndex == recordingChunkSize ) {
            updateSnippetProgress( true, true );
            cachedOutputBuffer = getCachedOutputBuffer( currentBufferIndex )->getBufferAtIndex( 0 );
            cachedInputBuffer  = getCachedOutputBuffer( currentBufferIndex )->getBufferAtIndex( 1 );
        }
        for ( ci = 0; ci < amountOfChannels; ++ci ) {
            cachedOutputBuffer->getBufferForChannel( ci )[ outputWriterIndex ] = outputBuffer[ c + ci ];
            cachedInputBuffer->getBufferForChannel( ci )[ outputWriterIndex ] = inputBuffer->getBufferForChannel( ci )[ i ];
        }
    }
}

void DiskWriter::writeBufferToFile( int bufferIndex, bool broadcastUpdate )
{
    if ( !prepared ) {
        return;
    }
    auto cachedBuffer = getCachedOutputBuffer( bufferIndex );

    if ( cachedBuffer == nullptr ) {
        return;
    }
    int sampleRate = ( int ) AudioEngineProps::SAMPLE_RATE;

    // create output file name
    std::string snippetFileName = std::string(
        tempDirectory
    ).append( "rec_snippet_" + TO_STRING( savedSnippets ) + ".WAV" );

    size_t writtenWAVsize;
    int groupSize = cachedBuffer->getGroupSize();

    if ( groupSize == 1 ) {
        writtenWAVsize = WaveWriter::bufferToWAV( snippetFileName, cachedBuffer->getBufferAtIndex( 0 ), sampleRate );
        flushOutput( bufferIndex ); // free allocated buffer memory
    } else {
        auto buffer = cachedBuffer->getContentSerialized();
        flushOutput( bufferIndex ); // free allocated buffer memory
        // we're cheating here, the written size will eventually be the size of a single mixed buffer
        writtenWAVsize = ceil( WaveWriter::bufferToWAV( snippetFileName, buffer, sampleRate ) / groupSize );
        delete buffer; // free allocated serialized buffer
    }

    // store output file in vector
    writtenFile file;
    file.path = snippetFileName;
    file.size = writtenWAVsize;
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
    auto currentBuffer = getCachedOutputBuffer( currentBufferIndex );
    if ( currentBuffer != nullptr ) {
        currentBuffer->resize( outputWriterIndex );
    }

    // swap the currently active buffer index
    currentBufferIndex = ( currentBufferIndex == 0 ) ? 1 : 0;

    // generate output buffer for the next snippets index
    generateOutputBuffer( currentBufferIndex, recordingChannelAmount );
}

ResizableBufferGroup* DiskWriter::getCachedOutputBuffer( int bufferIndex )
{
    return cachedBuffers.at(( unsigned long ) bufferIndex );
}

ResizableBufferGroup* DiskWriter::generateOutputBuffer( int bufferIndex, int amountOfChannels )
{
    flushOutput( bufferIndex ); // free previous contents when existing

    if (( bufferIndex + 1 ) > cachedBuffers.size()) {
        cachedBuffers.resize(( unsigned long ) ( bufferIndex ) + 1 );
    }
    int bufferAmount = isFullDuplex() ? 2 : 1;
    auto *out = new ResizableBufferGroup( bufferAmount, amountOfChannels, recordingChunkSize );

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
    auto *cachedBuffer = getCachedOutputBuffer( bufferIndex );
    delete cachedBuffer;
    cachedBuffers.at(( unsigned long ) bufferIndex ) = nullptr;
}

} // E.O namespace MWEngine
