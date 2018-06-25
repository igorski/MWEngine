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
#include "lfo.h"
#include "../global.h"
#include "../audioengine.h"
#include <definitions/waveforms.h>
#include <generators/wavegenerator.h>
#include <utilities/utils.h>
#include <utilities/tablepool.h>
#include <utilities/waveutil.h>

// constructor / destructor

LFO::LFO()
{
    _wave  = WaveForms::SILENCE;
    _rate  = MIN_RATE();
    _table = new WaveTable( WAVE_TABLE_PRECISION, _rate );

    _depth   = 1.f;
    _min     = 0.f;
    _max     = 1.f;
    _range   = _max - _min;

    _cvalue = 1.f;
    _cmin   = 0.f;
    _cmax   = _cvalue;
}

LFO::~LFO()
{
    delete _table;
}

/* public methods */

float LFO::getRate()
{
    return _rate;
}

void LFO::setRate( float value )
{
    _rate = std::min( MAX_RATE(), std::max( MIN_RATE(), value ));
    _table->setFrequency( _rate );
}

int LFO::getWave()
{
    return _wave;
}

void LFO::setWave( int value )
{
    if ( _wave == value || value == WaveForms::SILENCE )
        return;

    _wave = value;

    generate();
}

float LFO::getDepth()
{
    return _depth;
}

void LFO::setDepth( float value )
{
    _depth = value;
    cacheProperties( _cvalue, _cmin, _cmax );
}

void LFO::generate()
{
    if ( _wave == WaveForms::SILENCE )
        return;

    // we register the table by stringifying the waveform enum

    WaveTable* table = TablePool::getTable( SSTR( _wave ));

    if ( table == 0 ) {
        // no table in pool, LFO will generate table inline
        WaveGenerator::generate( _table, _wave );
    }
    else {
        _table->cloneTable( table );
    }

    // ensure tables are unipolar for easy lookup

    if ( WaveUtil::isBipolar( _table->getBuffer(), _table->tableLength )) {
        WaveUtil::toUnipolar( _table->getBuffer(), _table->tableLength );
    }
}

WaveTable* LFO::getTable()
{
    return _table;
}

void LFO::cacheProperties( float value, float min, float max )
{
    _cvalue = value;
    _cmin   = min;
    _cmax   = max;

    _range = value * _depth;
    _max   = std::min( max, value + _range / 2.f );
    _min   = std::max( min, value - _range / 2.f );
}
