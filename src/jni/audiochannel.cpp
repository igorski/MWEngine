#include "audiochannel.h"

#include "utils.h"

/* constructor / destructor */

AudioChannel::AudioChannel( ProcessingChain *aChain, double aMixVolume )
{
    processingChain   = aChain;
    mixVolume         = aMixVolume;
    maxBufferPosition = 0;

    init();
}

AudioChannel::AudioChannel( ProcessingChain *aChain, double aMixVolume, int aMaxBufferPosition )
{
    processingChain   = aChain;
    mixVolume         = aMixVolume;
    maxBufferPosition = aMaxBufferPosition;

    init();
}

AudioChannel::~AudioChannel()
{
    reset();
}

/* public methods */

void AudioChannel::addEvent( SampleEvent* aEvent )
{
    audioEvents.push_back( aEvent );
}

void AudioChannel::addLiveEvent( BaseAudioEvent* aLiveEvent )
{
    hasLiveEvents = true;
    liveEvents.push_back( aLiveEvent );
}

AudioBuffer* AudioChannel::readCachedBuffer( AudioBuffer* aOutputBuffer, int aReadOffset )
{
    if ( aReadOffset >= _cacheStartOffset && aReadOffset <= _cacheEndOffset )
    {
        aOutputBuffer->mergeBuffers( _cachedBuffer, _cacheReadPointer, 0 );
        _cacheReadPointer += aOutputBuffer->bufferSize;
    }
}

bool AudioChannel::canCache()
{
    return _canCache;
}

void AudioChannel::canCache( bool value, int aBufferSize, int aCacheStartOffset, int aCacheEndOffset )
{
    if ( !_canCache )
        _cacheWritePointer = 0;

    _canCache         = value;
    _cacheStartOffset = aCacheStartOffset;
    _cacheEndOffset   = aCacheEndOffset;

    if ( !value || ( _cachedBuffer != 0 && _cachedBuffer->bufferSize != aBufferSize ))
        clearCachedBuffer();

    if ( value )
    {
        if ( _cachedBuffer == 0 )
            _cachedBuffer = new AudioBuffer( audio_engine::OUTPUT_CHANNELS, aBufferSize );
    }
    else {
        isCaching = false;
    }
}

/**
 * write the current contents of the buffer
 * into the cached buffer
 */
void AudioChannel::writeCache( AudioBuffer* aBuffer, int aReadOffset )
{
    int mergedSamples   = _cachedBuffer->mergeBuffers( aBuffer, aReadOffset, _cacheWritePointer );
    _cacheWritePointer += mergedSamples;

    // caching completed ?

    if ( _cacheWritePointer >= _cachedBuffer->bufferSize )
    {
        DebugTool::log( "caching completed for channel" );

        hasCache           = true;
        isCaching          = false;
        _cacheReadPointer  = 0;
        _cacheWritePointer = 0;
    }
}

void AudioChannel::clearCachedBuffer()
{
    if ( _cachedBuffer != 0 )
    {
        delete _cachedBuffer;
        _cachedBuffer = 0;
    }
    hasCache = false;
}

void AudioChannel::reset()
{
    // we shouldn't invoke delete as we need the events on a next sweep or elsewhere ;-)
    /*
    while( !audioEvents.empty())
    {
        delete audioEvents.back();
        audioEvents.pop_back();
    }*/
    audioEvents.clear();
    /*
    while( !liveEvents.empty())
    {
        delete liveEvents.back();
        liveEvents.pop_back();
    }
    */
    liveEvents.clear();
}

/* protected methods */

void AudioChannel::init()
{
    muted              = false;
    isMono             = false;
    hasCache           = false;
    isCaching          = false;
    _canCache          = false;
    _cachedBuffer      = 0;
    _cacheReadPointer  = 0;
    _cacheWritePointer = 0;
    _cacheStartOffset  = 0;
    _cacheEndOffset    = 0;
}
