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
#include <modules/arpeggiator.h>
#include <utilities/utils.h>

namespace MWEngine {

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

    if ( _bufferPosition >= _stepSize )
        _bufferPosition = 0;
}

int Arpeggiator::getAmountOfSteps()
{
    return _stepAmount;
}

void Arpeggiator::setAmountOfSteps( int value )
{
    _stepAmount = value;

    if ( _step >= _stepAmount )
        _step = 0;
}

int Arpeggiator::getShiftForStep( int step )
{
    return _stepShifts[ step ];
}

void Arpeggiator::setShiftForStep( int step, int shift )
{
    _stepShifts[ step ] = shift;
}

void Arpeggiator::setStep( int value )
{
    _step = value;
}

int Arpeggiator::getBufferPosition()
{
    return _bufferPosition;
}

void Arpeggiator::setBufferPosition( int position )
{
    _bufferPosition = position;
}

void Arpeggiator::cloneProperties( Arpeggiator* source )
{
    source->setStepSize     ( _stepSize );
    source->setAmountOfSteps( _stepAmount );

    for ( int i = 0; i < _stepAmount; ++i )
        source->setShiftForStep( i, _stepShifts[ i ]);
}

Arpeggiator* Arpeggiator::clone()
{
    Arpeggiator* out = new Arpeggiator();
    cloneProperties( out );

    return out;
}

} // E.O namespace MWEngine
