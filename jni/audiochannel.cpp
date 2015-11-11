/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2015 Igor Zinken - http://www.igorski.nl
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
#include "audiochannel.h"

#include <utilities/utils.h>

unsigned int AudioChannel::INSTANCE_COUNT = 0;

/* constructor / destructor */

AudioChannel::AudioChannel( float aMixVolume )
{
    mixVolume         = aMixVolume;
    maxBufferPosition = 0;

    init();
}

AudioChannel::AudioChannel( float aMixVolume, int aMaxBufferPosition )
{
    mixVolume         = aMixVolume;
    maxBufferPosition = aMaxBufferPosition;

    init();
}

AudioChannel::~AudioChannel()
{
    reset();
    --INSTANCE_COUNT;

    delete _outputBuffer;
    delete _cachedBuffer;
    delete processingChain;
}

/* public methods */

void AudioChannel::addEvent( BaseAudioEvent* aEvent )
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
        aOutputBuffer->mergeBuffers( _cachedBuffer, _cacheReadPointer, 0, 1.0f );
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
            _cachedBuffer = new AudioBuffer( AudioEngineProps::OUTPUT_CHANNELS, aBufferSize );
    }
    else {
        isCaching = false;
    }
}

void AudioChannel::createOutputBuffer()
{
    int bufferSize     = AudioEngineProps::BUFFER_SIZE;
    int outputChannels = AudioEngineProps::OUTPUT_CHANNELS;

    if ( _outputBuffer != 0 )
    {
        if ( _outputBuffer->bufferSize       == bufferSize &&
             _outputBuffer->amountOfChannels == outputChannels )
        {
            return; // don't create, existing one is satisfactory
        }
        delete _outputBuffer;
    }
    _outputBuffer = new AudioBuffer( outputChannels, bufferSize );
}

AudioBuffer* AudioChannel::getOutputBuffer()
{
    return _outputBuffer;
}

/**
 * write the current contents of the buffer
 * into the cached buffer
 */
void AudioChannel::writeCache( AudioBuffer* aBuffer, int aReadOffset )
{
    int mergedSamples   = _cachedBuffer->mergeBuffers( aBuffer, aReadOffset, _cacheWritePointer, 1.0f );
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
    instanceId         = ++INSTANCE_COUNT;
    _canCache          = false;
    _outputBuffer      = 0;
    _cachedBuffer      = 0;
    _cacheReadPointer  = 0;
    _cacheWritePointer = 0;
    _cacheStartOffset  = 0;
    _cacheEndOffset    = 0;
    processingChain    = new ProcessingChain();

    createOutputBuffer();
}
