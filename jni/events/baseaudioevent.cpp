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
#include "baseaudioevent.h"
#include "../global.h"
#include "../audioengine.h"
#include <instruments/baseinstrument.h>
#include <algorithm>

// constructors / destructor

BaseAudioEvent::BaseAudioEvent()
{
    construct();
}

BaseAudioEvent::BaseAudioEvent( BaseInstrument* instrument )
{
    construct();
    _instrument = instrument;
}

BaseAudioEvent::~BaseAudioEvent()
{
    removeFromSequencer();
    destroyBuffer();
}

/* public methods */

BaseInstrument* BaseAudioEvent::getInstrument()
{
    return _instrument;
}

void BaseAudioEvent::setInstrument( BaseInstrument* aInstrument )
{
    // swap instrument if new one is different to existing reference
    // additionally, if event was added to the sequencer, add it to the new
    // instruments sequenced events list

    if ( aInstrument != 0 &&
        _instrument  != aInstrument )
    {
        bool wasAddedToSequencer = _addedToSequencer;

        if ( wasAddedToSequencer )
            removeFromSequencer();

        _instrument = aInstrument;

        if ( wasAddedToSequencer )
            addToSequencer();
    }
}

void BaseAudioEvent::addToSequencer()
{
    if ( _addedToSequencer )
        return;

    // adds the event to the sequencer so it can be heard

    if ( isSequenced )
        _instrument->getEvents()->push_back( this );
    else
        _instrument->getLiveEvents()->push_back( this );

    _addedToSequencer = true;
}

void BaseAudioEvent::removeFromSequencer()
{
    if ( !_addedToSequencer )
        return;

    if ( !isSequenced )
    {
        std::vector<BaseAudioEvent*>* liveEvents = _instrument->getLiveEvents();
        std::vector<BaseAudioEvent*>::iterator position = std::find( liveEvents->begin(),
                                                                     liveEvents->end(), this );
        if ( position != liveEvents->end() )
            liveEvents->erase( position );
    }
    else
    {
        std::vector<BaseAudioEvent*>* events = _instrument->getEvents();
        std::vector<BaseAudioEvent*>::iterator position = std::find( events->begin(),
                                                                     events->end(), this );
        if ( position != events->end() )
            events->erase( position );
    }
    _addedToSequencer = false;
}

int BaseAudioEvent::getSampleLength()
{
    return _sampleLength;
}

void BaseAudioEvent::setSampleLength( int value )
{
    _sampleLength = value;
}

int BaseAudioEvent::getSampleStart()
{
    return _sampleStart;
}

void BaseAudioEvent::setSampleStart( int value )
{
    _sampleStart = value;
}

int BaseAudioEvent::getSampleEnd()
{
    return _sampleEnd;
}

void BaseAudioEvent::setSampleEnd( int value )
{
    _sampleEnd = value;
}

void BaseAudioEvent::positionEvent( int startMeasure, int subdivisions, int offset )
{
    int bytesPerBar = AudioEngine::bytes_per_bar; // will always match current tempo, time sig at right sample rate

    int startOffset = bytesPerBar * startMeasure;
    startOffset    += offset * bytesPerBar / subdivisions;

    setSampleStart( startOffset );
    setSampleEnd  ( startOffset + getSampleLength() );
}

bool BaseAudioEvent::isLoopeable()
{
    return _loopeable;
}

void BaseAudioEvent::setLoopeable( bool value )
{
    _loopeable = value;

    if ( _buffer != 0 )
        _buffer->loopeable = _loopeable;
}

bool BaseAudioEvent::isDeletable()
{
    return _deleteMe;
}

void BaseAudioEvent::setDeletable( bool value )
{
    _deleteMe = value;
}

bool BaseAudioEvent::isEnabled()
{
    return _enabled;
}

void BaseAudioEvent::setEnabled( bool value )
{
    _enabled = value;
}

void BaseAudioEvent::lock()
{
    _locked = true;
}

void BaseAudioEvent::unlock()
{
    _locked = false;

    if ( _updateAfterUnlock )
    {
        // override in subclass for custom update-logic
    }
    _updateAfterUnlock = false;
}

bool BaseAudioEvent::isLocked()
{
    return _locked;
}

float BaseAudioEvent::getVolume()
{
    return _volume;
}

void BaseAudioEvent::setVolume( float value )
{
    _volume = value;
}

void BaseAudioEvent::mixBuffer( AudioBuffer* outputBuffer, int bufferPosition,
                                int minBufferPosition, int maxBufferPosition,
                                bool loopStarted, int loopOffset, bool useChannelRange )
{
    lock(); // prevents buffer mutations (from outside threads) during this read cycle

    // read from the pre-cached buffer for sequenced notes

    int startOffset  = getSampleStart();
    int endOffset    = getSampleEnd();
    int sampleLength = getSampleLength();
    int bufferSize   = outputBuffer->bufferSize;

    // if the output channel amount differs from this events channel amount, we might
    // potentially have a bad time (e.g. engine has mono output while this event is stereo)
    // ideally events should never hold more channels than AudioEngineProps::OUTPUT_CHANNELS

    int outputChannels = std::min( _buffer->amountOfChannels, outputBuffer->amountOfChannels );

    for ( int i = 0; i < bufferSize; ++i )
    {
        int readPointer = i + bufferPosition;

        // over the max position ? read from the start ( implies that sequence has started loop )
        if ( readPointer >= maxBufferPosition )
        {
            if ( useChannelRange )  // TODO: channels use a min buffer position too ? (currently drummachine only)
                readPointer -= maxBufferPosition;

            else if ( !loopStarted )
                readPointer -= ( maxBufferPosition - minBufferPosition );
        }

        if ( readPointer >= startOffset && readPointer <= endOffset )
        {
            // mind the offset ! ( cached buffer starts at 0 while
            // the startOffset defines where the event is positioned in the sequencer )
            readPointer -= startOffset;

            for ( int c = 0; c < outputChannels; ++c )
            {
                SAMPLE_TYPE* srcBuffer = _buffer->getBufferForChannel( c );
                SAMPLE_TYPE* tgtBuffer = outputBuffer->getBufferForChannel( c );

                tgtBuffer[ i ] += ( srcBuffer[ readPointer ] * _volume );
            }
        }
        else
        {
            if ( loopStarted && i >= loopOffset )
            {
                readPointer = minBufferPosition + ( i - loopOffset );

                if ( readPointer >= startOffset && readPointer <= endOffset )
                {
                    readPointer -= startOffset;

                    for ( int c = 0, ca = _buffer->amountOfChannels; c < ca; ++c )
                    {
                        SAMPLE_TYPE* srcBuffer = _buffer->getBufferForChannel( c );
                        SAMPLE_TYPE* tgtBuffer = outputBuffer->getBufferForChannel( c );

                        tgtBuffer[ i ] += ( srcBuffer[ readPointer ] * _volume );
                    }
                }
            }
        }
    }
    unlock();   // release lock
}

AudioBuffer* BaseAudioEvent::getBuffer()
{
    return _buffer;
}

void BaseAudioEvent::setBuffer( AudioBuffer* buffer, bool destroyable )
{
    _destroyableBuffer = destroyable;
    destroyBuffer(); // clear existing buffer

    _buffer = buffer;
}

AudioBuffer* BaseAudioEvent::synthesize( int aBufferLength )
{
    // override in subclass as this memory allocation requires cleanup
    // (and basically contains nothing but silence ;-) ... !

    return new AudioBuffer( AudioEngineProps::OUTPUT_CHANNELS, aBufferLength );
}

/* protected methods */

void BaseAudioEvent::construct()
{
    _buffer            = 0;
    _enabled           = true;
    _destroyableBuffer = true;
    _loopeable         = false;
    _locked            = false;
    _addedToSequencer  = false;
    _volume            = MAX_PHASE;
    _sampleStart       = 0;
    _sampleEnd         = 0;
    _sampleLength      = 0;
    _instrument        = 0;
    _deleteMe          = false;
    isSequenced        = true;
}

void BaseAudioEvent::destroyBuffer()
{
    if ( _destroyableBuffer && _buffer != 0 )
    {
        delete _buffer;
        _buffer = 0;
    }
}
