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
#ifndef __ARPEGGIATOR_H_INCLUDED__
#define __ARPEGGIATOR_H_INCLUDED__

class Arpeggiator
{
    public:
        Arpeggiator();
        ~Arpeggiator();

        // the amount of steps (i.e. notes) the arpeggio will cycle through
        // the size defines the buffer length for a single step (i.e. "speed"),
        // while the shift defines the semi-note pitch shift

        static const int MAX_STEPS = 16;

        int getStepSize();
        void setStepSize( int value );
        int getAmountOfSteps();
        void setAmountOfSteps( int value );
        int getShiftForStep( int step );
        void setShiftForStep( int step, int shift );
        int getStep();
        void setStep( int value );

        // increment the current buffer position, will return
        // a boolean indicating whether the arpeggiator has moved to the next step
        bool peek();

        int getBufferPosition();
        void setBufferPosition( int value );

        float getPitchForStep( int step, float basePitch );

        void cloneProperties( Arpeggiator* source );
        Arpeggiator* clone();

    private:
        int _step;
        int _stepAmount;
        int _stepSize;
        int _stepShifts[ MAX_STEPS ];
        int _bufferPosition;
};

#endif