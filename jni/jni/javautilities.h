/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2016 Igor Zinken - http://www.igorski.nl
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
#ifndef JAVAUTILITIES_H_INCLUDED
#define JAVAUTILITIES_H_INCLUDED

#include "javabridge.h"

/**
 * JavaUtilities provide convenient hooks into the engine to
 * register sample buffers directly into the SampleManager or TablePool
 * without having to use AudioBuffers (these should preferably remain a native layer component)
 */
class JavaUtilities
{
    public:

        // creates an AudioBuffer from a given WAV file and stores it inside
        // the SampleManager under given key "aKey"

        static bool createSampleFromFile( jstring aKey, jstring aWAVFilePath );

        // creates an AudioBuffer from a resource and stores it inside
        // the SampleManager under given key "aKey"

        static bool createSampleFromResource( jstring aKey, jstring aAssetPath, jint aAssetOffset, jint aAssetLength );

        // creates an AudioBuffer from given sample buffers and stores it inside
        // the SampleManager under given key "aKey", aOptRightBuffer can be null

        static void createSampleFromBuffer( jstring aKey, jint aBufferLength, jint aChannelAmount,
                                            jdoubleArray aBuffer, jdoubleArray aOptRightBuffer );

        // cache given sample buffers in the TablePool for given identifiers and waveform types

        static void cacheTable( jint tableLength, jint waveformType );
        static void cacheTable( jint tableLength, jint waveformType, jdoubleArray aBuffer );

        // take the raw contents of a WAV file at given path and
        // convert it to a WaveTable to be stored under given waveformType identifier inside the TablePool

        static bool createTableFromFile( jint waveformType, jstring aWAVFilePath );
};

#endif
