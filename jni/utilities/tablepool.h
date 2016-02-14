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
#ifndef __TABLEPOOL_H_INCLUDED__
#define __TABLEPOOL_H_INCLUDED__

#include "../wavetable.h"
#include <map>

class TablePool
{
    public:

        // retrieves the pooled table for given waveformType

        static WaveTable* getTable( int waveformType );

        // stores the given WaveTable for the given waveform type inside the pool.
        // if the table was empty and the waveformType exists inside the WaveGenerator,
        // the tables buffer contents are generated on the fly

        static void setTable( WaveTable* waveTable, int waveformType );

        // query whether the pool has a WaveTable for given waveformType

        static bool hasTable( int waveformType );

        // removes the reference to WaveTables of given waveformType
        // from the pool, the existing WaveTable for the type will be deleted

        static bool removeTable( int waveformType );

    private:

        static std::map<int, WaveTable*> _cachedTables;
};

#endif