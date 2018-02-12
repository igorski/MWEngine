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
#ifndef __SAMPLEEVENT_H_INCLUDED__
#define __SAMPLEEVENT_H_INCLUDED__

#include "baseaudioevent.h"
#include <instruments/baseinstrument.h>

class SampleEvent : public BaseAudioEvent
{
    public:
        SampleEvent();
        SampleEvent( BaseInstrument* aInstrument );
        virtual ~SampleEvent();

        void play();

        virtual int getBufferRangeStart();
        virtual void setBufferRangeStart( int value );
        virtual int getBufferRangeEnd();
        virtual void setBufferRangeEnd( int value );
        virtual int getBufferRangeLength();

        virtual AudioBuffer* synthesize( int aBufferLength );

        void setSample( AudioBuffer* sampleBuffer );

        void mixBuffer( AudioBuffer* outputBuffer, int bufferPos, int minBufferPosition, int maxBufferPosition,
                        bool loopStarted, int loopOffset, bool useChannelRange );

        // whether to mix sample data from a specific range instead of the full sampleLength range

        bool getRangeBasedPlayback();
        void setRangeBasedPlayback( bool value );
        bool getBufferForRange( AudioBuffer* buffer, int readPos );

        int getPlaybackPosition();

    protected:

        // total sample range

        int _rangePointer;

        // sample buffer regions (i.e. what is played)
        int _bufferRangeStart;
        int _bufferRangeEnd;
        int _bufferRangeLength;
        bool _useBufferRange;

        AudioBuffer*    _liveBuffer;
        int _lastPlaybackPosition;

        void init( BaseInstrument* aInstrument );
};

#endif
