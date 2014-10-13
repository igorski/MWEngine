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
#ifndef __BASESYNTHEVENT_H_INCLUDED__
#define __BASESYNTHEVENT_H_INCLUDED__

#include "basecacheableaudioevent.h"
#include "../adsr.h"
#include "../synthinstrument.h"
#include "../global.h"

/**
 * a SynthEvent describes a (cacheable) AudioEvent that can render
 * its audio when needed, it should basically combine rendering logic
 * into the basic AudioEvent interface
 */
class BaseSynthEvent : public BaseCacheableAudioEvent
{
    public:
        // constructors for base class, sequenced event and live-synthesis event
        BaseSynthEvent();
        BaseSynthEvent( float aFrequency, int aPosition, float aLength, SynthInstrument *aInstrument, bool aAutoCache );
        BaseSynthEvent( float aFrequency, SynthInstrument *aInstrument );

        ~BaseSynthEvent();

        // sequenced related properties
        bool isSequenced;
        int position;
        float length;

        // synthesis properties
        ADSR* getADSR();
        float getVolume();
        void setVolume( float aValue );
        float getFrequency();
        virtual void setFrequency( float aFrequency );

        // render related
        virtual void invalidateProperties( int aPosition, float aLength, SynthInstrument *aInstrument );
        virtual void calculateBuffers();
        virtual void mixBuffer( AudioBuffer* outputBuffer, int bufferPos, int minBufferPosition, int maxBufferPosition,
                                bool loopStarted, int loopOffset, bool useChannelRange );

        AudioBuffer* getBuffer();
        AudioBuffer* synthesize( int aBufferLength );

        // cache related
        virtual void cache( bool doCallback );
        void unlock();

    protected:

        // synthesis properties
        SynthInstrument* _instrument;
        SAMPLE_TYPE _frequency;
        float _volume;
        ADSR* _adsr;

        // properties for non-sequenced synthesis

        int _minLength;
        bool _hasMinLength;
        bool _queuedForDeletion;

        // setup related

        void init( SynthInstrument *aInstrument, float aFrequency, int aPosition,
                   int aLength, bool aIsSequenced );

        virtual void addToSequencer();
        virtual void removeFromSequencer();
        virtual void setDeletable( bool value );

        // render related
        virtual void render( AudioBuffer* outputBuffer );
        virtual void updateProperties();
        bool _rendering;
        bool _update;

        // caching
        void doCache();
};

#endif
