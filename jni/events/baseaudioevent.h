/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2016 Igor Zinken - http://www.igorski.nl
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
#ifndef __BASEAUDIOEVENT_H_INCLUDED__
#define __BASEAUDIOEVENT_H_INCLUDED__

#include "../audiobuffer.h"

class BaseInstrument;  // forward declaration, see <instruments/baseinstrument.h>

class BaseAudioEvent
{
    public:
        BaseAudioEvent( BaseInstrument* instrument );
        BaseAudioEvent();
        ~BaseAudioEvent();

        /**
         * used by the AudioEngine to mix in parts of this
         * event buffer at a specific range
         *
         * outputBuffer describes the AudioBuffer this event will mix its contents into
         * bufferPosition is the current position (playback head) of the sequencer
         * minBufferPosition describes the minimum buffer position of the sequencers current loop range
         * maxBufferPosition describes the maximum buffer position of the sequencers current loop range
         * loopStarted describes whether the sequencer is about to loop (e.g. the current bufferPos + the
         *     bufferSize of given outputBuffer will be greater than the maxBufferPosition, meaning the
         *     sequencer will also require a render for the first samples at minBufferPosition (the amount of
         *     bufferSize samples left after having rendered the last samples up until maxBufferPosition)
         * loopOffset describes at which sample position the loop is started so we can calculate the amount of
         *     samples to render from the minBufferPosition)
         * useChannelRange whether the channel we're mixing into has its own range
         */
        virtual void mixBuffer( AudioBuffer* outputBuffer, int bufferPosition, int minBufferPosition,
                                int maxBufferPosition, bool loopStarted, int loopOffset, bool useChannelRange );

        /**
         * get / set the AudioBuffer for this event
         *
         * depending on the inheriting class type buffers can be
         * set / maintained internally (e.g. via a referenced Synthesizer, SampleManager, etc.)
         */
        virtual AudioBuffer* getBuffer();
        virtual void setBuffer( AudioBuffer* buffer, bool destroyable );
        virtual bool hasBuffer();

        /**
         * an AudioEvent can also synthesize audio live, this
         * method should be called during the write cycle on
         * each buffer update of the AudioRenderer
         * @param aBufferLength {int} desired buffer length ( usually buffer size )
         * @return {AudioBuffer*} the buffer containing the live generated samples
         */
        virtual AudioBuffer* synthesize( int aBufferLength );

        virtual BaseInstrument* getInstrument(); // retrieve reference to the instrument this event belongs to
        virtual void setInstrument( BaseInstrument* aInstrument ); // set / swap instrument this event belongs to

        virtual void addToSequencer();      // add / remove event from Instruments events list
        virtual void removeFromSequencer(); // adding it makes event eligible for playback via the sequencer
        bool isSequenced;                   // when true, this event plays back at a strict time within the sequencer
                                            // when false, this is a live event playing immediately (this has nothing
                                            // to do with this Event actually being added/removed from the sequencer!!)

        virtual int getSampleLength();
        virtual int getSampleStart();
        virtual int getSampleEnd();

        // position the AudioEvent within the Sequencer using buffer samples as unit
        // this allows for high precision positioning, see positionEvent() for a
        // positioning example using musical concepts

        virtual void setSampleLength( int value );
        virtual void setSampleStart( int value );
        virtual void setSampleEnd( int value );

        virtual int getReadPointer();

        // position the AudioEvent within the Sequencer using musical timing concepts
        // NOTE : this results in strict "on the grid" timing, using buffer samples instead (see setSampleStart() and
        // setSampleEnd() ) allows for more accurate positioning for drifting / swing / early / late events
        //
        // startMeasure describes which measure the event belongs to (NOTE : first measure starts at 0)
        // subdivisions describes the amount of "steps" in each measure (e.g. 16 for 16 steps within a single measure)
        // offset describes the offset at which to position the event, this is a subset of the amount of subdivisons
        //
        // examples :
        // ( 0, 16, 4 ) positions audioEvent at 4 / 16 = start of the 2nd quaver in the first measure
        // ( 1, 32, 4 ) positions audioEvent at 4 / 32 = 1/8th note in the second measure
        virtual void positionEvent( int startMeasure, int subdivisions, int offset );

        virtual bool isLoopeable();
        virtual void setLoopeable( bool value );

        virtual bool isDeletable();   // query whether this event is queued for deletion
        virtual void setDeletable( bool value );

        virtual bool isEnabled();   // whether this audio event is elligible for playback
        virtual void setEnabled( bool value );

        virtual void lock();        // "lock" mutations within this events buffer during reading
        virtual void unlock();      // unlock
        virtual bool isLocked();

        virtual float getVolume();
        virtual void  setVolume( float value );

    protected:

        void construct();   // basic initialization which can be shared across overloaded constructors

        // buffer regions
        int _sampleStart;
        int _sampleEnd;
        int _sampleLength;
        int _readPointer;  // when loopeable, used to internally keep track of last read buffer offset

        // properties
        bool _enabled;
        bool _loopeable;
        float _volume;

        bool _addedToSequencer;      // whether this event exists in the instruments event list (and is eligible for playback)
        BaseInstrument* _instrument; // the BaseInstrument this event belongs to

        // cached buffer
        AudioBuffer* _buffer;
        void destroyBuffer();

        bool _deleteMe;
        bool _locked;
        bool _updateAfterUnlock; // use in update-methods when checking for lock

        // _destroyableBuffer indicates we can delete the buffer on destruction (true by default and
        // implies that this AudioEvent holds the only reference to its buffers
        // contents (SampleEvents on the other hand might share equal content-buffers with
        // other (which point to the same memory in location, and should thus be
        // managed / disposed outside of this AudioEvent!)

        bool _destroyableBuffer;
};

#endif
