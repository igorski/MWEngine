/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2019 Igor Zinken - https://www.igorski.nl
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
#ifndef __MWENGINE__SAMPLEEVENT_H_INCLUDED__
#define __MWENGINE__SAMPLEEVENT_H_INCLUDED__

#include "baseaudioevent.h"
#include <instruments/baseinstrument.h>

namespace MWEngine {
class SampleEvent : public BaseAudioEvent
{
    public:
        SampleEvent();
        SampleEvent( BaseInstrument* aInstrument );
        virtual ~SampleEvent();

        void play();

        void setEventLength( int value );
        void setEventStart( int value );
        void setEventEnd( int value );

        virtual int getBufferRangeStart();
        virtual void setBufferRangeStart( int value );
        virtual int getBufferRangeEnd();
        virtual void setBufferRangeEnd( int value );
        virtual int getBufferRangeLength();

        // set the sample that this SampleEvent will playback
        // by default the sample will playback at the sampling rate of the engine

        bool setSample( AudioBuffer* sampleBuffer );

        // use this method in case your samples are at a different sampling rate
        // than the engine (for instance read from WAV file created externally)

        bool setSample( AudioBuffer* sampleBuffer, unsigned int sampleRate );

        float getPlaybackRate();
        void setPlaybackRate( float value );

        // use these to repeat this SampleEvents buffer for the total
        // event duration. Optionally specify the point at which the loop will start
        // for samples where the end and start offsets are not at a zero crossing
        // cross fading can be applied to prevent popping sounds on loop start

        bool isLoopeable();
        void setLoopeable( bool value, int crossfadeInMilliseconds );
        int getReadPointer();

        int getLoopStartOffset();
        void setLoopStartOffset( int value );
        int getLoopEndOffset();
        void setLoopEndOffset( int value );

        // custom override allowing the engine to get this events
        // length relative to this playback rate

        int getEventLength();

        int getOriginalEventLength(); // original, untransformed event length
        int getEventEnd();

        void mixBuffer( AudioBuffer* outputBuffer, int bufferPos, int minBufferPosition, int maxBufferPosition,
                        bool loopStarted, int loopOffset, bool useChannelRange );

        void mixBuffer( AudioBuffer* outputBuffer );

        // whether to mix sample data from a specific range instead of the full sampleLength range

        bool getRangeBasedPlayback();
        void setRangeBasedPlayback( bool value );
        bool getBufferForRange( AudioBuffer* buffer, int readPos );

        int getPlaybackPosition();

        // sample rate of the AudioBuffer that this sample event references
        // this can differ from the AudioEngine's sample rate

        unsigned int getSampleRate();

    protected:

        // total sample range

        int _rangePointer;
        float _rangePointerF;

        // looping / custom repeat range

        bool _loopeable;
        int _crossfadeMs;
        int _crossfadeStart; // the amount of samples to crossfade when about to loop
        int _crossfadeEnd;   // the amount of samples to crossfade when reading from the loop offset
        int _readPointer;    // when loopeable, used internally to keep track of last read buffer offset
        int _loopStartOffset;
        int _loopEndOffset;

        // sample buffer regions (i.e. the sample contents thar are played)

        int _bufferRangeStart;
        int _bufferRangeEnd;
        int _bufferRangeLength;
        bool _useBufferRange;
        float _playbackRate;
        float _readPointerF;

        unsigned int _sampleRate;

        AudioBuffer* _liveBuffer;
        int _lastPlaybackPosition;

        void init( BaseInstrument* aInstrument );
        void cacheFades();
};
} // E.O namespace MWEngine

#endif
