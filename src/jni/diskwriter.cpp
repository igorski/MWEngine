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
#include "diskwriter.h"
#include "wavewriter.h"
#include "utils.h"
#include "global.h"
#include "java_bridge.h"

namespace DiskWriter
{
    std::string outputDirectory;
    unsigned long outputBufferSize  = 0;
    unsigned long outputWriterIndex = 0;
    short int* cachedBuffer         = 0;

    void prepareOutput( std::string aOutputDir, int aBufferSize )
    {
        DiskWriter::outputDirectory  = aOutputDir;
        DiskWriter::outputBufferSize = aBufferSize;

        DiskWriter::generateOutputBuffer();
    }

    void generateOutputBuffer()
    {
        int bufferSize = DiskWriter::outputBufferSize;

        DiskWriter::flushOutput(); // free previous contents
        DiskWriter::cachedBuffer = new short int[ bufferSize ];

        // fill buffer with silence
        for ( int i = 0; i < bufferSize; ++i )
        {
            // VERY poor mans check if no flushing operation occurred during creation
            if ( DiskWriter::cachedBuffer != 0 )
                DiskWriter::cachedBuffer[ i ] = 0;
        }
        DiskWriter::outputWriterIndex = 0;
    }

    void appendBuffer( AudioBuffer* aBuffer )
    {
        int bufferSize = aBuffer->bufferSize;

        if ( DiskWriter::cachedBuffer == 0 )
            generateOutputBuffer();

        // TODO: currently MONO ONLY !!
        float* channelBuffer = aBuffer->getBufferForChannel( 0 );
        int writerIndex       = DiskWriter::outputWriterIndex;
        int MAX_VALUE         = 32767; // convert floats to shorts

        // write floats as short values
        for ( int i = 0; i < bufferSize; ++i )
        {
            short int sample = ( short int )( channelBuffer[ i ] * MAX_VALUE );

            if ( sample > MAX_VALUE )
                sample = MAX_VALUE;

            else if ( sample < -MAX_VALUE )
                sample = -MAX_VALUE;

            DiskWriter::cachedBuffer[ writerIndex + i ] = sample;
        }
        DiskWriter::outputWriterIndex = writerIndex + bufferSize;
    }

    bool bufferFull()
    {
        return DiskWriter::outputWriterIndex >= DiskWriter::outputBufferSize;
    }

    void flushOutput()
    {
        if ( DiskWriter::cachedBuffer != 0 )
        {
            delete[] DiskWriter::cachedBuffer;
            DiskWriter::cachedBuffer = 0;
        }
        DiskWriter::outputWriterIndex = 0;
    }

    void writeBufferToFile( int aSampleRate, int aNumChannels, bool broadcastUpdate )
    {
        // quick assertion
        if ( DiskWriter::cachedBuffer == 0 )
            return;

        aNumChannels = 1;   // TODO : currently MONO only (see appendBuffer above)

        // we improve the use of the CPU resources
        // by creating a small local thread
        // TODO: do it ? (this non-threading blocks the renderer, but nicely omits issue w/ continuous writes ;) )

        //pthread_t t1;
        //pthread_create( &t1, NULL, &print_message, NULL );

        // copy string contents for appending of filename
        std::string outputFile = std::string( DiskWriter::outputDirectory.c_str());

        int bufferSize = DiskWriter::outputBufferSize;

        // uh oh.. recorded less than maximum available in buffer ? cut silence
        if ( DiskWriter::outputWriterIndex < bufferSize )
        {
            bufferSize = DiskWriter::outputWriterIndex;

            short int* tempBuffer = new short int[ bufferSize ];

            for ( int i = 0; i < bufferSize; ++i )
                tempBuffer[ i ] = DiskWriter::cachedBuffer[ i ];

            write_wav( outputFile.append( SSTR( recordingFileName )), bufferSize, tempBuffer, aSampleRate, aNumChannels );

            delete[] tempBuffer;
        }
        else {
            write_wav( outputFile.append( SSTR( recordingFileName )), bufferSize, DiskWriter::cachedBuffer, aSampleRate, aNumChannels );
        }
        DiskWriter::flushOutput(); // free memory

        if ( broadcastUpdate )
        {
            // broadcast update via JNI, pass buffer identifier name to identify last recording
            jmethodID native_method_id = getJavaMethod( JavaAPIs::RECORDING_UPDATE );

            if ( native_method_id != 0 )
            {
                JNIEnv* env = getEnvironment();

                if ( env != 0 )
                    env->CallStaticVoidMethod( getJavaInterface(), native_method_id, recordingFileName );
            }
        }
        //void* result;
        //pthread_join( t1, &result );
    }
}
