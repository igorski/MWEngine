#ifndef __BASECACHEABLEAUDIOEVENT_H_INCLUDED__
#define __BASECACHEABLEAUDIOEVENT_H_INCLUDED__

#include "baseaudioevent.h"

class BaseCacheableAudioEvent : public BaseAudioEvent
{
    public:
        BaseCacheableAudioEvent();
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
            int _lastWriteIndex;    // last buffer index written to

            void resetCache();      // resets all for next caching sequence
};

#endif
