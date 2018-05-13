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
#include "basecacheableaudioevent.h"
#include "../sequencer.h"

/* constructor / destructor */

BaseCacheableAudioEvent::BaseCacheableAudioEvent( BaseInstrument* instrument )
{
    setInstrument( instrument );
    construct();
}

BaseCacheableAudioEvent::~BaseCacheableAudioEvent()
{

}

/* public methods */

void BaseCacheableAudioEvent::setBulkCacheable( bool value )
{
    _bulkCacheable = value;
}

bool BaseCacheableAudioEvent::isCached()
{
    return _cachingCompleted;
}

void BaseCacheableAudioEvent::setAutoCache( bool aValue )
{
    _autoCache = aValue;
}

/**
 * (pre-)cache the contents of the BaseSynthEvent in its entirety
 * this can be done in idle time to make optimum use of resources
 */
void BaseCacheableAudioEvent::cache( bool doCallback )
{
    if ( _buffer == 0 ) return; // cache request likely invoked after destruction

    _caching = true;

    // custom  derived class cache implementation here

    if ( doCallback )
        Sequencer::bulkCacher->cacheQueue();

    _caching = false;
}

void BaseCacheableAudioEvent::resetCache()
{
    _cacheWriteIndex  = 0;
    _cachingCompleted = false;
    _caching          = false;
}
