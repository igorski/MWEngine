#ifndef __AUDIOCHANNEL_H_INCLUDED__
#define __AUDIOCHANNEL_H_INCLUDED__

#include <vector>
#include "audiobuffer.h"
#include "baseaudioevent.h"
#include "sampleevent.h"
#include "processingchain.h"

class AudioChannel
{
    public:

        AudioChannel( ProcessingChain *aChain, double aMixVolume );
        AudioChannel( ProcessingChain *aChain, double aMixVolume, int aMaxBufferPosition );
        ~AudioChannel();

        // queried and modified by sequencer
        std::vector<SampleEvent*> audioEvents;
        std::vector<BaseAudioEvent*> liveEvents;

        ProcessingChain *processingChain;

        bool hasLiveEvents;
        bool isMono;
        bool muted;
        bool hasCache;
        bool isCaching;
        double mixVolume;

        void addEvent( SampleEvent* aEvent );

        /**
         * a channel can both contain buffered output as well
         * as live events ( for instance a keyboard plays the
         * same synthesizer as the sequenced patterns ). We need to
         * add them here so they can benefit from the same processing chain
         *
         * @param aLiveEvent {BaseAudioEvent} the live event
         */
        void addLiveEvent( BaseAudioEvent* aLiveEvent );

        /**
         * an AudioChannel may contain audio that spans a different
         * buffer range than the other channels in a Sequencer
         * (for instance: a looping one bar drum pattern over
         * four bars of synthesis)
         *
         * specify a max buffer position for the AudioChannel
         * to have the AudioRenderer loop its contents while
         * processing the whole buffer range for the other channels
         *
         * if the max buffer position is equal to the sequencer
         * max range, pass no value or 0 for maxBufferPosition
         */
        int maxBufferPosition;

        /**
         * clears all references to previously linked events
         */
        void reset();

        AudioBuffer* readCachedBuffer( AudioBuffer* aOutputBuffer, int aReadOffset );
        bool canCache();
        void canCache( bool value, int aBufferSize, int aCacheStartOffset, int aCacheEndOffset );
        void clearCachedBuffer();
        void writeCache( AudioBuffer* aBuffer, int aReadOffset );

        // to spare CPU resources an AudioChannel can cache its contents

    protected:

        void init();
        AudioBuffer* _cachedBuffer;
        bool _canCache;
        int _cacheReadPointer;
        int _cacheWritePointer;

        // the cache start and end offset specify at which buffer offset
        // the sequencer must read from the cache
        int _cacheStartOffset;
        int _cacheEndOffset;
};

#endif
