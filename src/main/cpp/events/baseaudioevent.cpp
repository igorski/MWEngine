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
#include "baseaudioevent.h"
#include "../global.h"
#include "../audioengine.h"
#include <instruments/baseinstrument.h>
#include <utilities/bufferutility.h>
#include <utilities/volumeutil.h>
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

void BaseAudioEvent::play()
{
    if ( _livePlayback || _instrument == 0 )
        return;

    setDeletable( false );

    // move this event to the live events list of the instrument (keep
    // the current sequenced event - if it was added - as is)

    _instrument->getLiveEvents()->push_back( this );
    _livePlayback = true;
}

void BaseAudioEvent::stop()
{
    if ( !_livePlayback )
        return;

    // remove this event from the live events list of the instrument (keep
    // the current sequenced event - if it was added - as is)

    removeLiveEvent();
    _livePlayback = false;
}

void BaseAudioEvent::addToSequencer()
{
    if ( _addedToSequencer )
        return;

    // adds the event to the sequencer so it can be heard

    if ( isSequenced )
        _instrument->getEvents()->push_back( this );
    else
        play();

    _addedToSequencer = true;
}

void BaseAudioEvent::removeFromSequencer()
{
    if ( !_addedToSequencer || _instrument == 0 )
        return;

    if ( !isSequenced )
    {
        stop();
    }
    else
    {
        std::vector<BaseAudioEvent*>* events = _instrument->getEvents();

        if ( events != 0 ) {
            std::vector<BaseAudioEvent*>::iterator position = std::find( events->begin(),
                                                                         events->end(), this );
            if ( position != events->end() )
                events->erase( position );
        }
    }
    _addedToSequencer = false;
}

void BaseAudioEvent::removeLiveEvent()
{
    std::vector<BaseAudioEvent*>* liveEvents = _instrument->getLiveEvents();
    std::vector<BaseAudioEvent*>::iterator position = std::find( liveEvents->begin(), liveEvents->end(), this );

    if ( position != liveEvents->end() )
        liveEvents->erase( position );
}

int BaseAudioEvent::getEventLength()
{
    return _eventLength;
}

void BaseAudioEvent::setEventLength( int value )
{
    _eventLength = value;

    // for non loopeable-events the existing event end must not
    // be smaller than (or equal to) the event start nor
    // be smaller than the event length or
    // exceed the range set by the event start and event length

    if ( !_loopeable )
    {
        if ( _eventEnd <= _eventStart ||
             _eventEnd <  ( _eventStart + _eventLength ) ||
             _eventEnd >= ( _eventStart + _eventLength ))
        {
            _eventEnd = _eventStart + ( _eventLength - 1 );
        }
    }
    // update end position in seconds
    _endPosition = BufferUtility::bufferToSeconds( _eventEnd, AudioEngineProps::SAMPLE_RATE );
}

int BaseAudioEvent::getEventStart()
{
    return _eventStart;
}

void BaseAudioEvent::setEventStart( int value )
{
    _eventStart = value;

    if ( _eventEnd <= _eventStart )
    {
        if ( !_loopeable && _eventLength > 0 )
            _eventEnd = _eventStart + ( _eventLength - 1 );
        else
            _eventEnd = _eventStart;

        // update end position in seconds
        _endPosition = BufferUtility::bufferToSeconds( _eventEnd, AudioEngineProps::SAMPLE_RATE );
    }
    // update start position in seconds
    _startPosition = BufferUtility::bufferToSeconds( _eventStart, AudioEngineProps::SAMPLE_RATE );
}

int BaseAudioEvent::getEventEnd()
{
    return _eventEnd;
}

void BaseAudioEvent::setEventEnd( int value )
{
    // for non loopeable-events the event end cannot exceed
    // beyond the start and the total event length (it can
    // be smaller though for a cut-off playback)

    if ( !_loopeable && value >= ( _eventStart + _eventLength ))
        _eventEnd = _eventStart + ( _eventLength - 1 );
    else
        _eventEnd = value;

    // update end position in seconds
    _endPosition = BufferUtility::bufferToSeconds( _eventEnd, AudioEngineProps::SAMPLE_RATE );
}

int BaseAudioEvent::getReadPointer()
{
    return _readPointer;
}

void BaseAudioEvent::positionEvent( int startMeasure, int subdivisions, int offset )
{
    int samplesPerBar = AudioEngine::samples_per_bar; // will always match current tempo, time sig at right sample rate

    int startOffset = samplesPerBar * startMeasure;
    startOffset    += offset * samplesPerBar / subdivisions;

    setEventStart( startOffset );
    setEventEnd  (( startOffset + _eventLength ) - 1 );
}

void BaseAudioEvent::setStartPosition( float value )
{
    _startPosition = value;

    if ( _endPosition < _startPosition ) {
        float duration = getDuration();
        setEndPosition(( duration > 0 ) ? value + duration : value );
    }

    // update position in buffer samples
    _eventStart  = BufferUtility::secondsToBuffer( _startPosition, AudioEngineProps::SAMPLE_RATE );
    _eventLength = std::max( 0, ( _eventEnd - 1 ) - _eventStart );
}

void BaseAudioEvent::setEndPosition( float value )
{
    _endPosition = value;

    if ( _endPosition < _startPosition ) {
        _endPosition = _startPosition;
    }

    // update position in buffer samples
    _eventEnd    = BufferUtility::secondsToBuffer( _endPosition, AudioEngineProps::SAMPLE_RATE );
    _eventLength = std::max( 0, ( _eventEnd - 1 ) - _eventStart );
}

void BaseAudioEvent::setDuration( float value )
{
    setEndPosition( _startPosition + value );
}

float BaseAudioEvent::getStartPosition()
{
    return _startPosition;
}

float BaseAudioEvent::getEndPosition()
{
    return _endPosition;
}

float BaseAudioEvent::getDuration()
{
    return _endPosition - _startPosition;
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

    // if the output channel amount differs from this events channel amount, we might
    // potentially have a bad time (e.g. engine has mono output while this event is stereo)
    // ideally events should never hold more channels than AudioEngineProps::OUTPUT_CHANNELS

    int outputChannels = std::min( _buffer->amountOfChannels, outputBuffer->amountOfChannels );
    int bufferPointer, readPointer, i, c, ca;
    SAMPLE_TYPE* srcBuffer;
    SAMPLE_TYPE* tgtBuffer;

    // non-loopeable event whose playback is tied to the Sequencer

    if ( !_loopeable )
    {
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
                    srcBuffer = _buffer->getBufferForChannel( c );
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
                        srcBuffer = _buffer->getBufferForChannel( c );
                        tgtBuffer = outputBuffer->getBufferForChannel( c );

                        tgtBuffer[ i ] += ( srcBuffer[ readPointer ] * _volume );
                    }
                }
            }
        }
    }
    else
    {
        // loopeable events mix their buffer contents using an internal read pointer

        bool monoCopy = _buffer->amountOfChannels < outputBuffer->amountOfChannels;
        int maxBufPos = _buffer->bufferSize - 1;

        for ( i = 0; i < bufferSize; ++i )
        {
            bufferPointer = i + bufferPosition;

            // read sample when the read pointer is within event start and end points
            if ( bufferPointer >= _eventStart && bufferPointer <= _eventEnd )
            {
                // use range pointers to read within the specific buffer ranges
                for ( c = 0, ca = _buffer->amountOfChannels; c < ca; ++c )
                {
                    // this sample might have less channels than the output buffer
                    if ( !monoCopy )
                        srcBuffer = _buffer->getBufferForChannel( c );
                    else
                        srcBuffer = _buffer->getBufferForChannel( 0 );

                    tgtBuffer       = outputBuffer->getBufferForChannel( c );
                    tgtBuffer[ i ] += ( srcBuffer[ _readPointer ] * _volume );
                }
                // this is a loopeable event (thus using internal read pointer)
                // set the internal read pointer to the event start so it keeps playing indefinitely

                if ( ++_readPointer > maxBufPos )
                    _readPointer = 0;

            }
            else if ( loopStarted && bufferPointer > maxBufferPosition )
            {
                // in case the Sequencers read offset exceeds the maximum and the
                // Sequencer is looping, read from start. internal _readPointer takes care of correct offset
                bufferPosition -= loopOffset;

                // decrement iterator as no write occurred in this iteration
                --i;
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
    destroyBuffer(); // clears existing buffer (if destroyable)
    _buffer = buffer;
}

bool BaseAudioEvent::hasBuffer()
{
    return _buffer != 0;
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
    _volume            = VolumeUtil::toLog( MAX_PHASE );
    _eventStart        = 0;
    _eventEnd          = 0;
    _eventLength       = 0;
    _readPointer       = 0;
    _startPosition     = 0.f;
    _endPosition       = 0.f;
    _instrument        = 0;
    _deleteMe          = false;
    _livePlayback      = false;
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

/* TO BE DEPRECATED */

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

/* E.O. DEPRECATION */
