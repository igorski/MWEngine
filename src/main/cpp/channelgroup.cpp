/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Igor Zinken - https://www.igorski.nl
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
#include <channelgroup.h>
#include <audioengine.h>
#include <utilities/volumeutil.h>

namespace MWEngine {

/* constructor / destructor */

ChannelGroup::ChannelGroup()
{
    construct();
}

ChannelGroup::ChannelGroup( float volume )
{
    construct();
    setVolume( volume );
}

ChannelGroup::~ChannelGroup()
{
    AudioEngine::removeChannelGroup( this );

    delete _processingChain;
    _audioChannels.clear();
}

/* public methods */

float ChannelGroup::getVolume()
{
    return VolumeUtil::toLinear( _volume );
}

float ChannelGroup::getVolumeLogarithmic()
{
    return _volume;
}

void ChannelGroup::setVolume( float value )
{
    _volume = VolumeUtil::toLog( value );
}

ProcessingChain* ChannelGroup::getProcessingChain()
{
    return _processingChain;
}

bool ChannelGroup::addAudioChannel( AudioChannel* audioChannel )
{
    if ( !containsAudioChannel( audioChannel )) {
        _audioChannels.push_back( audioChannel );
        return true;
    }
    return false;
}

bool ChannelGroup::removeAudioChannel( AudioChannel* audioChannel )
{
    auto it = std::find( _audioChannels.begin(), _audioChannels.end(), audioChannel );
    if ( it != _audioChannels.end() ) {
        _audioChannels.erase( it );
        return true;
    }
    return false;
}

bool ChannelGroup::containsAudioChannel( AudioChannel* audioChannel )
{
    auto it = std::find( _audioChannels.begin(), _audioChannels.end(), audioChannel );
    return it != _audioChannels.end();
}

bool ChannelGroup::applyEffectsToChannels( AudioBuffer* bufferToMixInto )
{
    if ( _audioChannels.empty() ) {
        return false;
    }

    // mix all audio channels into one single buffer

    _mixBuffer->silenceBuffers();
    unsigned long total = _audioChannels.size();
    bool isMono = AudioEngineProps::OUTPUT_CHANNELS == 1;
    size_t i;

    for ( i = 0; i < total; ++i ) {
        auto audioChannel = _audioChannels[ i ];

        // divide the channels volume by the amount of channels to provide extra headroom
        auto mixVolume = audioChannel->getVolumeLogarithmic() / total;

        audioChannel->mixBuffer( _mixBuffer, mixVolume );

        if ( !audioChannel->isMono ) isMono = false;
    }

    // apply the processing chain onto the mix buffer

    auto processors = _processingChain->getActiveProcessors();
    total = processors.size();

    for ( i = 0; i < total; ++i ) {
        processors[ i ]->process( _mixBuffer, isMono );
    }

    // write the processed mix buffer into the output

    bufferToMixInto->mergeBuffers( _mixBuffer, 0, 0, getVolumeLogarithmic() );

    return true;
}

void ChannelGroup::construct()
{
    _processingChain = new ProcessingChain();
    _mixBuffer = new AudioBuffer( AudioEngineProps::OUTPUT_CHANNELS, AudioEngineProps::BUFFER_SIZE );
}


} // E.O namespace MWEngine
