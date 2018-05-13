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
#include "wavewriter.h"
#include "global.h"
#include "utils.h"
#include <fstream>

void WaveWriter::bufferToFile( std::string outputFile, AudioBuffer* buffer, int sampleRate )
{
    int writerIndex      = 0;
    int bufferSize       = buffer->bufferSize;
    int amountOfChannels = buffer->amountOfChannels;

    // convert floating point buffer samples to interleaved shorts (PCM audio)

    short int* outputBuffer = new short int[ bufferSize * amountOfChannels ];
    short int MAX_VALUE     = 32767;

    // write samples into PCM short buffer

    for ( int i = 0; i < bufferSize; ++i, writerIndex += amountOfChannels )
    {
        for ( int c = 0; c < amountOfChannels; ++c )
        {
            short int sample = ( short int )( buffer->getBufferForChannel( c )[ i ] * MAX_VALUE );

            // keep sample within range

            if ( sample > +MAX_VALUE )
                sample = +MAX_VALUE;

            else if ( sample < -MAX_VALUE )
                sample = -MAX_VALUE;

            outputBuffer[ writerIndex + c ] = sample;
        }
    }

    unsigned int bytes_per_sample = sizeof( short int );
    const char* filename          = outputFile.c_str();

    // actual writing of the file

    writeWAVData( filename, outputBuffer, bytes_per_sample * bufferSize * amountOfChannels,
                  sampleRate, ( short ) amountOfChannels );

    delete[] outputBuffer;  // free allocated memory
}
