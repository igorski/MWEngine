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
#ifndef __BUFFER_UTILITY_H_INCLUDED__
#define __BUFFER_UTILITY_H_INCLUDED__

/**
 * BufferUtility provides several functions
 * for basic time / BPM related calculations
 * where musical concepts are translated into buffers
 */
class BufferUtility
{
    public:

        static double getBPMbyLength( double length, int amountOfBars );
        static double getBPMbySamples( int length, int amountOfBars, int sampleRate );
        static int bufferToMilliseconds( int bufferSize, int sampleRate );
        static int millisecondsToBuffer( int milliSeconds, int sampleRate );
        static int getBitRate( int sampleRate, int bitDepth, int channels );
        static int calculateSamplesPerBeatDivision( int sampleRate, double tempo, int subdivision );
        static int getSamplesPerBeat( int sampleRate, double tempo );
        static int getSamplesPerBar( int sampleRate, double tempo, int beatAmount, int beatUnit );
};

#endif