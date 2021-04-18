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
#include "bufferpool.h"
#include <utilities/bufferutility.h>

namespace MWEngine {
namespace BufferPool
{
    std::map<unsigned int, SAMPLE_TYPE*> _silentBufferMap;
    ringMap                              _eventBufferMap;

    SAMPLE_TYPE* getSilentBuffer( int aBufferSize )
    {
        // retrieve buffer from map if existed

        std::map<unsigned int, SAMPLE_TYPE*>::iterator it = _silentBufferMap.find( aBufferSize );

        if ( it != _silentBufferMap.end()) {
            return it->second;
        }
        else
        {
            SAMPLE_TYPE* silentBuffer = BufferUtility::generateSilentBuffer( aBufferSize );
            _silentBufferMap.insert( std::pair<unsigned int, SAMPLE_TYPE*>( aBufferSize, silentBuffer ));

            return silentBuffer;
        }
    }

    RingBuffer* getRingBufferForEvent( BaseSynthEvent* aEvent, float aFrequency )
    {
        // retrieve eventMap from map if existed

        std::map<unsigned int, innerRingMap>::iterator it = _eventBufferMap.find( aEvent->instanceId );
        innerRingMap eventMap;

        if ( it != _eventBufferMap.end())
        {
            eventMap = it->second;
        }
        else
        {
            innerRingMap inner = innerRingMap();
            /*if (*/ _eventBufferMap.insert( std::make_pair( aEvent->instanceId, inner ));//.second )
            //{
                it = _eventBufferMap.find( aEvent->instanceId );
                eventMap = it->second;
            //}
        }

        // use integer value of frequency for map lookup (should be fine for
        // everything but the most microtonal requests !! )

        unsigned int frequency = ( unsigned int ) aFrequency;

        // retrieve ringBuffer from eventMap if existed

        std::map<unsigned int, RingBuffer*>::iterator mapIt = eventMap.find( frequency );

        if ( mapIt != eventMap.end() )
            return mapIt->second;

        // new ringBuffer, create on the fly

        int ringBufferSize     = ( int ) (( SAMPLE_TYPE ) AudioEngineProps::SAMPLE_RATE / aFrequency );
        RingBuffer* ringBuffer = new RingBuffer( ringBufferSize );

        _eventBufferMap[ aEvent->instanceId ][ frequency ] = ringBuffer;

        return ringBuffer;
    }

    bool destroyRingBuffersForEvent( BaseSynthEvent* aEvent )
    {
        std::map<unsigned int, innerRingMap>::iterator it = _eventBufferMap.find( aEvent->instanceId );

        if ( it != _eventBufferMap.end())
        {
            std::map<unsigned int, RingBuffer*> eventMap = it->second;

            // destroy ring buffers

            for ( innerRingMap::iterator in_it = eventMap.begin(); in_it != eventMap.end(); )
            {
                RingBuffer* ringBuffer = in_it->second;

                delete ringBuffer;

                eventMap.erase( in_it++ );
            }
            _eventBufferMap.erase( it );    // erase contents for events instanceId
            return true;
        }
        return false;
    }
}

} // E.O namespace MWEngine
