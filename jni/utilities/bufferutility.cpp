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
#include <utilities/bufferutility.h>
#include <cstring>
#include <sstream>
#include <fstream>
#include <iostream>

/* public methods */

/**
 * get the tempo in Beats per Minute from the length
 * in milliseconds of a audio snippet
 *
 * @param length       {double} length in milliseconds
 * @param amountOfBars {int} the amount of bars the snippet lasts
 */
double BufferUtility::getBPMbyLength( double length, int amountOfBars )
{
    // length to seconds
    length = length / 1000;

    return 240.0 / ( length / amountOfBars );
}

/**
 * get the tempo in Beats Per Minute by the length in
 * samples of a audio snippet
 *
 * @param length       {int} length in samples
 * @param amountOfBars {int} the amount of bars the snippet lasts
 * @param sampleRate   {int} sampleRate in Hz
 */
double BufferUtility::getBPMbySamples( int length, int amountOfBars, int sampleRate )
{
     return 240.0 / (( length / amountOfBars ) / sampleRate );
}

/**
 * translates the size of a given buffer to its duration in milliseconds
 *
 * @param {int} bufferSize
 * @param {int} sampleRate in Hz
 */
int BufferUtility::bufferToMilliseconds( int bufferSize, int sampleRate )
{
    return ( int ) ( bufferSize / ( sampleRate / 1000 ));
}

/**
 * translates the size of a given buffer to its duration in seconds
 *
 * @param {int} bufferSize
 * @param {int} sampleRate in Hz
 */
float BufferUtility::bufferToSeconds( int bufferSize, int sampleRate )
{
    return ( float ) bufferSize / ( float ) sampleRate;
}

/**
 * return the required buffer size for a value in milliseconds
 *
 * @param {int} milliSeconds
 * @param {int} sampleRate in Hz
 */
int BufferUtility::millisecondsToBuffer( int milliSeconds, int sampleRate )
{
    return ( int ) ( milliSeconds * ( sampleRate / 1000 ));
}

/**
 * return the required buffer size for a value in seconds
 *
 * @param {float} seconds
 * @param {int} sampleRate in Hz
 */
int BufferUtility::secondsToBuffer( float seconds, int sampleRate )
{
    return ( int ) ( seconds * sampleRate );
}

/**
 * calculate the bitRate of a given audiostream
 *
 * @param sampleRate  {int} sampleRate in Hz
 * @param bitDepth    {int} bit depth
 * @param channels    {int} the amount of audio channels
 */
int BufferUtility::getBitRate( int sampleRate, int bitDepth, int channels )
{
    return sampleRate * bitDepth * channels;
}

/**
 * calculations within a musical context:
 * calculate the sample length required for each
 * beat at a given tempo at a given bar subdivision
 *
 * @param tempo       {double} tempo in BPM
 * @param sampleRate  {int} the sample rate in Hz
 * @param subdivision {int} the desired subdivision of the measure, i.e.
 *                    1 = full measure, 2 = half measure, 4 = quaver,
 *                    8 = 8th note, 16 = sixteenth note, 32 = 32nd note, etc.
 */
int BufferUtility::calculateSamplesPerBeatDivision( int sampleRate, double tempo, int subdivision )
{
    int samplesPerBar = getSamplesPerBar( sampleRate, tempo, 4, 4 );
    return ( int ) ( samplesPerBar / subdivision );
}

/**
 * calculate the amount of samples necessary to hold a buffer
 * representing a single beat at the given sample rate and tempo
 *
 * @param sampleRate {int} the sample rate in Hz
 * @param tempo      {double} tempo in BPM
 */
int BufferUtility::getSamplesPerBeat( int sampleRate, double tempo )
{
    return ( int ) (( sampleRate * 60 ) / tempo );
}

/**
 * calculate the amount of samples necessary to hold a buffer
 * representing a full bar/measure at the given sample rate, tempo and time signature
 *
 * @param sampleRate {int} the sample rate in Hz
 * @param tempo      {double} tempo in BPM
 * @param beatAmount {int} upper numeral in time signature (i.e. the "3" in 3/4)
 * @param beatUnit   {int} lower numeral in time signature (i.e. the "4" in 3/4)
 */
int BufferUtility::getSamplesPerBar( int sampleRate, double tempo, int beatAmount, int beatUnit )
{
    int samplesPerDoubleFourTime = getSamplesPerBeat( sampleRate, tempo ) * 4;
    return samplesPerDoubleFourTime / beatUnit * beatAmount;
}

/**
 * calculate the amount of samples a single cycle of a waveform
 * will hold at a give rate in Hz, at the current sample rate
 *
 * @param aMinRate {SAMPLE_TYPE} length in Hz for the waveform
 */
int BufferUtility::calculateBufferLength( SAMPLE_TYPE aMinRate )
{
    SAMPLE_TYPE phaseStep = aMinRate / AudioEngineProps::SAMPLE_RATE;
    return ceil( 1.0 / phaseStep );
}

/**
 * calculate the amount of samples necessary for
 * writing the given amount in milliseconds
 */
int BufferUtility::calculateBufferLength( int milliSeconds )
{
    return milliSeconds * ( AudioEngineProps::SAMPLE_RATE / 1000 );
}

std::vector<SAMPLE_TYPE*>* BufferUtility::createSampleBuffers( int amountOfChannels, int bufferSize )
{
    std::vector<SAMPLE_TYPE*>* buffers = new std::vector<SAMPLE_TYPE*>( amountOfChannels );

    // fill buffers with silence

    for ( int i = 0; i < amountOfChannels; ++i ) {
        buffers->at( i ) = BufferUtility::generateSilentBuffer( bufferSize );
    }
    return buffers;
}

/**
 * creates a new buffer of given aBufferSize in length
 * containing nothing but silence
 */
SAMPLE_TYPE* BufferUtility::generateSilentBuffer( int aBufferSize )
{
    SAMPLE_TYPE* out = new SAMPLE_TYPE[ aBufferSize ];
    memset( out, 0, aBufferSize * sizeof( SAMPLE_TYPE )); // zero bits should equal 0.0f

    return out;
}

/**
 * write the contents of a buffer
 * into a file onto the file system
 */
void BufferUtility::bufferToFile( const char* aFileName, SAMPLE_TYPE* aBuffer, int aBufferSize )
{
    std::ofstream myFile;

    myFile.open( aFileName );
    myFile << "{ ";

    for ( int i = 0; i < aBufferSize; ++i )
    {
        SAMPLE_TYPE tmp = aBuffer[ i ];
        myFile << tmp;

        if ( i < ( aBufferSize - 1 ))
            myFile << ", ";
    }
    myFile << " } ";
    myFile << std::endl;

    myFile.close();
}
