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
#include "bulkcacher.h"
#include "utils.h"
#include <algorithm>
#include <vector>

/* constructor / destructor */

/**
 * @param sequential when true, the BulkCacher queue
 *        will be processed one after another instead of all in one go
 */
BulkCacher::BulkCacher( bool sequential )
{
    _queue      = new std::vector<BaseCacheableAudioEvent*>();
    _sequential = sequential;
}

BulkCacher::~BulkCacher()
{
    delete _queue;
}

/* public methods */

void BulkCacher::addToQueue( std::vector<BaseCacheableAudioEvent*>* aEvents )
{
    int amount = aEvents->size();
    for ( int i = 0; i < amount; i++ )
    {
        BaseCacheableAudioEvent* event = aEvents->at( i );
        event->setBulkCacheable( true );

        addToQueue( event );
    }
}

void BulkCacher::addToQueue( BaseCacheableAudioEvent* aEvent )
{
    // make sure we don't add the same event twice
    if ( std::find( _queue->begin(), _queue->end(), aEvent ) == _queue->end())
    {
        if ( !aEvent->isCached())
            _queue->push_back( aEvent );
    }
}

bool BulkCacher::removeFromQueue( BaseCacheableAudioEvent* aEvent )
{
    if ( std::find( _queue->begin(), _queue->end(), aEvent ) != _queue->end())
    {
        _queue->erase( std::find( _queue->begin(), _queue->end(), aEvent ));
        return true;
    }
    return false;
}

bool BulkCacher::hasQueue()
{
    return _queue->size() > 0;
}

/**
 * invokes the cache method on all queued audio events
 * (this will also remove them from the queue)
 *
 * in case the BulkCacher was constructed to work in
 * sequence, this method will cache the queue one event at a time
 * note that this method must be invoked after each cache-invocation
 * from the freshly cached event when it finishes its caching routine
 */
void BulkCacher::cacheQueue()
{
    int amount = _queue->size();

    //Debug::log( "BulkCacher::caching %d events", amount );

    for ( int i = 0; i < amount; i++ )
    {
        BaseCacheableAudioEvent* event = _queue->at( i );

        // (interesting detail must remove the event from the queue
        // BEFORE invoking cache (isn't executed synchronously and
        // would cause a recursive loop!)

        removeFromQueue( event );
        event->cache( _sequential );

        // just one when sequential

        if ( _sequential )
            return;
    }
    clearQueue(); // superfluous actually
}

void BulkCacher::clearQueue()
{
    _queue->clear();
}
