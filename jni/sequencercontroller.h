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
#ifndef __SEQUENCERCONTROLLER_H_INCLUDED__
#define __SEQUENCERCONTROLLER_H_INCLUDED__

#include "sequencer.h"
#include <utilities/bulkcacher.h>

/**
 * SequencerAPI acts as the interface to control the Sequencers
 * speed, position, range, etc. it is basically the mediator between
 * the user interface and the Sequencer
 */
class SequencerController
{
    public:
        SequencerController();
        ~SequencerController();
        
        void prepare       ( int aBufferSize, int aSampleRate, float aQueuedTempo, int aTimeSigBeatAmount, int aTimeSigBeatUnit );
        float getTempo     ();
        void setTempo      ( float aTempo, int aTimeSigBeatAmount, int aTimeSigBeatUnit );
        void setTempoNow   ( float aTempo, int aTimeSigBeatAmount, int aTimeSigBeatUnit );
        void setVolume     ( float aVolume );
        void setPlaying    ( bool aPlaying );

        void setLoopRange  ( int aStartPosition, int aEndPosition );
        void setLoopRange  ( int aStartPosition, int aEndPosition, int aStepsPerBar );
        void updateMeasures( int aAmount, int aStepsPerBar );
        void setNotificationMarker( int aPosition );
        int  getPosition   ();
        void setPosition   ( int aPosition );
        void rewind        ();

        BulkCacher* getBulkCacher();
        void cacheAudioEventsForMeasure( int aMeasure );

        void setBounceState             ( bool aIsBouncing, int aMaxBuffers, char* aOutputDirectory );
        void setRecordingState          ( bool aRecording,  int aMaxBuffers, char* aOutputDirectory );
        void setRecordingFromDeviceState( bool aRecording,  int aMaxBuffers, char* aOutputDirectory );

    protected:
        int stepsPerBar;
};

#endif
