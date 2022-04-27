/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2021 Igor Zinken - https://www.igorski.nl
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
#include "processingchain.h"
#include "global.h"

namespace MWEngine {

// constructor

ProcessingChain::ProcessingChain()
{

}

ProcessingChain::~ProcessingChain()
{
    while ( _activeProcessors.size() > 0 ) {
        auto processor = _activeProcessors.at( 0 );
        if ( processor != nullptr ) {
            removeProcessor( processor );
        } else {
            _activeProcessors.erase( _activeProcessors.begin() );
        }
    }
}

/* public methods */

void ProcessingChain::addProcessor( BaseProcessor* processor )
{
    auto it = std::find( _activeProcessors.begin(), _activeProcessors.end(), processor );
    bool hasProcessor = it != _activeProcessors.end();

    if ( hasProcessor ) {
        return;
    }
    _activeProcessors.push_back( processor );
    processor->setChain( this );
}

bool ProcessingChain::removeProcessor( BaseProcessor* processor )
{
    auto it = std::find( _activeProcessors.begin(), _activeProcessors.end(), processor );
    if ( it != _activeProcessors.end() ) {
        processor->setChain( nullptr );
        _activeProcessors.erase( it );

        return true;
    }
    return false;
}

BaseProcessor* ProcessingChain::getProcessorAt( int index )
{
    if ( index >= amountOfProcessors()) {
        return nullptr;
    }
    return _activeProcessors.at( index );
}

std::vector<BaseProcessor*> ProcessingChain::getActiveProcessors()
{
    return _activeProcessors;
}

bool ProcessingChain::hasProcessors()
{
    return !_activeProcessors.empty();
}

int ProcessingChain::amountOfProcessors()
{
    return _activeProcessors.size();
}

void ProcessingChain::reset()
{
    _activeProcessors.clear();
}


} // E.O namespace MWEngine
