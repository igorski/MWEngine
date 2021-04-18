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
#ifndef __MWENGINE__DISKWRITER_H_INCLUDED__
#define __MWENGINE__DISKWRITER_H_INCLUDED__

#include "audiobuffer.h"
#include <string>
#include <vector>

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
namespace DiskWriter
{
    /* internal properties */

    //namespace
    //{
        typedef struct
        {
            std::string path; // path to written WAV file
            size_t size;      // total size of the WAV file's sample buffer

        } writtenFile;

        extern std::string               outputFile;         // the file name to write the output to when finished
        extern std::string               tempDirectory;      // output directory to write temporary files to
        extern std::vector<writtenFile>  outputFiles;
        extern std::vector<AudioBuffer*> cachedBuffers;

        extern int recordingChunkSize;
        extern int outputWriterIndex;
        extern int savedSnippets;      // amount of snippets within the current recording that have been saved
        extern int recordingChannelAmount;

        extern bool prepared;

        extern AudioBuffer* getCachedBuffer( int bufferIndex );
        extern void flushOutput( int bufferIndex );

        extern AudioBuffer* generateOutputBuffer( int bufferIndex, int amountOfChannels );
    //}

    /* public properties / methods */

    extern int currentBufferIndex; // index of the cachedBuffer currently being written to

    /**
     * Prepare for a new recording. The recording can consist
     * of multiple iterations, each of given chunkSize in buffer size.
     */
    extern void prepare( std::string outputFilename, int chunkSize, int amountOfChannels );

    /**
     * Invoked by the engine whenever a snippet has filled its buffer. This method will prepare the
     * next snippet to continue recording audio into. We allow two snippets to exist at a time.
     * When this is invoked, the engine will at the same time broadcast RECORDED_SNIPPET_READY
     * which should be used to save the snippet contents onto storage so it can be unloaded from memory.
     * This saving should happen in a separate thread to prevent buffer under runs from occurring while
     * the engine continues to render audio while recording large fragments. Saving should however
     * complete before the next snippet has filled its complete buffer to prevent data loss!
     */
    extern void prepareSnippet();

    /**
     * Complete a recording. This will combine all written files
     * into a single WAV file defined by outputFile.
     *
     * For best performance while keeping the engine rendering audio, execute
     * this in a separate thread.
     */
    extern bool finish();

    extern void appendBuffer( AudioBuffer* aBuffer );
    extern void appendBuffer( float* aBuffer, int aBufferSize, int amountOfChannels );
    extern bool bufferFull();
    extern void writeBufferToFile( int bufferIndex, bool broadcastUpdate );
}
} // E.O namespace MWEngine

#endif
