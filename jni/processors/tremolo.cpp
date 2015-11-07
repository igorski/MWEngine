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

Tremolo::Tremolo( int aWaveForm, float aFrequency, bool aStereo )
{
    _table = new WaveTable( WAVE_TABLE_PRECISION, aFrequency );

    setWaveForm ( aWaveForm );
    setFrequency( aFrequency );
    setStereo   ( aStereo );
}

Tremolo::~Tremolo()
{
    delete _table;
}

/* public methods */

int Tremolo::getWaveForm()
{
    return _waveform;
}

void Tremolo::setWaveForm( int aWaveForm )
{
    _waveform = aWaveForm;
    TablePool::getTable( _table, _waveform );
}

float Tremolo::getFrequency()
{
    return _table->getFrequency();
}

void Tremolo::setFrequency( float aFrequency )
{
    _table->setFrequency( aFrequency );
}

bool Tremolo::getStereo()
{
    return _stereo;
}

void Tremolo::setStereo( bool aStereo )
{
    _stereo = aStereo;
}

void Tremolo::process( AudioBuffer* sampleBuffer, bool isMonoSource )
{
    int bufferSize = sampleBuffer->bufferSize;
    bool doStereo  = getStereo() && sampleBuffer->amountOfChannels > 1;

    for ( int c = 0, ca = sampleBuffer->amountOfChannels; c < ca; ++c )
    {
        SAMPLE_TYPE* channelBuffer     = sampleBuffer->getBufferForChannel( c );
        SAMPLE_TYPE* nextChannelBuffer = ( doStereo && ( c + 1 ) < ca ) ? sampleBuffer->getBufferForChannel( c + 1 ) : 0;
        SAMPLE_TYPE sample;

        for ( int i = 0; i < bufferSize; ++i )
        {
            sample = channelBuffer[ i ] * _table->peek();

            if ( doStereo && nextChannelBuffer != 0 )
            {
                if ( sample < 0 ) {
                    // pan left
                    channelBuffer[ i ]      = sample;
                    nextChannelBuffer[ i ] = 0.0;
                }
                else {
                    // pan right
                    channelBuffer[ i ]      = 0.0;
                    nextChannelBuffer[ i ] = sample;
                }
            }
            else {
                channelBuffer[ i ] = sample;
            }
        }

        // save CPU cycles when source and output are mono
        if ( isMonoSource && !doStereo )
        {
            sampleBuffer->applyMonoSource();
            break;
        }
        else if ( doStereo )
            ++c;    // extra increment as next buffer has already been filled
    }
}
