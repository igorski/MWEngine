#ifndef __SEQUENCER_H_INCLUDED__
#define __SEQUENCER_H_INCLUDED__

#include "basecacheableaudioevent.h"
#include "baseaudioevent.h"
#include "bulkcacher.h"
#include "audiochannel.h"
#include "synthinstrument.h"
#include "druminstrument.h"
#include "drumpattern.h"
#include <vector>

namespace sequencer
{
    extern std::vector<SynthInstrument*>* synthesizers;
    extern DrumInstrument* drummachine;

    // collect all audio events which should be rendered at the given buffer range
    extern std::vector<AudioChannel*> getAudioEvents( std::vector<AudioChannel*> channels, int bufferPosition, int bufferEnd, bool addLiveInstruments );
    extern void clearEvents();

    // TODO : all drum patterns and associated audio events should be migrated to DRUM INSTRUMENT !!

    extern std::vector<DrumPattern*> drumPatterns;
    extern int activeDrumPattern;

    extern BulkCacher* bulkCacher;
}

/* "internal" methods */

void collectSequencerEvents( AudioChannel *channel, std::vector<BaseCacheableAudioEvent*>* audioEvents, int bufferPosition, int bufferEnd );
std::vector<BaseCacheableAudioEvent*>* collectCacheableSequencerEvents( int bufferPosition, int bufferEnd );
void collectLiveEvents( AudioChannel *channel, std::vector<BaseAudioEvent*>* liveEvents );
void collectDrumEvents( AudioChannel *channel, int bufferPosition, int bufferEnd );

#endif
