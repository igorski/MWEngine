/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2016 Igor Zinken - http://www.igorski.nl
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

    SR_OVER_LENGTH = ( SAMPLE_TYPE ) AudioEngineProps::SAMPLE_RATE / ( SAMPLE_TYPE ) tableLength;
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

void WaveTable::setBuffer( SAMPLE_TYPE* aBuffer )
{
    if ( _buffer != nullptr )
        delete[] _buffer;

    _buffer = aBuffer;
}

void WaveTable::cloneTable( WaveTable* waveTable )
{
    if ( tableLength != waveTable->tableLength )
    {
        delete[] _buffer;
        tableLength = waveTable->tableLength;
        _buffer = BufferUtility::generateSilentBuffer( tableLength );
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
