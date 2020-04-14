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
#include "channelgroup.h"

namespace MWEngine {

/* constructor / destructor */

ChannelGroup::ChannelGroup()
{
    _processingChain = new ProcessingChain();
    _mixBuffer = new AudioBuffer( AudioEngineProps::OUTPUT_CHANNELS, AudioEngineProps::BUFFER_SIZE );
}

ChannelGroup::~ChannelGroup()
{
    delete _processingChain;
    _audioChannels.clear();
}

/* public methods */

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
        auto audioChannel = _audioChannels.at( i );

        // divide the channels volume by the amount of channels to provide extra headroom
        SAMPLE_TYPE mixVolume = ( SAMPLE_TYPE ) audioChannel->getVolumeLogarithmic() / ( SAMPLE_TYPE ) total;

        audioChannel->mixBuffer( _mixBuffer, mixVolume );

        if ( !audioChannel->isMono ) isMono = false;
    }

    // apply the processing chain onto the mix buffer

    auto processors = _processingChain->getActiveProcessors();
    total = processors.size();

    for ( i = 0; i < total; ++i )
    {
        processors.at( i )->process( _mixBuffer, isMono );
    }

    // write the processed mix buffer into the output

    bufferToMixInto->mergeBuffers( _mixBuffer, 0, 0, 1.0F );

    return true;
}


} // E.O namespace MWEngine
