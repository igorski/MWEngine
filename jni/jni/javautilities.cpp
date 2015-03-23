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
#include "javautilities.h"
#include "../audiobuffer.h"
#include "..//wavetable.h"
#include <utilities/samplemanager.h>
#include <utilities/tablepool.h>

/* SampleManager hooks */

void JavaUtilities::createSampleFromBuffer( jstring aKey, jint aBufferLength, jint aChannelAmount, jdoubleArray aBuffer, jdoubleArray aOptRightBuffer )
{
    AudioBuffer* sampleBuffer = new AudioBuffer( aChannelAmount, aBufferLength );

    int i = 0;

    // get a pointer to the Java array
    jdouble *c_array;
    c_array = JavaBridge::getEnvironment()->GetDoubleArrayElements( aBuffer, 0 );

    // exception checking
    if ( c_array == NULL )
        return;

    // copy buffer contents
    SAMPLE_TYPE* channelBuffer = sampleBuffer->getBufferForChannel( 0 );

    for ( i = 0; i < aBufferLength; i++ )
        channelBuffer[ i ] = ( float ) c_array[ i ];

    // release the memory so Java can have it again
    JavaBridge::getEnvironment()->ReleaseDoubleArrayElements( aBuffer, c_array, 0 );

    // stereo ?

    if ( aChannelAmount == 2 )
    {
        c_array = JavaBridge::getEnvironment()->GetDoubleArrayElements( aOptRightBuffer, 0 );

        // exception checking
        if ( c_array == NULL )
            return;

        // copy buffer contents
        channelBuffer = sampleBuffer->getBufferForChannel( 1 );

        for ( i = 0; i < aBufferLength; i++ )
            channelBuffer[ i ] = ( float ) c_array[ i ];

        // release the memory so Java can have it again
        JavaBridge::getEnvironment()->ReleaseDoubleArrayElements( aOptRightBuffer, c_array, 0 );
    }

    // convert jstring to std::string
    const char *s = JavaBridge::getEnvironment()->GetStringUTFChars( aKey, NULL );
    std::string theKey = s;
    JavaBridge::getEnvironment()->ReleaseStringUTFChars( aKey, s );

    SampleManager::setSample( theKey, sampleBuffer );
}

/* TablePool hooks */

void JavaUtilities::cacheTable( jint tableLength, jint waveformType )
{
    WaveTable* tempTable = new WaveTable( tableLength, waveformType );
    TablePool::getTable( tempTable, waveformType ); // generate from and store in pool
    delete tempTable;                               // free memory
}

void JavaUtilities::cacheTable( jint tableLength, jint waveformType, jdoubleArray aBuffer )
{
    WaveTable* tempTable = new WaveTable( tableLength, waveformType );
    SAMPLE_TYPE* buffer  = tempTable->getBuffer();

    // get a pointer to the Java array
    jdouble *c_array;
    c_array = JavaBridge::getEnvironment()->GetDoubleArrayElements( aBuffer, 0 );

    // exception checking
    if ( c_array == NULL )
        return;

    // copy buffer contents

    for ( int i = 0; i < tableLength; i++ )
        buffer[ i ] = ( float ) c_array[ i ];

    // release the memory so Java can have it again
    JavaBridge::getEnvironment()->ReleaseDoubleArrayElements( aBuffer, c_array, 0 );

    TablePool::getTable( tempTable, waveformType ); // is fully generated, store it in the pool
    delete tempTable;                               // free memory
}
