/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2014 Igor Zinken - http://www.igorski.nl
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
#include "arpeggiator.h"
#include "utils.h"
#include <cmath>

/* constructor / destructor */

Arpeggiator::Arpeggiator()
{
    _step           = 0;
    _stepAmount     = 0;
    _stepSize       = 0;
    _bufferPosition = 0;

    for ( int i = 0; i < MAX_STEPS; ++i )
        _stepShifts[ i ] = 0;
}

Arpeggiator::~Arpeggiator()
{

}

/* public methods */

int Arpeggiator::getStepSize()
{
    return _stepSize;
}

void Arpeggiator::setStepSize( int value )
{
    _stepSize = value;
}

int Arpeggiator::getAmountOfSteps()
{
    return _stepAmount;
}

void Arpeggiator::setAmountOfSteps( int value )
{
    _stepAmount = value;
}

int Arpeggiator::getShiftForStep( int step )
{
    return _stepShifts[ step ];
}

void Arpeggiator::setShiftForStep( int step, int shift )
{
    _stepShifts[ step ] = shift;
}

int Arpeggiator::getStep()
{
    return _step;
}

bool Arpeggiator::peek()
{
    bool stepped = false;

    // buffer position equals the buffer size of a single step > increment
    if ( ++_bufferPosition >= _stepSize )
    {
        _bufferPosition = 0;

        // increment step, but keep step in range
        if ( ++_step == _stepAmount )
            _step = 0;

        stepped = true;
    }
    return stepped;
}

float Arpeggiator::getPitchForStep( int step, float basePitch )
{
    float pitch = basePitch;
    int shift   = _stepShifts[ step ];
    int i       = 0;

    // semitone up   = 1.0594
    // semitone down = 0.9439

    if ( shift > 0 )
    {
        for ( i; i < shift; ++i )
            pitch *= 1.0594;
    }
    else if ( shift < 0 )
    {
        for ( i; i < std::abs( shift ); ++i )
            pitch *= 0.9439;
    }
    return pitch;
}

Arpeggiator* Arpeggiator::clone()
{
    Arpeggiator* out = new Arpeggiator();

    out->setStepSize     ( _stepSize );
    out->setAmountOfSteps( _stepAmount );

    for ( int i = 0; i < _stepAmount; ++i )
        out->setShiftForStep( i, _stepShifts[ i ]);

    return out;
}
