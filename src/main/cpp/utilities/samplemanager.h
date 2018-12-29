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
#ifndef __MWENGINE__SAMPLEMANAGER_H_INCLUDED__
#define __MWENGINE__SAMPLEMANAGER_H_INCLUDED__

#include "audiobuffer.h"
#include <string>
#include <map>
#include <utility>

namespace MWEngine {

typedef struct
{
   int sampleLength;
   unsigned int sampleRate;
   AudioBuffer* sampleBuffer;
} cachedSample;

/**
 * SampleManager is a static class that holds a map of Sampled audio, it can be
 * used to pool samples (AudioBuffers) that are to be used repeatedly or simultaneously, etc...
 * SampleManager will also manage the memory, when samples can be removed you can do
 * so via SampleManager which will in turn release the memory allocated by the AudioBuffers
 */
class SampleManager
{
    public:

        // store given AudioBuffer under given identifier name in this SampleManager
        static void setSample( std::string aIdentifier, AudioBuffer* aBuffer, unsigned int sampleRate );

        // retrieve AudioBuffer registered under given identifier from this SampleManager
        // returns 0 if no associated AudioBuffer is found
        static AudioBuffer* getSample( std::string aIdentifier );

        // retrieve the length (in samples) of the AudioBuffer registered under given
        // identifier, returns 0 if no associated AudioBuffer is found
        static int getSampleLength( std::string aIdentifier );

        // retrieve the sample rate of the AudioBuffer registered under given
        // identifier, returns audio engine's sample rate if no associated AudioBuffer is found
        static int getSampleRateForSample( std::string aIdentifier );

        // queries whether the SampleManager has an AudioBuffer registered under given identifier
        static bool hasSample( std::string aIdentifier );

        // remove the sample from the SampleManager, if free is true, the sample will also be deleted
        static void removeSample( std::string aIdentifier, bool free );
        static void flushSamples();
};

namespace SampleManagerSamples
{
    extern std::map<std::string, cachedSample> _sampleMap;
}

} // E.O namespace MWEngine

#endif
