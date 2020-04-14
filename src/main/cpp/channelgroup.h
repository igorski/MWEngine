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
#ifndef __MWENGINE__CHANNELGROUP_H_INCLUDED__
#define __MWENGINE__CHANNELGROUP_H_INCLUDED__

#include "processingchain.h"
#include "audiochannel.h"
#include <vector>

namespace MWEngine {
class ChannelGroup
{
    public:
        ChannelGroup();
        ~ChannelGroup();

        ProcessingChain* getProcessingChain();

        bool addAudioChannel( AudioChannel* audioChannel );
        bool removeAudioChannel( AudioChannel* audioChannel );
        bool containsAudioChannel( AudioChannel* audioChannel );

        bool applyEffectsToChannels( AudioBuffer* bufferToMixInto );

    protected:
        std::vector<AudioChannel*> _audioChannels;
        ProcessingChain* _processingChain = nullptr;
        AudioBuffer* _mixBuffer = nullptr;
};
} // E.O namespace MWEngine

#endif
