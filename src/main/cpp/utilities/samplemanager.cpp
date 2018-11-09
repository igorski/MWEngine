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
#include "samplemanager.h"

namespace SampleManagerSamples
{
    std::map<std::string, cachedSample> _sampleMap;
}

/* public methods */

void SampleManager::setSample( std::string aIdentifier, AudioBuffer* aBuffer, unsigned int sampleRate )
{
    cachedSample sample = { aBuffer->bufferSize, sampleRate, aBuffer };

    // Assignment using member function insert() and STL pair
    SampleManagerSamples::_sampleMap.insert( std::pair<std::string, cachedSample>( aIdentifier, sample ));
}

AudioBuffer* SampleManager::getSample( std::string aIdentifier )
{
    if ( !hasSample( aIdentifier ))
        return nullptr;

    std::map<std::string, cachedSample>::iterator it = SampleManagerSamples::_sampleMap.find( aIdentifier );

    // key stored in first, value stored in second
    return it->second.sampleBuffer;
}

int SampleManager::getSampleLength( std::string aIdentifier )
{
    if ( !hasSample( aIdentifier ))
        return 0;

    std::map<std::string, cachedSample>::iterator it = SampleManagerSamples::_sampleMap.find( aIdentifier );

    // key stored in first, value stored in second
    return it->second.sampleLength;
}

int SampleManager::getSampleRateForSample( std::string aIdentifier )
{
    if ( !hasSample( aIdentifier ))
        return AudioEngineProps::SAMPLE_RATE;

    std::map<std::string, cachedSample>::iterator it = SampleManagerSamples::_sampleMap.find( aIdentifier );

    // key stored in first, value stored in second
    return it->second.sampleRate;
}

bool SampleManager::hasSample( std::string aIdentifier )
{
    std::map<std::string, cachedSample>::iterator it = SampleManagerSamples::_sampleMap.find( aIdentifier );
    return ( it != SampleManagerSamples::_sampleMap.end());
}

void SampleManager::removeSample( std::string aIdentifier, bool free )
{
    if ( hasSample( aIdentifier ))
    {
        std::map<std::string, cachedSample>::iterator it = SampleManagerSamples::_sampleMap.find( aIdentifier );

        if ( free )
            delete it->second.sampleBuffer;

        SampleManagerSamples::_sampleMap.erase( SampleManagerSamples::_sampleMap.find( aIdentifier ));
    }
}

void SampleManager::flushSamples()
{
    // invoke destructors on all AudioBuffers

    std::map<std::string, cachedSample>::iterator it;

    for ( it  = SampleManagerSamples::_sampleMap.begin();
          it != SampleManagerSamples::_sampleMap.end(); ++it )
    {
        delete it->second.sampleBuffer;
    }
    SampleManagerSamples::_sampleMap.clear();
}
