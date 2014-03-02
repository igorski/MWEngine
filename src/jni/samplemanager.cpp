#include "samplemanager.h"
#include "java_bridge.h"

namespace SampleManagerSamples
{
    std::map<std::string, cachedSample> _sampleMap;
}

/* public methods */

void SampleManager::setSample( std::string aKey, float* aBuffer, int aBufferLength )
{
    cachedSample sample = { aBufferLength, aBuffer };

    // Assignment using member function insert() and STL pair
    SampleManagerSamples::_sampleMap.insert( std::pair<std::string, cachedSample>( aKey, sample ));
}

// TODO: doesn't really belong here... move to Java bridge ?
void SampleManager::setSample( jstring aKey, jfloatArray aBuffer, jint aBufferLength )
{
    float* sampleBuffer = BufferUtil::generateSilentBuffer( aBufferLength );

    int i = 0;

    // get a pointer to the Java array
    jfloat *c_array;
    c_array = getEnvironment()->GetfloatArrayElements( aBuffer, 0 );

    // exception checking
    if ( c_array == NULL )
        return;

    // copy buffer contents
    for ( i = 0; i < aBufferLength; i++ ) {
        sampleBuffer[ i ] = c_array[ i ];
    }
    // release the memory so Java can have it again
    getEnvironment()->ReleasefloatArrayElements( aBuffer, c_array, 0 );

    // convert jstring to std::string
    const char *s = getEnvironment()->GetStringUTFChars( aKey,NULL );
    std::string theKey = s;
    getEnvironment()->ReleaseStringUTFChars( aKey, s );

    setSample( theKey, sampleBuffer, ( int ) aBufferLength );
}

float* SampleManager::getSample( std::string aIdentifier )
{
    std::map<std::string, cachedSample>::iterator it = SampleManagerSamples::_sampleMap.find( aIdentifier );
    return it->second.sampleBuffer; // key stored in first, value stored in second
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
    // TODO: not actually deleting buffers and leaking memory
    SampleManagerSamples::_sampleMap.clear();
}
