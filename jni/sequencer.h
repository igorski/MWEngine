/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2014 Igor Zinken - http://www.igorski.nl
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
#ifndef __SEQUENCER_H_INCLUDED__
#define __SEQUENCER_H_INCLUDED__

#include "bulkcacher.h"
#include "audiochannel.h"
#include "sampledinstrument.h"
#include "synthinstrument.h"
#include "druminstrument.h"
#include "drumpattern.h"
#include <events/basecacheableaudioevent.h>
#include <events/baseaudioevent.h>
#include <vector>

/**
 * Sequencer is the brain that collects
 * AudioEvents for all AudioChannels for the
 * AudioEngine to play at the given "playback head" offset
 *
 * It shouldn't be used directly, but rather via the SequencerAPI
 */
namespace sequencer
{
    extern std::vector<SynthInstrument*> synthesizers;
    extern std::vector<SampledInstrument*> samplers;
    extern DrumInstrument* drummachine;

    // collect all audio events which should be rendered at the given buffer range
    extern std::vector<AudioChannel*> getAudioEvents( std::vector<AudioChannel*> channels, int bufferPosition, int bufferEnd, bool addLiveInstruments );
    extern void clearEvents();

    // TODO : all drum patterns and associated audio events should be migrated to DRUM INSTRUMENT !!

    extern std::vector<DrumPattern*> drumPatterns;
    extern int activeDrumPattern;

    extern BulkCacher* bulkCacher;

    extern void collectSequencedEvents( AudioChannel *channel, std::vector<BaseCacheableAudioEvent*>* audioEvents, int bufferPosition, int bufferEnd );
    extern std::vector<BaseCacheableAudioEvent*>* collectCacheableSequencerEvents( int bufferPosition, int bufferEnd );
    extern void collectLiveEvents( AudioChannel *channel, std::vector<BaseAudioEvent*>* liveEvents );
    extern void collectSequencerSamplerEvents( AudioChannel *channel, std::vector<SampleEvent*> *audioEvents, int bufferPosition, int bufferEnd );
    extern void collectDrumEvents( AudioChannel *channel, int bufferPosition, int bufferEnd );
}

#endif
