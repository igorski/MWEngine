#ifndef __BULKCACHER_H_INCLUDED__
#define __BULKCACHER_H_INCLUDED__

#include "basecacheableaudioevent.h"
#include <vector>

class BulkCacher
{
    public:
        BulkCacher( bool sequential );
        ~BulkCacher();

        void addToQueue     ( std::vector<BaseCacheableAudioEvent*>* aEvents );
        void addToQueue     ( BaseCacheableAudioEvent* aEvent );
        bool removeFromQueue( BaseCacheableAudioEvent* aEvent );
        bool hasQueue();
        void cacheQueue();
        void clearQueue();

    private:
        std::vector<BaseCacheableAudioEvent*>* _queue;
        bool _sequential;
};

#endif