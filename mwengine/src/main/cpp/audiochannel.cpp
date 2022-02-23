/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2022 Igor Zinken - https://www.igorski.nl
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
#include <utilities/volumeutil.h>

namespace MWEngine {

unsigned int AudioChannel::INSTANCE_COUNT = 0;

/* constructor / destructor */

AudioChannel::AudioChannel( float aVolume )
{
    init();
    setVolume( aVolume );
}

AudioChannel::AudioChannel( float aVolume, int aMaxBufferPosition )
{
    init();
    setVolume( aVolume );
    maxBufferPosition = aMaxBufferPosition;
}

AudioChannel::~AudioChannel()
{
    reset();
    --INSTANCE_COUNT;

    delete _outputBuffer;
    delete _cachedBuffer;
    delete processingChain;

    _outputBuffer   = nullptr;
    _cachedBuffer   = nullptr;
    processingChain = nullptr;
}

/* public methods */

float AudioChannel::getVolume()
{
    return VolumeUtil::toLinear( _volume );
}

float AudioChannel::getVolumeLogarithmic()
{
    return _volume;
}

void AudioChannel::setVolume( float value )
{
    _volume = VolumeUtil::toLog( value );
}

void AudioChannel::addEvent( BaseAudioEvent* aEvent )
{
    audioEvents.push_back( aEvent );
}

void AudioChannel::addLiveEvent( BaseAudioEvent* aLiveEvent )
{
    hasLiveEvents = true;
    liveEvents.push_back( aLiveEvent );
}

void AudioChannel::readCachedBuffer( AudioBuffer* aOutputBuffer, int aReadOffset )
{
    if ( aReadOffset >= _cacheStartOffset && aReadOffset <= _cacheEndOffset )
    {
        aOutputBuffer->mergeBuffers( _cachedBuffer, _cacheReadPointer, 0, 1.0 );
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

    if ( !value || ( _cachedBuffer != nullptr && _cachedBuffer->bufferSize != aBufferSize ))
        clearCachedBuffer();

    if ( value )
    {
        if ( _cachedBuffer == nullptr )
            _cachedBuffer = new AudioBuffer( AudioEngineProps::OUTPUT_CHANNELS, aBufferSize );
    }
    isCaching = value;
}

void AudioChannel::createOutputBuffer()
{
    int bufferSize     = AudioEngineProps::BUFFER_SIZE;
    int outputChannels = AudioEngineProps::OUTPUT_CHANNELS;

    if ( _outputBuffer != nullptr )
    {
        if ( _outputBuffer->bufferSize       == bufferSize &&
             _outputBuffer->amountOfChannels == outputChannels )
        {
            return; // don't create, existing one is satisfactory
        }
        delete _outputBuffer;
    }
    _outputBuffer = new ResizableAudioBuffer( outputChannels, bufferSize );
}

ResizableAudioBuffer* AudioChannel::getOutputBuffer()
{
    return _outputBuffer;
}

void AudioChannel::mixBuffer( AudioBuffer* bufferToMixInto, float mixVolume ) {

    // if channels panning is set to center, use AudioBuffer mix method

    if ( _pan == 0 ) {
        bufferToMixInto->mergeBuffers( _outputBuffer, 0, 0, mixVolume );
    }
    else {

        int buffersToWrite = std::min( bufferToMixInto->bufferSize, _outputBuffer->bufferSize );

        // TODO: currently stereo only
        // assumption that buffers have equal amount of channels

        SAMPLE_TYPE* leftSrcBuffer     = _outputBuffer->getBufferForChannel( 0 );
        SAMPLE_TYPE* rightSrcBuffer    = _outputBuffer->getBufferForChannel( 1 );
        SAMPLE_TYPE* leftTargetBuffer  = bufferToMixInto->getBufferForChannel( 0 );
        SAMPLE_TYPE* rightTargetBuffer = bufferToMixInto->getBufferForChannel( 1 );

        // apply pan to output volume
        SAMPLE_TYPE leftVolume  = mixVolume * _leftVolume;
        SAMPLE_TYPE rightVolume = mixVolume * _rightVolume;
        bool isLeftPanned = ( _pan < 0 );

        // or pro tools style ? (https://stackoverflow.com/questions/67062207/how-to-pan-audio-sample-data-naturally)
        for ( int i = 0; i < buffersToWrite; ++i ) {
            leftTargetBuffer[ i ]  += leftSrcBuffer[ i ]  * leftVolume;
            rightTargetBuffer[ i ] += rightSrcBuffer[ i ] * rightVolume;

            // pan the channel contents into the opposite channel

            if ( isLeftPanned )
                leftTargetBuffer[ i ] += rightSrcBuffer[ i ] * _rightVolume;
            else
                rightTargetBuffer[ i ] += leftSrcBuffer[ i ] * _leftVolume;
        }
    }
}

/**
 * write the current contents of the buffer
 * into the cached buffer
 */
void AudioChannel::writeCache( AudioBuffer* aBuffer, int aReadOffset )
{
    int mergedSamples   = _cachedBuffer->mergeBuffers( aBuffer, aReadOffset, _cacheWritePointer, 1.0 );
    _cacheWritePointer += mergedSamples;

    // caching completed ?

    if ( _cacheWritePointer >= _cachedBuffer->bufferSize )
    {
        hasCache           = true;
        isCaching          = false;
        _cacheReadPointer  = 0;
        _cacheWritePointer = 0;
    }
}

void AudioChannel::clearCachedBuffer()
{
    if ( _cachedBuffer != nullptr )
    {
        delete _cachedBuffer;
        _cachedBuffer = nullptr;
    }
    hasCache  = false;
    isCaching = _canCache;
}

void AudioChannel::reset()
{
    audioEvents.clear();
    liveEvents.clear();
    hasLiveEvents = false;
}

float AudioChannel::getPan()
{
    return _pan;
}

void AudioChannel::setPan( float value )
{
    _pan = value;

    SAMPLE_TYPE pan_mapped = (( _pan + 1.0 ) / 2.0 ) * ( PI / 2.0 );

    _leftVolume  = sin( pan_mapped );
    _rightVolume = cos( pan_mapped );
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
    _outputBuffer      = nullptr;
    _cachedBuffer      = nullptr;
    _cacheReadPointer  = 0;
    _cacheWritePointer = 0;
    _cacheStartOffset  = 0;
    _cacheEndOffset    = 0;
    _volume            = VolumeUtil::toLog( 1.0 );
    maxBufferPosition  = 0;
    processingChain    = new ProcessingChain();

    setPan( 0 );
    createOutputBuffer();
}

} // E.O namespace MWEngine
