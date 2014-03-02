#ifndef __SEQUENCERAPI_H_INCLUDED__
#define __SEQUENCERAPI_H_INCLUDED__

#include "bulkcacher.h"

class SequencerAPI
{
    public:
        SequencerAPI();
        ~SequencerAPI();
        
        void prepare                   ( int aBufferSize, int aSampleRate, float aQueuedTempo, int aTimeSigBeatAmount, int aTimeSigBeatUnit );
        void setLoopPoint              ( int aStartPosition, int aEndPosition, int aStepsPerBar );
        void updateMeasures            ( int aAmount, int aStepsPerBar );
        void setTempo                  ( float aTempo, int aTimeSigBeatAmount, int aTimeSigBeatUnit );
        void setTempoNow               ( float aTempo, int aTimeSigBeatAmount, int aTimeSigBeatUnit );
        void setVolume                 ( float aVolume );
        void setPlaying                ( bool aPlaying );
        void rewind                    ();
        void setActiveDrumPattern      ( int activePattern );

        BulkCacher* getBulkCacher();
        void cacheAudioEventsForMeasure( int aMeasure );

        void setBounceState            ( bool aIsBouncing, int aMaxBuffers, char* aOutputDirectory );
        void setRecordingState         ( bool aRecording,  int aMaxBuffers, char* aOutputDirectory );
};

#endif
