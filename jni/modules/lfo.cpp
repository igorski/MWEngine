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
#include "lfo.h"
#include "../global.h"
#include "../audioengine.h"
#include <definitions/waveforms.h>
#include <utilities/utils.h>
#include <utilities/tablepool.h>
#include <math.h>

// constructor / destructor

LFO::LFO()
{
    _wave  = WaveForms::SINE;
    _rate  = MIN_LFO_RATE;
    _table = new WaveTable( WAVE_TABLE_PRECISION, _rate );
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
    _rate = value;
    _table->setFrequency( _rate );
}

int LFO::getWave()
{
    return _wave;
}

void LFO::setWave( int value )
{
    _wave = value;
    generate();
}

void LFO::generate()
{
    WaveTable* table = TablePool::getTable( _wave );

    if ( table != 0 )
        _table->cloneTable( table );
}

WaveTable* LFO::getTable()
{
    return _table;
}
