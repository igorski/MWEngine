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
#ifndef __WAVEWRITER_H_INCLUDED__
#define __WAVEWRITER_H_INCLUDED__

#include <string>
#include <fstream>

void write_wav( std::string outputFile, unsigned long num_samples, short int * buffer, int sample_rate, int num_channels );

/**
 * Fri Jun 18 16:36:23 PDT 2010 Kevin Karplus
 * Creative Commons license Attribution-NonCommercial
 * http://creativecommons.org/licenses/by-nc/3.0/
 */
template <typename T>
void t_streamwrite( std::ofstream& stream, const T& t )
{
    stream.write(( const char* )&t, sizeof( T ));
}

template <typename SampleType>
void writeWAVData( const char* outFile, SampleType* buf, size_t bufSize,
                   int sampleRate, short channels )
{
    std::ofstream stream( outFile, std::ios::binary );
    stream.write( "RIFF", 4 );
    t_streamwrite<int>( stream, 36 + bufSize );
    stream.write( "WAVE", 4 );
    stream.write( "fmt ", 4 );
    t_streamwrite<int>  ( stream, 16 );
    t_streamwrite<short>( stream, 1 );                                           // Format (1 = PCM)
    t_streamwrite<short>( stream, channels );                                    // Channels
    t_streamwrite<int>  ( stream, sampleRate );                                  // Sample Rate
    t_streamwrite<int>  ( stream, sampleRate * channels * sizeof( SampleType )); // Byterate
    t_streamwrite<short>( stream, channels * sizeof( SampleType ));              // Frame size
    t_streamwrite<short>( stream, 8 * sizeof( SampleType ));                     // Bits per sample
    stream.write( "data", 4 );
    stream.write(( const char* )&bufSize, 4 );
    stream.write(( const char* )buf, bufSize );
}

#endif