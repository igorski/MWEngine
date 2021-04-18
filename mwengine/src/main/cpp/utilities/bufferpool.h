/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Igor Zinken - http://www.igorski.nl
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
#ifndef __MWENGINE__BUFFERPOOL_H_INCLUDED__
#define __MWENGINE__BUFFERPOOL_H_INCLUDED__

#include "../ringbuffer.h"
#include <events/basesynthevent.h>
#include <map>

namespace MWEngine {
namespace BufferPool
{
    // lazily instantiates / retrieves existing buffers of SAMPLE_TYPE
    // at the given buffer size, allows for memcpy of contents instead
    // using loops te re-initialize existing buffers to 0.0 values

    extern SAMPLE_TYPE* getSilentBuffer( int aBufferSize );

    // lazily instantiates / retrieves existing RingBuffer for given aEvent
    // as pitch modulators (see arpeggiator) might shift the frequency of the
    // given event, second argument aFrequency ensures multiple buffers for a single event

    extern RingBuffer* getRingBufferForEvent( BaseSynthEvent* aEvent, float aFrequency );

    // destroys all ring buffers associated with given aEvent

    extern bool destroyRingBuffersForEvent( BaseSynthEvent* aEvent );

    // internal maps

    typedef std::map<unsigned int, RingBuffer*>  innerRingMap;
    typedef std::map<unsigned int, innerRingMap> ringMap;

    extern ringMap                              _eventBufferMap;
    extern std::map<unsigned int, SAMPLE_TYPE*> _silentBufferMap;
}
} // E.O namespace MWEngine

#endif
