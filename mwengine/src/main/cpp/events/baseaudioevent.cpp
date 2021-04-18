/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2020 Igor Zinken - https://www.igorski.nl
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
#include <global.h>
#include <audioengine.h>
#include <instruments/baseinstrument.h>
#include <utilities/bufferutility.h>
#include <utilities/eventutility.h>
#include <utilities/volumeutil.h>
#include <algorithm>

namespace MWEngine {

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

    if ( aInstrument != nullptr &&
        _instrument  != aInstrument )
    {
        bool wasAddedToSequencer = isAddedToSequencer();

        if ( wasAddedToSequencer )
            removeFromSequencer();

        _instrument = aInstrument;

        if ( wasAddedToSequencer )
            addToSequencer();
    }
}

void BaseAudioEvent::play()
{
    if ( _livePlayback || _instrument == nullptr ) {
        return;
    }
    setDeletable( false );

    // add this event to the live events list of the instrument (keep
    // the current sequenced event - if it was added - as is, we should
    // be able to audition - live play - an event that is part of a sequence)

    _instrument->addEvent( this, true );
    _livePlayback = true;
}

void BaseAudioEvent::stop()
{
    if ( !_livePlayback || _instrument == nullptr ) {
        return;
    }

    // remove this event from the live events list of the instrument (keep
    // the current sequenced event - if it was added - as is)

    _instrument->removeEvent( this, true );

    resetPlayState();
}

void BaseAudioEvent::resetPlayState()
{
    _livePlayback = false;
}

void BaseAudioEvent::addToSequencer()
{
    if ( isAddedToSequencer() ) {
        return;
    }

    // adds the event to the sequencer so it can be heard

    if ( isSequenced ) {
        _instrument->addEvent( this, false );
    }
    else {
        play();
    }
}

void BaseAudioEvent::removeFromSequencer()
{
    if ( _instrument == nullptr )
        return;

    if ( isSequenced ) {
        _instrument->removeEvent( this, false );
    }
    stop(); // event can be both sequenced as well as playing back live
}

int BaseAudioEvent::getEventLength()
{
    return _eventLength;
}

void BaseAudioEvent::setEventLength( int value )
{
    if ( _eventLength == value ) return;

    // if the events playback range is about to change, remove/add the event after the update
    // operation to ensure the instruments measure cache spans the correct range

    bool mustSyncWithInstrument = isAddedToSequencer();
    if ( mustSyncWithInstrument ) _instrument->removeEvent( this, false );

    _eventLength = value;

    // the existing event end must not be smaller than (or equal to)
    // the event start nor be smaller than the event length or
    // exceed the range set by the event start and event length

    if ( _eventEnd <= _eventStart ||
         _eventEnd <  ( _eventStart + _eventLength ) ||
         _eventEnd >= ( _eventStart + _eventLength ))
    {
        _eventEnd = _eventStart + ( _eventLength - 1 );
    }

    // update end position in seconds
    _endPosition = BufferUtility::bufferToSeconds( _eventEnd, AudioEngineProps::SAMPLE_RATE );

    if ( mustSyncWithInstrument ) _instrument->addEvent( this, false );
}

int BaseAudioEvent::getEventStart()
{
    return _eventStart;
}

void BaseAudioEvent::setEventStart( int value )
{
    if ( _eventStart == value ) return;

    // if the events playback range is about to change, remove/add the event after the update
    // operation to ensure the instruments measure cache spans the correct range

    bool mustSyncWithInstrument = isAddedToSequencer();
    if ( mustSyncWithInstrument ) _instrument->removeEvent( this, false );

    _eventStart = value;

    if ( _eventLength > 0 )
        _eventEnd = _eventStart + ( _eventLength - 1 );
    else if ( _eventEnd <= _eventStart )
        _eventEnd = _eventStart;

    // update end position in seconds
    _endPosition = BufferUtility::bufferToSeconds( _eventEnd, AudioEngineProps::SAMPLE_RATE );

    // update start position in seconds
    _startPosition = BufferUtility::bufferToSeconds( _eventStart, AudioEngineProps::SAMPLE_RATE );

    if ( mustSyncWithInstrument ) _instrument->addEvent( this, false );
}

int BaseAudioEvent::getEventEnd()
{
    return _eventEnd;
}

void BaseAudioEvent::setEventEnd( int value )
{
    if ( _eventEnd == value ) return;

    // if the events playback range is about to change, remove/add the event after the update
    // operation to ensure the instruments measure cache spans the correct range

    bool mustSyncWithInstrument = isAddedToSequencer();
    if ( mustSyncWithInstrument ) _instrument->removeEvent( this, false );

    // the event end cannot come before the start position
    _eventEnd = std::max( _eventStart, value );

    // it also cannot exceed beyond the start-plus-length
    // of the event (it can be smaller though for a cut-off playback)

    if ( _eventLength > 0 ) {
        _eventEnd = std::min( _eventEnd, _eventStart + ( _eventLength - 1 ));
    }

    // update end position in seconds
    _endPosition = BufferUtility::bufferToSeconds( _eventEnd, AudioEngineProps::SAMPLE_RATE );

    // this updates the length in case given end position is shorter than start + length
    int expectedLength = ( _eventEnd - _eventStart ) + 1;

    if ( _eventLength != expectedLength ) {
        _eventLength = expectedLength;
    }
    if ( mustSyncWithInstrument ) _instrument->addEvent( this, false );
}

void BaseAudioEvent::positionEvent( int startMeasure, int subdivisions, int offset )
{
    int samplesPerBar = AudioEngine::samples_per_bar; // will always match current tempo, time sig at right sample rate

    int startOffset = samplesPerBar * startMeasure;
    startOffset    += offset * samplesPerBar / subdivisions;

    setEventStart( startOffset );
}

void BaseAudioEvent::repositionToTempoChange( float ratio )
{
    // updating the start offset should automatically adjust the eventEnd accordingly
    // observe we keep the event length equal (BaseSynthEvent does adjust the length to the tempo)
    setEventStart(( int )( _eventStart  * ratio ));
}

float BaseAudioEvent::getStartPosition()
{
    return _startPosition;
}

void BaseAudioEvent::setStartPosition( float value )
{
    if ( _startPosition == value ) return;

    // update position in buffer samples (will assign given value to _startPosition)
    setEventStart( BufferUtility::secondsToBuffer( value, AudioEngineProps::SAMPLE_RATE ));
}

float BaseAudioEvent::getEndPosition()
{
    return _endPosition;
}

void BaseAudioEvent::setEndPosition( float value )
{
    if ( _endPosition == value ) return;

    // update position in buffer samples (will assign given value to _endPosition)
    setEventEnd( BufferUtility::secondsToBuffer( value, AudioEngineProps::SAMPLE_RATE ));
}

float BaseAudioEvent::getDuration()
{
    return _endPosition - _startPosition;
}

void BaseAudioEvent::setDuration( float value )
{
    setEndPosition( _startPosition + value );
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
    return VolumeUtil::toLinear( _volume );
}

float BaseAudioEvent::getVolumeLogarithmic()
{
    return _volume;
}

void BaseAudioEvent::setVolume( float value )
{
    _volume = VolumeUtil::toLog( value );
}

void BaseAudioEvent::mixBuffer( AudioBuffer* outputBuffer, int bufferPosition,
                                int minBufferPosition, int maxBufferPosition,
                                bool loopStarted, int loopOffset, bool useChannelRange )
{
    if ( !hasBuffer() )
        return;

    lock(); // prevents buffer mutations (from outside threads) during this read cycle

    int bufferSize = outputBuffer->bufferSize;

    // if the buffer channel amount differs from the output channel amount, we might
    // potentially have a bad time (e.g. engine has mono output while this event is stereo)
    // ideally events should never hold more channels than AudioEngineProps::OUTPUT_CHANNELS

    int outputChannels = outputBuffer->amountOfChannels;

    // but mixing mono events into multichannel output is OK
    bool mixMono = _buffer->amountOfChannels < outputChannels;

    int bufferPointer, readPointer, i, c;
    SAMPLE_TYPE* srcBuffer;
    SAMPLE_TYPE* tgtBuffer;

    // prevent overflowing allocated memory when reading from the source buffer
    int maxReadPos = _buffer->bufferSize;

    for ( i = 0; i < bufferSize; ++i )
    {
        bufferPointer = i + bufferPosition;

        // over the max position ? read from the start ( implies that sequence has started loop )
        if ( bufferPointer > maxBufferPosition )
        {
            if ( useChannelRange )  // TODO: channels use a min buffer position too ? (currently drummachine only)
                bufferPointer -= maxBufferPosition;

            else if ( !loopStarted )
                break;
        }

        if ( bufferPointer >= _eventStart && bufferPointer <= _eventEnd )
        {
            // mind the offset here ( source buffer starts at 0 while
            // the _eventStart defines where the event is positioned
            // subtract it from current sequencer pointer to get the
            // offset relative to the source buffer

            readPointer = bufferPointer - _eventStart;

            for ( c = 0; c < outputChannels; ++c )
            {
                srcBuffer = _buffer->getBufferForChannel( mixMono ? 0 : c );
                tgtBuffer = outputBuffer->getBufferForChannel( c );

                if ( readPointer < maxReadPos )
                    tgtBuffer[ i ] += ( srcBuffer[ readPointer ] * _volume );
            }
        }
        else if ( loopStarted && i >= loopOffset )
        {
            bufferPointer = minBufferPosition + ( i - loopOffset );

            if ( bufferPointer >= _eventStart && bufferPointer <= _eventEnd )
            {
                readPointer = bufferPointer - _eventStart;

                for ( c = 0; c < outputChannels; ++c )
                {
                    srcBuffer = _buffer->getBufferForChannel( mixMono ? 0 : c );
                    tgtBuffer = outputBuffer->getBufferForChannel( c );

                    tgtBuffer[ i ] += ( srcBuffer[ readPointer ] * _volume );
                }
            }
        }
    }
    unlock();   // release lock
}

/**
 * Invoked by the Sequencer in case this event isn't sequenced
 * but triggered manually via a "noteOn" / "noteOff" operation for instant "live" playback
 */
void BaseAudioEvent::mixBuffer( AudioBuffer* outputBuffer )
{
    // custom implementation should be implemented in derivative classes, see SampleEvent or BaseSynthEvent
}

AudioBuffer* BaseAudioEvent::getBuffer()
{
    return _buffer;
}

void BaseAudioEvent::setBuffer( AudioBuffer* buffer, bool destroyable )
{
    _destroyableBuffer = destroyable;
    destroyBuffer(); // clears existing buffer (if destroyable)
    _buffer = buffer;
}

bool BaseAudioEvent::hasBuffer()
{
    return _buffer != nullptr;
}

/* protected methods */

void BaseAudioEvent::construct()
{
    _buffer            = nullptr;
    _enabled           = true;
    _destroyableBuffer = true;
    _locked            = false;
    _volume            = VolumeUtil::toLog( 1.F );
    _eventStart        = 0;
    _eventEnd          = 0;
    _eventLength       = 0;
    _startPosition     = 0.F;
    _endPosition       = 0.F;
    _instrument        = nullptr;
    _deleteMe          = false;
    _livePlayback      = false;
    isSequenced        = true;
}

void BaseAudioEvent::destroyBuffer()
{
    if ( _destroyableBuffer && _buffer != nullptr )
    {
        delete _buffer;
        _buffer = nullptr;
    }
}

bool BaseAudioEvent::isAddedToSequencer()
{
    if ( _instrument != nullptr )
    {
        return EventUtility::vectorContainsEvent( _instrument->getEvents(), this );
    }
    return false;
}

/* TO BE DEPRECATED */

#ifndef SWIG
void BaseAudioEvent::setSampleLength( int value ) {
    setEventLength( value );
}

void BaseAudioEvent::setSampleStart( int value ) {
    setEventStart( value );
}

void BaseAudioEvent::setSampleEnd( int value ) {
    setEventEnd( value );
}

int BaseAudioEvent::getSampleLength() {
    return getEventLength();
}

int BaseAudioEvent::getSampleStart() {
    return getEventStart();
}

int BaseAudioEvent::getSampleEnd() {
    return getEventEnd();
}
#endif
/* E.O. DEPRECATION */

} // E.O namespace MWEngine
