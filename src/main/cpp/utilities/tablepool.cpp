/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2018 Igor Zinken - http://www.igorski.nl
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

std::map<std::string, WaveTable*> TablePool::_cachedTables;

WaveTable* TablePool::getTable( std::string tableId )
{
    std::map<std::string, WaveTable*>::iterator it = _cachedTables.find( tableId );

    if ( it != _cachedTables.end())
    {
        // table existed, load the pooled version
        return ( WaveTable* )( it->second );
    }
    return nullptr;
}

bool TablePool::setTable( WaveTable* waveTable, std::string tableId )
{
    // don't set a table for the same id twice

    if ( hasTable( tableId ))
        return false;

    std::map<std::string, WaveTable*>::iterator it = _cachedTables.find( tableId );

    // insert the generated table into the pools table map
    _cachedTables.insert( std::pair<std::string, WaveTable*>( tableId, waveTable ));

    return true;
}

bool TablePool::hasTable( std::string tableId )
{
    std::map<std::string, WaveTable*>::iterator it = _cachedTables.find( tableId );
    return it != _cachedTables.end();
}

bool TablePool::removeTable( std::string tableId, bool free )
{
    std::map<std::string, WaveTable*>::iterator it = _cachedTables.find( tableId );

    if ( it != _cachedTables.end())
    {
        if ( free )
            delete ( WaveTable* )( it->second );

        _cachedTables.erase( it );

        return true;
    }
    return false;
}
