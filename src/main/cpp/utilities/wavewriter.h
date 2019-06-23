/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2019 Igor Zinken - http://www.igorski.nl
 * Based on work by 2010 Kevin Karplus (writing WAV files using streams)
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
#ifndef __MWENGINE__WAVEWRITER_H_INCLUDED__
#define __MWENGINE__WAVEWRITER_H_INCLUDED__

#include <string>
#include <fstream>
#include <vector>
#include "../audiobuffer.h"
#include "../global.h"

namespace MWEngine {
class WaveWriter
{
    public:
        /**
         * Writes the contents of given AudioBuffer into a WAV file
         * Returns the size of the written WAV files buffer content
         */
        static size_t bufferToWAV( std::string outputFile, AudioBuffer* buffer, int sampleRate );

        /**
         * Create an output stream for writing a WAV file to
         * This can be used to write successive buffers into a single file
         * Note: totalBufSizeToWrite describes the final size of the written
         * WAV data after all write iterations have completed.
         */
        static std::ofstream createWAVStream( const char* outFile, size_t totalBufSizeToWrite,
                                              int sampleRate, int channels )
        {
            std::ofstream stream( outFile, std::ios::binary );

            // write the header data
            stream.write( "RIFF", 4 );
            t_streamwrite<UINT32>( stream, 36 + totalBufSizeToWrite );                   // file size
            stream.write( "WAVE", 4 );
            stream.write( "fmt ", 4 );
            t_streamwrite<UINT32>  ( stream, 16 );                                       // format length
            t_streamwrite<short>( stream, 1 );                                           // Format (1 = PCM)
            t_streamwrite<short>( stream, ( short ) channels );                          // Channels
            t_streamwrite<UINT32>( stream, sampleRate );                                 // Sample rate
            t_streamwrite<UINT32>( stream, sampleRate * channels * sizeof( short int )); // Byte rate
            t_streamwrite<short>( stream, channels * sizeof( short int ));               // Frame size
            t_streamwrite<short>( stream, 8 * sizeof( short int ));                      // Bits per sample
            stream.write( "data", 4 );
            stream.write(( const char* )&totalBufSizeToWrite, 4 );

            return stream;
        }

        /**
         * Appends the contents of given buffer to given stream
         */
        template <typename SampleType>
        static void appendBufferToStream( std::ofstream& stream, SampleType* buf, size_t bufSize )
        {
            stream.write(( const char* )buf, bufSize );
        }

        /**
         * Allocates a buffer that contains PCM compliant
         * representation of the samples in given AudioBuffer
         */
        static short int* bufferToPCM( AudioBuffer* buffer );

    protected:

        template <typename T>
        static void t_streamwrite( std::ofstream& stream, const T& t )
        {
            stream.write(( const char* )&t, sizeof( T ));
        }
};
} // E.O namespace MWEngine

#endif
