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
#include "tablepool.h"
#include <generators/wavegenerator.h>

std::map<int, WaveTable*> TablePool::_cachedTables;

WaveTable* TablePool::getTable( int waveformType )
{
    std::map<int, WaveTable*>::iterator it = _cachedTables.find( waveformType );

    if ( it != _cachedTables.end())
    {
        // table existed, load the pooled version
        return ( WaveTable* )( it->second );
    }
    return 0;
}

void TablePool::setTable( WaveTable* waveTable, int waveformType )
{
    std::map<int, WaveTable*>::iterator it = _cachedTables.find( waveformType );

    if ( it != _cachedTables.end())
    {
        // table existed, remove it so we can replace it
        delete ( WaveTable* )( it->second );
        _cachedTables.erase( it );
    }

    // wave table hasn't been generated yet ? generate its contents on the fly
    if ( !waveTable->hasContent() )
        WaveGenerator::generate( waveTable, waveformType );

    // insert the generated table into the pools table map
    _cachedTables.insert( std::pair<int, WaveTable*>( waveformType, waveTable ));
}

bool TablePool::hasTable( int waveformType )
{
    std::map<int, WaveTable*>::iterator it = _cachedTables.find( waveformType );
    return it != _cachedTables.end();
}

bool TablePool::removeTable( int waveformType )
{
    std::map<int, WaveTable*>::iterator it = _cachedTables.find( waveformType );

    if ( it != _cachedTables.end())
    {
        // table existed, remove it so we can replace it
        delete ( WaveTable* )( it->second );
        _cachedTables.erase( it );

        return true;
    }
    return false;
}
