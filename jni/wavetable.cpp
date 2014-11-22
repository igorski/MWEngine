/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Igor Zinken - http://www.igorski.nl
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
#include "wavetable.h"
#include <utilities/bufferutility.h>

/* constructor / destructor */

WaveTable::WaveTable( int aTableLength, float aFrequency )
{
    tableLength  = aTableLength;
    _accumulator = 0.0;
    _buffer      = BufferUtility::generateSilentBuffer( tableLength );
    setFrequency( aFrequency );

    SR_OVER_LENGTH = AudioEngineProps::SAMPLE_RATE / ( SAMPLE_TYPE ) tableLength;
}

WaveTable::~WaveTable()
{
    delete[] _buffer;
}

/* public methods */

void WaveTable::setFrequency( float aFrequency )
{
    _frequency = aFrequency;
}

float WaveTable::getFrequency()
{
    return _frequency;
}

bool WaveTable::hasContent()
{
    for ( int i = 0; i < tableLength; ++i )
    {
        if ( _buffer[ i ] != 0.0 )
            return true;
    }
    return false;
}

void WaveTable::setAccumulator( SAMPLE_TYPE value )
{
    _accumulator = value;
}

SAMPLE_TYPE WaveTable::getAccumulator()
{
    return _accumulator;
}

SAMPLE_TYPE* WaveTable::getBuffer()
{
    return _buffer;
}

/**
 * returns a value from the wave table for the current
 * accumulator position, this method also increments
 * the accumulator (and keeps it within bounds)
 *
 * you can "peek" at the sample-level when working with
 * the mixing of AudioBuffers
 */
SAMPLE_TYPE WaveTable::peek()
{
    int readOffset = 0;   // the wave table offset to read from

    if ( _accumulator == 0 )
        readOffset = 0;
    else
        readOffset = ( int ) ( _accumulator / SR_OVER_LENGTH );

    // increment the accumulators read offset
    _accumulator += _frequency;

    // keep the accumulator in the bounds of the sample frequency
    if ( _accumulator > AudioEngineProps::SAMPLE_RATE )
        _accumulator -= AudioEngineProps::SAMPLE_RATE;

    // return the sample present at the calculated offset within the table
    return _buffer[ readOffset ];
}

void WaveTable::cloneTable( WaveTable* waveTable )
{
    if ( tableLength != waveTable->tableLength )
    {
        delete _buffer;
        _buffer = BufferUtility::generateSilentBuffer( tableLength );
        tableLength = waveTable->tableLength;
    }
    for ( int i = 0; i < tableLength; ++i )
        _buffer[ i ] = waveTable->_buffer[ i ];
}

WaveTable* WaveTable::clone()
{
    WaveTable* out    = new WaveTable( tableLength, _frequency );
    out->_accumulator = _accumulator;
    out->cloneTable( this );

    return out;
}
