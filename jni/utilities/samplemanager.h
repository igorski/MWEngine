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
#ifndef __SAMPLEMANAGER_H_INCLUDED__
#define __SAMPLEMANAGER_H_INCLUDED__

#include "audiobuffer.h"
#include <string>
#include <map>
#include <utility>

typedef struct
{
   int sampleLength;
   AudioBuffer* sampleBuffer;
} cachedSample;

/**
 * SampleManager is a static class that
 * holds a map of Sampled audio, it can be
 * used to pool samples that are to be used
 * repeatedly or simultaneously, etc...
 */
class SampleManager
{
    public:

        static void setSample( std::string aKey, AudioBuffer* aBuffer );

        static AudioBuffer* getSample( std::string aIdentifier );
        static int getSampleLength( std::string aIdentifier );

        static bool hasSample( std::string aIdentifier );
        static void removeSample( std::string aIdentifier );
        static void flushSamples();
};

namespace SampleManagerSamples
{
    extern std::map<std::string, cachedSample> _sampleMap;
}

#endif
