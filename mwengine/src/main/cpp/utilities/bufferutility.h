/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2022 Igor Zinken - https://www.igorski.nl
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
#ifndef __MWENGINE__BUFFER_UTILITY_H_INCLUDED__
#define __MWENGINE__BUFFER_UTILITY_H_INCLUDED__

#include "global.h"
#include <vector>
#include <audiobuffer.h>

namespace MWEngine {
class BufferUtility
{
    public:

        /* size calculation */

        /**
         * Returns the time in milliseconds a buffer of given bufferSize lasts for given sampleRate.
         */
        static int bufferToMilliseconds( int bufferSize, int sampleRate );

        /**
         * The reverse of bufferToMilliseconds. Calculates the required buffer size to represent
         * given milliSeconds at given sampleRate.
         */
        static int millisecondsToBuffer( int milliSeconds, int sampleRate );

        /**
         * Returns the time in seconds a buffer of given bufferSize lasts for given sampleRate.
         */
        static float bufferToSeconds( int bufferSize, int sampleRate );

        /**
         * The reverse of bufferToSeconds. Calculates the required buffer size to represent given
         * seconds at given sampleRate.
         */
        static int secondsToBuffer( float seconds, int sampleRate );

        /**
         * Returns the bit rate for an audio stream at given sampleRate, bitDepth and channel amount.
         */
        static int getBitRate( int sampleRate, int bitDepth, int channels );

        /**
         * Calculates the amount of samples a single cycle of a waveform will require at given
         * minRate (in Hertz), for the current engine sample rate.
         */
        static int calculateBufferLength( SAMPLE_TYPE aMinRate );

        /**
         * Calculates the amount of samples necessary to represent given time in milliSeconds, for
         * the current engine sample rate.
         */
        static int calculateBufferLength( int milliSeconds );

        /**
         * Calculates the amount of samples necessary to render audio at the duration of requested
         * subdivision at the current sampleRate and tempo, where subdivision is a number by which
         * to divide a measure (e.g. 1 = full meausure, 2 = half measure, 4 = quaver,
         * 8 = eight note, 16 = sixteenth note, etc.)
         */
        static int calculateSamplesPerBeatDivision( int sampleRate, double tempo, int subdivision );

        /**
         * Calculates the amount of samples necessary to render audio lasting a single beat for the
         * given tempo at the given sampleRate.
         */
        static int getSamplesPerBeat( int sampleRate, double tempo );

        /**
         * Calculates the amount of samples necessary to render audio lasting a single measure for
         * the given tempo at the given sampleRate.
         */
        static int getSamplesPerBar( int sampleRate, double tempo, int beatAmount, int beatUnit );

        /* beat calculation */

        /**
         * When given length represents a length in milliseconds and amountOfBars represents the
         * known amount of measures the snippet represents, the resulting value is the tempo in BPM.
         */
        static double getBPMbyLength( double length, int amountOfBars );

        /**
         * The same as getBPMbyLength() except that given length represents the size of a buffer
         * in samples and sampleRate the known sample rate of the snippet.
         */
        static double getBPMbySamples( int length, int amountOfBars, int sampleRate );

        /* buffer generation */

        /**
         * Returns a vector of given amountOfChannels in size, containing silent audio buffers
         * at given bufferSize in length.
         */
        static std::vector<SAMPLE_TYPE*>* createSampleBuffers( int amountOfChannels, int bufferSize );

        /**
         * Creates a silent audio (0.0f) buffer at given bufferSize in length.
         */
        static SAMPLE_TYPE* generateSilentBuffer( int bufferSize );

        /**
         * Writes the contents of given buffer onto storage under given fileName.
         * The result is a comma separated list of SAMPLE_TYPE values.
         */
        static void bufferToFile( const char* filename, const SAMPLE_TYPE* buffer, int bufferLength );

        /**
         * Writes the contents of given sourceBuffer into a one dimensional float array
         * representing interleaved sample format
         */
        static void mixBufferInterleaved( AudioBuffer* sourceBuffer, float* bufferToMixInto, int amountOfSamples, int outputChannels );
};
} // E.O namespace MWEngine

#endif
