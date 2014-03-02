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
#ifndef __SAMPLEEVENT_H_INCLUDED__
#define __SAMPLEEVENT_H_INCLUDED__

#include "basecacheableaudioevent.h"

class SampleEvent : public BaseCacheableAudioEvent
{
    public:
        SampleEvent();
        SampleEvent( int aPosition );
        virtual ~SampleEvent();

        // used by view representation
        int position;

        virtual int getSampleLength();
        virtual int getSampleStart();
        virtual int getSampleEnd();

        virtual int getBufferRangeStart();
        virtual void setBufferRangeStart( int value );
        virtual int getBufferRangeEnd();
        virtual void setBufferRangeEnd( int value );
        virtual int getBufferRangeLength();
        virtual void setBufferRangeLength( int value );
        virtual bool getLoopeable();
        virtual void setLoopeable( bool value );
        virtual int getReadPointer();

        virtual bool deletable();
        virtual void setDeletable( bool value );

        virtual AudioBuffer* getBuffer();
        virtual AudioBuffer* synthesize( int aBufferLength );
        virtual bool isCached();
        virtual void setAutoCache( bool aValue );
        virtual void cache();
        virtual void setSample( AudioBuffer* sampleBuffer );
        virtual void addToSequencer( int aSamplerNum );
        virtual void removeFromSequencer();

        bool getBufferForRange( AudioBuffer* buffer, int readPos );
        void playNow(); // enable the event and set it at the current sequencer position (auditioning purposes)

    protected:
        // total sample range
        int _sampleStart;
        int _sampleEnd;
        int _sampleLength;

        int _readPointer;
        int _rangePointer;
        bool _loopeable;

        // sample buffer regions (i.e. what is played)
        int _bufferRangeStart;
        int _bufferRangeEnd;
        int _bufferRangeLength;

        int _samplerNum; // identifier of the sampler playing this event

        // removal of AudioEvents must occur outside of the
        // cache loop, by activating this boolean we're queuing
        // the AudioEvent for removal

        bool _deleteMe;
        bool _cancel;            // whether we should cancel caching
        bool _cachingCompleted;  // whether we're done caching
        bool _addedToSequencer;  // whether this event is part of a sequence

        void init();
};

#endif
