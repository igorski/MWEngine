/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2015 Igor Zinken - http://www.igorski.nl
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
#ifndef __DISKWRITER_H_INCLUDED__
#define __DISKWRITER_H_INCLUDED__

#include "audiobuffer.h"
#include <string>
#include <vector>

/**
 * DiskWriter is a util that will record audio for a given buffer length
 * (for instance to record a whole measure of live generated music) and
 * appends the AudioBuffer generatad by the engines render queue
 * until the buffer is full, after which the contents can be written
 * onto disk in PCM WAV format
 */
namespace DiskWriter
{
    // output directory to write to
    extern std::string   outputDirectory;
    extern unsigned long outputBufferSize;
    extern unsigned long outputWriterIndex;
    extern AudioBuffer*  cachedBuffer;

    extern void prepare( std::string aOutputDir, int aBufferSize, int amountOfChannels );
    extern void generateOutputBuffer( int amountOfChannels );
    extern void flushOutput();
    extern void appendBuffer( AudioBuffer* aBuffer );
    extern void appendBuffer( float* aBuffer, int aBufferSize, int amountOfChannels );
    extern bool bufferFull();
    extern void writeBufferToFile( int aSampleRate, int aNumChannels, bool broadcastUpdate );
}

#endif
