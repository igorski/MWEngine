/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 Igor Zinken - https://www.igorski.nl
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
#include "resizable_buffergroup.h"

namespace MWEngine {

/* constructor / destructor */

ResizableBufferGroup::ResizableBufferGroup( int bufferAmount, int amountOfChannels, int bufferSize )
{
    _buffers = new std::vector<ResizableAudioBuffer*>( bufferAmount );

    for ( size_t i = 0; i < bufferAmount; ++i ) {
        _buffers->at( i ) = new ResizableAudioBuffer( amountOfChannels, bufferSize );
    }
}

ResizableBufferGroup::~ResizableBufferGroup()
{
    if ( _buffers != nullptr ) {
        while ( !_buffers->empty()) {
            delete _buffers->back(), _buffers->pop_back();
        }
        delete _buffers;
        _buffers = nullptr;
    }
}

/* public methods */

ResizableAudioBuffer* ResizableBufferGroup::getBufferAtIndex( int index )
{
    return _buffers->at( index );
}

int ResizableBufferGroup::getGroupSize()
{
    return _buffers->size();
}

AudioBuffer* ResizableBufferGroup::getContentSerialized()
{
    auto firstBuffer = _buffers->at( 0 );
    int bufferAmount = getGroupSize();

    if ( bufferAmount == 1 ) {
        return firstBuffer->clone();
    }
    auto out = new AudioBuffer( firstBuffer->amountOfChannels, firstBuffer->bufferSize * bufferAmount );

    for ( size_t i = 0; i < bufferAmount; ++i ) {
        out->mergeBuffers( _buffers->at( i ), 0, firstBuffer->bufferSize * i, MAX_VOLUME );
    }
    return out;
}

void ResizableBufferGroup::resize( int newSize )
{
    int amount = getGroupSize();
    for ( size_t i = 0; i < amount; ++i ) {
        _buffers->at( i )->resize( newSize );
    }
}

} // E.O. namespace MWEngine