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
#ifndef __MWENGINE__LFO_H_INCLUDED__
#define __MWENGINE__LFO_H_INCLUDED__

#include "global.h"
#include "wavetable.h"
#include <algorithm>

namespace MWEngine {
class LFO
{
    public:

        // Note that auto-generation of waveforms can be
        // expensive on the CPU when it happens upon construction
        // during engine use (though subsequent uses of the generated
        // table are pooled inside TablePool. Ideally, register custom waveforms
        // in the TablePool before instantiating an LFO (or SynthInstrument)

        LFO();
        ~LFO();

        static const float MAX_RATE() { return 10.f; } // the maximum rate of oscillation in Hz
        static const float MIN_RATE() { return 0.1f; } // the minimum rate of oscillation in Hz

        float getRate();
        void setRate( float value );
        int getWave();
        void setWave( int value );
        float getDepth();
        void setDepth( float value );

        void generate();
        WaveTable* getTable();

        // set the properties the LFO will modulate
        // value == the value the LFO's sweep is modulating
        // min == the minimum allowed value the LFO should return
        // max == the maximum allowed value the LFO should return
        // minimum and maximum values are capped automatically by
        // by the LFO's depth. Also see filter.cpp

        void cacheProperties( float value, float min, float max );

        // sweep the LFO by a single sample and return the modulated
        // value (specified in "cacheProperties"), relative to the LFO
        // depth. Also see filter.cpp

        inline float sweep()
        {
            return std::min( _max, _min + _range * ( float ) _table->peek() );
        }


    protected:
        float _rate;
        int   _wave;
        WaveTable* _table;

        float _depth;   // between 0 - 1
        float _range;   // range is determine by the depth of the LFO sweep
        float _min;     // max frequency expected for value when modulated at current depth
        float _max;     // min frequency expected for value when modulated at current depth

        float _cvalue, _cmin, _cmax;
};
} // E.O namespace MWEngine

#endif
