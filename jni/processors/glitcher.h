/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Igor Zinken - http://www.igorski.nl
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
#ifndef __GLITCHER_H_INCLUDED__
#define __GLITCHER_H_INCLUDED__

#include "baseprocessor.h"
#include "../audiobuffer.h"
#include <events/sampleevent.h>

class Glitcher : public BaseProcessor
{
    public:
        Glitcher( int amountOfChannels, int fragmentLengthInMilliseconds );
        ~Glitcher();

        void setRecording( bool value ); // whether to record input, also records during playback
        void setPlayback ( bool value ); // whether to play back from the recorded buffer
        void setPlaybackRange( int bufferStartPos, int bufferEndPos ); // the buffer range to play back

        int getSampleLength();

        // when recording, this will record the unmodified input signal into the buffer
        // if playback is true, the input signal is replaced by the pre-recorded buffer
        // if a custom buffer range is set, it is looped for maximum glitchiness !

        void process( AudioBuffer* sampleBuffer, bool isMonoSource );

    private:
        SampleEvent* _sample;   // the range based playback is proxied via a SampleEvent
        AudioBuffer* _buffer;   // used to record the input, is the buffer for SampleEvent _sample

        bool _recording;
        bool _playback;
        int  _writeOffset;
        int  _readOffset;
};

#endif
