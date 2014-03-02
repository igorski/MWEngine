#include "basecacheableaudioevent.h"

/* constructor / destructor */

BaseCacheableAudioEvent::BaseCacheableAudioEvent()
{

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
 * @param doCallback whether to execute a callback command on copmletion
 */
void BaseCacheableAudioEvent::cache( bool doCallback )
{
    // override in subclass
}

void BaseCacheableAudioEvent::resetCache()
{
    _lastWriteIndex   = 0;
    _cachingCompleted = false;
}