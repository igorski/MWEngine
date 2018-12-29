/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2018 Igor Zinken - http://www.igorski.nl
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
#ifndef __MWENGINE__SEQUENCER_H_INCLUDED__
#define __MWENGINE__SEQUENCER_H_INCLUDED__

#include "audiochannel.h"
#include <instruments/baseinstrument.h>
#include <events/basecacheableaudioevent.h>
#include <events/baseaudioevent.h>
#include <utilities/bulkcacher.h>
#include <vector>

/**
 * Sequencer is the brain that collects
 * AudioEvents for all AudioChannels for the
 * AudioEngine to play at the given "playback head" offset, as such
 * Sequencer is an internal tool and shouldn't be used directly for
 * song position / length / tempo manipulations, for these purposes
 * you should be using the methods exposed by the SequencerController instead
 */
namespace MWEngine {
class Sequencer
{
    public:

        static bool playing;
        static std::vector<BaseInstrument*> instruments;
        static BulkCacher* bulkCacher;

        static int registerInstrument   ( BaseInstrument* instrument );
        static bool unregisterInstrument( BaseInstrument* instrument );

        // collect all audio events which should be rendered at the given buffer range

        static bool getAudioEvents( std::vector<AudioChannel*>* channels, int bufferPosition,
                                    int bufferSize, bool addLiveInstruments, bool flushChannels );

        static void updateEvents();
        static void clearEvents();

        static void collectSequencedEvents( BaseInstrument* aInstrument, int bufferPosition, int bufferEnd );
        static void collectLiveEvents     ( BaseInstrument* aInstrument );

        static std::vector<BaseCacheableAudioEvent*>* collectCacheableSequencerEvents( int bufferPosition, int bufferEnd );
};
} // E.O namespace MWEngine

#endif
