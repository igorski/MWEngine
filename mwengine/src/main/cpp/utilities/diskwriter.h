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
#ifndef __MWENGINE__DISKWRITER_H_INCLUDED__
#define __MWENGINE__DISKWRITER_H_INCLUDED__

#include <string>
#include <vector>
#include <resizable_buffergroup.h>
#include <audioengine.h>

/**
 * DiskWriter is a utility that records audio for a given buffer length
 * (for instance to record a whole measure of live generated music) and
 * appends the AudioBuffer rendered by the engines until the buffer is full,
 * after which the contents can be written onto disk in PCM .WAV format
 *
 * To overcome excessive memory consumption / CPU resources recordings are
 * written in snippets (with a length defined by chunkSize in the prepare()-method).
 * When a full snippet has been recorded, a
 */
namespace MWEngine {
class DiskWriter
{
    public:
        static int currentBufferIndex; // index of the cachedBuffer the output is currently being written to

        /**
         * Prepare for a new recording. The recording can consist
         * of multiple iterations, each of given chunkSize in buffer size.
         * When completed (through finish()) the rendered file will be written to the path
         * described by outputFilename.
         */
        static void prepare( std::string outputFilename, int chunkSize, int amountOfChannels );
    
        /**
         * Verifies whether the current snippet is "complete" (e.g. its buffer is full)
         * If so, this will broadcast RECORDED_SNIPPET_READY message so the current recording can
         * be written to temporary storage.
         *
         * This saving should happen in a separate thread to prevent buffer under runs from occurring
         * while the engine continues to render audio while recording large fragments. Saving should
         * however complete before the next snippet has filled its complete buffer to prevent data loss!
         *
         * Additionally, this will prepare the next snippet and reset the write indices.
         *
         * @param {bool} force whether to force finish writing to the current snippet and swap the
         *               active snippet buffers (otherwise swap only occurs when current snippet buffer is full)
         * @param {bool} broadcast whether to broadcast RECORDED_SNIPPET_READY message (can be false
         *               when halting all recording and executing synchronous writing during completion)
         */
        static bool updateSnippetProgress( bool force, bool broadcast );
    
        /**
         * Completes a recording. This will close the current snippet, write it to disk
         * (synchronously) and subsequently combine all written files into a single WAV file
         * (path defined by outputFile).
         *
         * For best performance while keeping the engine rendering audio, execute
         * this in a separate thread.
         */
        static bool finish();

        // TODO rename appendBuffer and appendDuplexBuffers to imply non full duplex recording ?

        /**
         * appends an AudioBuffer into the current snippets output buffer
         */
        static void appendBuffer( AudioBuffer* aBuffer );

        /**
         * append the actual output buffer from the engine
         * into the current snippets output buffer
         */
        static void appendBuffer( const float* outputBuffer, int bufferSize, int amountOfChannels );

        /**
         * same as appendBuffer() except that the contents of given inputBuffer are also mixed
         * into the current snippets output buffer correcting for the given latencyInSamples
         * to address a mismatch when mixing device input and internal output streams)
         */
        static void appendDuplexBuffers( const float* outputBuffer, AudioBuffer* inputBuffer, int outputBufferSize, int amountOfChannels, int latencyInSamples );

        /**
         * write the contents of the snippet buffer into an output file, this will only write content
         * up until the point the buffer was written to in order to cut silence in case the buffer
         * wasn't filled in its entirety
         */
        static void writeBufferToFile( int bufferIndex, bool broadcastUpdate );

    private:

        typedef struct
        {
            std::string path; // path to written WAV file
            size_t size;      // total size of the WAV file's sample buffer
    
        } writtenFile;
        
        static std::string               outputFile;         // the file name to write the output to when finished
        static std::string               tempDirectory;      // output directory to write temporary files to
        static std::vector<writtenFile>  outputFiles;
        static std::vector<ResizableBufferGroup*> cachedBuffers;

        static int recordingChunkSize;
        static int outputWriterIndex;
        static int savedSnippets; // amount of snippets within the current recording that have been saved
        static int recordingChannelAmount;
        static bool prepared;

        /**
         * Prepares the next snippet to continue recording audio into.
         * We allow two snippets to exist at a time for either output- or input buffer types
         */
        static void prepareSnippet();

        static ResizableBufferGroup* getCachedOutputBuffer( int bufferIndex );

        /**
         * allocates a new buffer (at given index) for writing output or input into
         */
        static ResizableBufferGroup* generateOutputBuffer( int bufferIndex, int amountOfChannels );

        /**
         * checks whether the current write buffer is full
         */
        static bool isSnippetBufferFull();
        static void flushOutput( int bufferIndex );

        /**
         * when the correctLatency flag is set, we know we are dealing with a full duplex recording
         * as such, we write into separate engine output and device input buffers (so we can correct
         * for the roundtrip latency on finish())
         */
        static bool isFullDuplex() {
            return AudioEngine::recordingState.correctLatency;
        }
};
} // E.O namespace MWEngine

#endif
