/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Igor Zinken - http://www.igorski.nl
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
#include "tremolo.h"
#include <utilities/tablepool.h>

/* constructors / destructor */

Tremolo::Tremolo( int aLeftWaveForm, int aRightWaveForm, float aLeftFrequency, float aRightFrequency )
{
    // create stereo tables
    for ( int i = 0; i < 2; ++i )
        _tables[ i ] = new WaveTable( WAVE_TABLE_PRECISION, ( i == 0 ) ? aLeftFrequency : aRightFrequency );

    setWaveFormForChannel ( 0, aLeftWaveForm );
    setFrequencyForChannel( 0, aLeftFrequency );
    setWaveFormForChannel ( 1, aRightWaveForm );
    setFrequencyForChannel( 1, aRightFrequency );
}

Tremolo::~Tremolo()
{
    for ( int i = 0; i < 2; ++i )
        delete _tables[ i ];
}

/* public methods */

int Tremolo::getWaveFormForChannel( int aChannelNum )
{
    return _waveforms[ aChannelNum ];
}

void Tremolo::setWaveFormForChannel( int aChannelNum, int aWaveForm )
{
    _waveforms[ aChannelNum ] = aWaveForm;
    // TODO : USE ENVELOPE GENERATOR!!!
    TablePool::getTable( getTableForChannel( aChannelNum ), aWaveForm );
}

float Tremolo::getFrequencyForChannel( int aChannelNum )
{
    return getTableForChannel( aChannelNum )->getFrequency();
}

void Tremolo::setFrequencyForChannel( int aChannelNum, float aFrequency )
{
    getTableForChannel( aChannelNum )->setFrequency( aFrequency );
}

WaveTable* Tremolo::getTableForChannel( int aChannelNum )
{
    return _tables[ aChannelNum ];
}

bool Tremolo::isStereo()
{
    if ( _waveforms[ 0 ] != _waveforms[ 1 ])
        return true;

    return getFrequencyForChannel( 0 ) != getFrequencyForChannel( 1 );
}

void Tremolo::process( AudioBuffer* sampleBuffer, bool isMonoSource )
{
    int bufferSize        = sampleBuffer->bufferSize;
    bool doStereo         = isStereo() && sampleBuffer->amountOfChannels > 1;
    WaveTable* leftTable  = getTableForChannel( 0 );
    WaveTable* rightTable = getTableForChannel( 1 );

    // in case sampleBuffer has more than 2 output channels we most
    // store and restore the accumulators for every additional channel

    int orgLeftAccumulator  = leftTable->getAccumulator();
    int orgRightAccumulator = rightTable->getAccumulator();

    for ( int c = 0, ca = sampleBuffer->amountOfChannels; c < ca; ++c )
    {
        SAMPLE_TYPE* channelBuffer     = sampleBuffer->getBufferForChannel( c );
        SAMPLE_TYPE* nextChannelBuffer = ( doStereo && ( c + 1 ) < ca ) ? sampleBuffer->getBufferForChannel( c + 1 ) : 0;

        for ( int i = 0; i < bufferSize; ++i )
        {
            if ( doStereo && nextChannelBuffer != 0 )
            {
                channelBuffer[ i ]     *= leftTable->peek();
                nextChannelBuffer[ i ] *= rightTable->peek();
            }
            else {
                channelBuffer[ i ] *= leftTable->peek();
            }
        }

        // save CPU cycles when source and output are mono
        if ( isMonoSource && !doStereo )
        {
            sampleBuffer->applyMonoSource();
            break;
        }
        else if ( doStereo )
        {
            // here we restore the accumulators for the additional channels, note the additional
            // increment as the next buffer has already been filled

            int nextChannelNum = ( ++c + 1 );

            // if the amount of channels remaining for processing is an even
            // number, also reset the right table

            if ( nextChannelNum >= 2 && nextChannelNum < ca )
            {
                leftTable->setAccumulator( orgLeftAccumulator );

                if (( ca - nextChannelNum ) % 2 == 0 )
                    rightTable->setAccumulator( orgRightAccumulator );
            }
        }
    }
}
