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
#ifndef __MWENGINE__WAVETABLE_H_INCLUDED__
#define __MWENGINE__WAVETABLE_H_INCLUDED__

#include "global.h"

namespace MWEngine {
class WaveTable
{
    public:
        WaveTable( int aTableLength, float aFrequency );
        ~WaveTable();

        int tableLength;
        SAMPLE_TYPE* getBuffer();
        void setBuffer( SAMPLE_TYPE* aBuffer );

        void setFrequency( float aFrequency );
        float getFrequency();

        bool hasContent();

        // accumulators are used to retrieve a sample from the wave table

        SAMPLE_TYPE getAccumulator();
        void setAccumulator( SAMPLE_TYPE offset );

        /**
         * retrieve a value from the wave table for the current
         * accumulator position, this method also increments
         * the accumulator and keeps it within bounds
         */
        inline SAMPLE_TYPE peek()
        {
            // the wave table offset to read from
            int readOffset = ( _accumulator == 0 ) ? 0 : ( int ) ( _accumulator / SR_OVER_LENGTH );

            // increment the accumulators read offset
            _accumulator += _frequency;

            // keep the accumulator in the bounds of the sample frequency
            if ( _accumulator > AudioEngineProps::SAMPLE_RATE )
                _accumulator -= AudioEngineProps::SAMPLE_RATE;

            // return the sample present at the calculated offset within the table
            return _buffer[ readOffset ];
        }

        void cloneTable( WaveTable* waveTable );
        WaveTable* clone();

    protected:
        SAMPLE_TYPE* _buffer;       // cached buffer (is a wave table)
        SAMPLE_TYPE _accumulator;   // is read offset in wave table buffer
        SAMPLE_TYPE SR_OVER_LENGTH;
        float       _frequency;     // frequency (in Hz) of waveform cycle when reading
};
} // E.O namespace MWEngine

#endif
