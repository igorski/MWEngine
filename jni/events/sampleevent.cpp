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
#include "sampleevent.h"
#include "../audioengine.h"
#include "../global.h"
#include "../sequencer.h"

/* constructor / destructor */

SampleEvent::SampleEvent()
{
    construct();
    init( 0 );
}

SampleEvent::SampleEvent( BaseInstrument* aInstrument )
{
    construct();
    init( aInstrument );
}

SampleEvent::~SampleEvent()
{
    delete _liveBuffer;
}

/* public methods */

void SampleEvent::play()
{
    _lastPlaybackPosition = _bufferRangeStart;
    BaseAudioEvent::play();
}

int SampleEvent::getBufferRangeStart()
{
    return _bufferRangeStart;
}

void SampleEvent::setBufferRangeStart( int value )
{
    _bufferRangeStart = ( _bufferRangeEnd > 0 ) ? std::min( value, _bufferRangeEnd - 1 ) : value;

    if ( _rangePointer < _bufferRangeStart )
        _rangePointer = _bufferRangeStart;

    if ( _bufferRangeEnd <= _bufferRangeStart )
        _bufferRangeEnd = std::max( _bufferRangeStart + ( _bufferRangeLength - 1 ), _bufferRangeStart );

    // buffer range may never exceed the length of the source buffer (which can be unequal to the sample length)

    if ( _buffer != 0 && _bufferRangeEnd >= _buffer->bufferSize )
        setBufferRangeEnd( _buffer->bufferSize - 1 );

    _bufferRangeLength = ( _bufferRangeEnd - _bufferRangeStart ) + 1;
    setRangeBasedPlayback( _bufferRangeLength != _eventLength );
}

int SampleEvent::getBufferRangeEnd()
{
    return _bufferRangeEnd;
}

void SampleEvent::setBufferRangeEnd( int value )
{
    // buffer range may never exceed the length of the source buffer (which can be unequal to the sample length)
    _bufferRangeEnd = ( _buffer != 0 ) ? std::min( value, _buffer->bufferSize - 1 ): value;

    if ( _rangePointer > _bufferRangeEnd )
        _rangePointer = _bufferRangeEnd;

    if ( _bufferRangeStart >= _bufferRangeEnd )
        _bufferRangeStart = std::max( _bufferRangeEnd - 1, 0 );

    _bufferRangeLength = ( _bufferRangeEnd - _bufferRangeStart ) + 1;
    setRangeBasedPlayback( _bufferRangeLength != _eventLength );
}

int SampleEvent::getBufferRangeLength()
{
    return _bufferRangeLength;
}

/**
 * return the current playback position for a live event
 * e.g. a SampleEvent triggered by play()
 */
int SampleEvent::getPlaybackPosition()
{
    return _lastPlaybackPosition;
}

/**
 * only invoked for a live event (see sequencer.cpp and audioengine.cpp)
 * or a SampleEvent triggered by play()
 */
AudioBuffer* SampleEvent::synthesize( int aBufferLength )
{
    if ( _liveBuffer == 0 )
        _liveBuffer = new AudioBuffer( _buffer->amountOfChannels, AudioEngineProps::BUFFER_SIZE );
    else
        _liveBuffer->silenceBuffers();  // clear previous contents

    int mergeLength = _liveBuffer->mergeBuffers( _buffer, _lastPlaybackPosition, 0, 1 );

    // if the SampleEvent is loopeable and the merge is smaller than given aBufferLength
    // (in other words: the full sample has been renderered, append from the beginning)

    if ( _loopeable && mergeLength < aBufferLength ) {
        _liveBuffer->mergeBuffers( _buffer, 0, mergeLength, 1 );
    }

    if (( _lastPlaybackPosition += aBufferLength ) > _bufferRangeEnd )
    {
        // if this is a one-shot SampleEvent, remove it from the sequencer when we have exceeded
        // the sample length (e.g. played it in its entirety)

        if ( !_loopeable )
            removeLiveEvent();
        else
            _lastPlaybackPosition -= _bufferRangeLength;
    }
    return _liveBuffer;
}

void SampleEvent::setSample( AudioBuffer* sampleBuffer )
{
    // make sure we lock read/write operations as setting a large sample
    // while the engine is running is a tad dangerous ;)

    bool wasLocked = _locked;
    _locked        = true;

    int sampleLength = sampleBuffer->bufferSize;

    // delete previous contents
    if ( _eventLength != sampleLength || _buffer == 0 )
        destroyBuffer();

    // is this events buffer destroyable ? then clone
    // the input buffer, if not, merely point to it to
    // minimize memory consumption when re-using existing samples

    if ( _destroyableBuffer )
        _buffer = sampleBuffer->clone();
    else
        _buffer = sampleBuffer;

    _buffer->loopeable = _loopeable;
    setEventLength( sampleLength );
    setEventEnd   ( _eventStart + ( _eventLength - 1 ));

    // when switching samples, existing buffer ranges are reset

    _bufferRangeStart  = 0;
    setBufferRangeEnd( _bufferRangeStart + ( _eventLength - 1 )); // also updates range length
    setRangeBasedPlayback( false );

    _updateAfterUnlock = false; // unnecessary

    if ( !wasLocked )
        _locked = false;
}

void SampleEvent::mixBuffer( AudioBuffer* outputBuffer, int bufferPos, int minBufferPosition, int maxBufferPosition,
                             bool loopStarted, int loopOffset, bool useChannelRange )
{
    // if we have a range length that is unequal to the total sample duration, read from the range
    // otherwise invoke the base mixBuffer method

    if ( _useBufferRange )
        getBufferForRange( outputBuffer, bufferPos );
    else
        BaseAudioEvent::mixBuffer( outputBuffer, bufferPos, minBufferPosition, maxBufferPosition,
                                   loopStarted, loopOffset, useChannelRange );
}

bool SampleEvent::getRangeBasedPlayback()
{
    return _useBufferRange;
}

void SampleEvent::setRangeBasedPlayback( bool value )
{
    _useBufferRange = value;
}

bool SampleEvent::getBufferForRange( AudioBuffer* buffer, int readPos )
{
    int bufferSize          = buffer->bufferSize;
    int amountOfChannels    = buffer->amountOfChannels;
    bool gotBuffer          = false;
    bool monoCopy           = _buffer->amountOfChannels < amountOfChannels;

    bool useInternalPointer = _loopeable;

    if ( useInternalPointer )
        readPos = _readPointer;

    SAMPLE_TYPE* srcBuffer;

    for ( int i = 0; i < bufferSize; ++i )
    {
        // read sample when the read pointer is within sample start and end points
        if ( readPos >= _eventStart && readPos <= _eventEnd )
        {
            // use range pointers to read within the specific sample ranges
            for ( int c = 0; c < amountOfChannels; ++c )
            {
                // this sample might have less channels than the output buffer
                if ( !monoCopy )
                    srcBuffer = _buffer->getBufferForChannel( c );
                else
                    srcBuffer = _buffer->getBufferForChannel( 0 );

                SAMPLE_TYPE* targetBuffer = buffer->getBufferForChannel( c );
                targetBuffer[ i ]        += ( srcBuffer[ _rangePointer ] * _volume );
            }

            if ( ++_rangePointer > _bufferRangeEnd )
                _rangePointer = _bufferRangeStart;

            gotBuffer = true;
        }

        // if this is a loopeable sample (thus using internal read pointer)
        // set the read pointer to the sample start so it keeps playing indefinitely

        if ( ++readPos > _eventEnd && _loopeable )
            readPos = _eventStart;
    }

    if ( useInternalPointer )
        _readPointer = readPos;

    return gotBuffer;
}

/* protected methods */

void SampleEvent::init( BaseInstrument* instrument )
{
    _bufferRangeLength     = 0;
    _rangePointer          = 0;
    _lastPlaybackPosition  = 0;
    _destroyableBuffer     = false; // is referenced via SampleManager !
    _useBufferRange        = false;
    _instrument            = instrument;
    _liveBuffer            = 0;
}
