/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2016 Igor Zinken - http://www.igorski.nl
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
#include "oscillatorproperties.h"
#include <definitions/waveforms.h>
#include <utilities/tablepool.h>

/* constructor / destructor */

OscillatorProperties::OscillatorProperties( int aWaveform, float aDetune, int aOctaveShift, int aFineShift )
{
    detune      = aDetune;
    octaveShift = aOctaveShift;
    fineShift   = aFineShift;
    waveTable   = 0;

    setWaveform( aWaveform );
}

OscillatorProperties::~OscillatorProperties()
{
    delete waveTable;
}

/* public methods */

int OscillatorProperties::getWaveform()
{
    return _waveform;
}

void OscillatorProperties::setWaveform( int aWaveform )
{
    if ( waveTable != 0 )
        delete waveTable;

    if ( aWaveform == WaveForms::TABLE ) {
        WaveTable* tableFromPool = TablePool::getTable( aWaveform );

        if ( tableFromPool != 0 )
            waveTable = tableFromPool->clone();
        else
            aWaveform == WaveForms::SINE;
    }
    _waveform = aWaveform;
}
