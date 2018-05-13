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
#ifndef __BASECACHEABLEAUDIOEVENT_H_INCLUDED__
#define __BASECACHEABLEAUDIOEVENT_H_INCLUDED__

#include "baseaudioevent.h"

/**
 * BaseCacheableAudioEvent provides an interface for
 * AudioEvents that synthesize their content on-the-fly
 * but have immutable content (e.g. buffer contents don't change
 * each time the Event plays back). To spare CPU resources, this
 * event can cache its contents once they have fully rendered and
 * read from cache whenever the buffer is requested.
 *
 * NOTE : this consumes more memory though.
 */
class BaseCacheableAudioEvent : public BaseAudioEvent
{
    public:
        BaseCacheableAudioEvent( BaseInstrument* instrument );
        virtual ~BaseCacheableAudioEvent();

        virtual void setAutoCache( bool value );
        virtual void cache( bool doCallback );
        virtual bool isCached();

        void setBulkCacheable( bool value );

    protected:

            // removal of AudioEvents must occur outside of the
            // cache loop, by activating this bool we're queuing
            // the SynthEvent for removal

            bool _cancel;           // whether we should cancel caching
            bool _caching;          // whether we're currently caching
            bool _cachingCompleted; // whether we're done caching
            bool _bulkCacheable;    // whether we can be part of a bulk cacher
            bool _autoCache;        // whether we can cache the entire buffer in one go

            // render-specific
            int _cacheWriteIndex;    // last buffer index written to

            void resetCache();      // resets all for next caching sequence
};

#endif
