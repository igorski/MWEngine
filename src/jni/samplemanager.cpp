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
#include "samplemanager.h"
#include "java_bridge.h"

namespace SampleManagerSamples
{
    std::map<std::string, cachedSample> _sampleMap;
}

/* public methods */

void SampleManager::setSample( std::string aKey, AudioBuffer* aBuffer )
{
    cachedSample sample = { aBuffer->bufferSize, aBuffer };

    // Assignment using member function insert() and STL pair
    SampleManagerSamples::_sampleMap.insert( std::pair<std::string, cachedSample>( aKey, sample ));
}

void SampleManager::setSample( jstring aKey, jint aBufferLength, jint aChannelAmount, jdoubleArray aBuffer, jdoubleArray aOptRightBuffer )
{
    AudioBuffer* sampleBuffer = new AudioBuffer( aChannelAmount, aBufferLength );

    int i = 0;

    // get a pointer to the Java array
    jdouble *c_array;
    c_array = getEnvironment()->GetDoubleArrayElements( aBuffer, 0 );

    // exception checking
    if ( c_array == NULL )
        return;

    // copy buffer contents
    float* channelBuffer = sampleBuffer->getBufferForChannel( 0 );

    for ( i = 0; i < aBufferLength; i++ )
        channelBuffer[ i ] = ( float ) c_array[ i ];

    // release the memory so Java can have it again
    getEnvironment()->ReleaseDoubleArrayElements( aBuffer, c_array, 0 );

    // stereo ?

    if ( aChannelAmount == 2 )
    {
        c_array = getEnvironment()->GetDoubleArrayElements( aOptRightBuffer, 0 );

        // exception checking
        if ( c_array == NULL )
            return;

        // copy buffer contents
        channelBuffer = sampleBuffer->getBufferForChannel( 1 );

        for ( i = 0; i < aBufferLength; i++ )
            channelBuffer[ i ] = ( float ) c_array[ i ];

        // release the memory so Java can have it again
        getEnvironment()->ReleaseDoubleArrayElements( aOptRightBuffer, c_array, 0 );
    }

    // convert jstring to std::string
    const char *s = getEnvironment()->GetStringUTFChars( aKey, NULL );
    std::string theKey = s;
    getEnvironment()->ReleaseStringUTFChars( aKey, s );

    setSample( theKey, sampleBuffer );
}

AudioBuffer* SampleManager::getSample( std::string aIdentifier )
{
    std::map<std::string, cachedSample>::iterator it = SampleManagerSamples::_sampleMap.find( aIdentifier );
    return it->second.sampleBuffer; // key stored in first, value stored in second
}

AudioBuffer* SampleManager::getSample( jstring aIdentifier )
{
    // convert jstring to std::string
    const char *s = getEnvironment()->GetStringUTFChars( aIdentifier, NULL );
    std::string theKey = s;
    getEnvironment()->ReleaseStringUTFChars( aIdentifier, s );

    return getSample( theKey );
}

int SampleManager::getSampleLength( std::string aIdentifier )
{
    std::map<std::string, cachedSample>::iterator it = SampleManagerSamples::_sampleMap.find( aIdentifier );
    return it->second.sampleLength; // key stored in first, value stored in second
}

bool SampleManager::hasSample( std::string aIdentifier )
{
    std::map<std::string, cachedSample>::iterator it = SampleManagerSamples::_sampleMap.find( aIdentifier );
    return ( it != SampleManagerSamples::_sampleMap.end());
}

bool SampleManager::hasSample( jstring aIdentifier )
{
// convert jstring to std::string
    const char *s = getEnvironment()->GetStringUTFChars( aIdentifier, NULL );
    std::string theKey = s;
    getEnvironment()->ReleaseStringUTFChars( aIdentifier, s );

    return hasSample( theKey );
}

void SampleManager::removeSample( std::string aIdentifier )
{
    if ( hasSample( aIdentifier ))
    {
        std::map<std::string, cachedSample>::iterator it = SampleManagerSamples::_sampleMap.find( aIdentifier );
        delete it->second.sampleBuffer;  // clear the buffer pointers to release memory
        SampleManagerSamples::_sampleMap.erase( SampleManagerSamples::_sampleMap.find( aIdentifier ));
    }
}

void SampleManager::flushSamples()
{
    // TODO: not actually deleting buffers and thus leaking memory
    SampleManagerSamples::_sampleMap.clear();
}
