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

namespace MWEngine {

/* public methods */

int BufferUtility::bufferToMilliseconds( int bufferSize, int sampleRate )
{
    return bufferSize / ( sampleRate / 1000 );
}

float BufferUtility::bufferToSeconds( int bufferSize, int sampleRate )
{
    return ( float ) bufferSize / ( float ) sampleRate;
}

int BufferUtility::millisecondsToBuffer( int milliSeconds, int sampleRate )
{
    return ( int ) ( milliSeconds * ( sampleRate / 1000 ));
}

int BufferUtility::secondsToBuffer( float seconds, int sampleRate )
{
    return ( int ) ( seconds * sampleRate );
}

int BufferUtility::getBitRate( int sampleRate, int bitDepth, int channels )
{
    return sampleRate * bitDepth * channels;
}

int BufferUtility::calculateSamplesPerBeatDivision( int sampleRate, double tempo, int subdivision )
{
    int samplesPerBar = getSamplesPerBar( sampleRate, tempo, 4, 4 );
    return samplesPerBar / subdivision;
}

int BufferUtility::getSamplesPerBeat( int sampleRate, double tempo )
{
    return ( int ) (( sampleRate * 60 ) / tempo );
}

int BufferUtility::getSamplesPerBar( int sampleRate, double tempo, int beatAmount, int beatUnit )
{
    int samplesPerDoubleFourTime = getSamplesPerBeat( sampleRate, tempo ) * 4;
    return samplesPerDoubleFourTime / beatUnit * beatAmount;
}

int BufferUtility::calculateBufferLength( SAMPLE_TYPE aMinRate )
{
    SAMPLE_TYPE phaseStep = aMinRate / AudioEngineProps::SAMPLE_RATE;
    return ( int ) ceil( 1.0 / phaseStep );
}

int BufferUtility::calculateBufferLength( int milliSeconds )
{
    return milliSeconds * ( AudioEngineProps::SAMPLE_RATE / 1000 );
}

/* beat calculation */

double BufferUtility::getBPMbyLength( double length, int amountOfBars )
{
    // length to seconds
    length = length / 1000;

    return 240.0 / ( length / amountOfBars );
}

double BufferUtility::getBPMbySamples( int length, int amountOfBars, int sampleRate )
{
    return 240.0 / (( length / amountOfBars ) / sampleRate );
}

/* buffer generation */

std::vector<SAMPLE_TYPE*>* BufferUtility::createSampleBuffers( int amountOfChannels, int bufferSize )
{
    std::vector<SAMPLE_TYPE*>* buffers = new std::vector<SAMPLE_TYPE*>( amountOfChannels );

    // fill buffers with silence

    for ( int i = 0; i < amountOfChannels; ++i ) {
        buffers->at( i ) = BufferUtility::generateSilentBuffer( bufferSize );
    }
    return buffers;
}

SAMPLE_TYPE* BufferUtility::generateSilentBuffer( int bufferSize )
{
    SAMPLE_TYPE* out = new SAMPLE_TYPE[ bufferSize ];
    memset( out, 0, bufferSize * sizeof( SAMPLE_TYPE )); // zero bits should equal 0.0f

    return out;
}

void BufferUtility::bufferToFile( const char* fileName, SAMPLE_TYPE* buffer, int bufferSize )
{
    std::ofstream myFile;

    myFile.open( fileName );
    myFile << "{ ";

    for ( int i = 0; i < bufferSize; ++i )
    {
        SAMPLE_TYPE tmp = buffer[ i ];
        myFile << tmp;

        if ( i < ( bufferSize - 1 ))
            myFile << ", ";
    }
    myFile << " } ";
    myFile << std::endl;

    myFile.close();
}

} // E.O namespace MWEngine
