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
#include <utilities/bufferutility.h>
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
    _liveBuffer = nullptr;
}

/* public methods */

void SampleEvent::play()
{
    // when invoking play() ensure the read pointers
    // are back at the start (when looping, at the beginning of
    // the entire sample, when playing from a range, at the beginning of the range)
    _readPointer          = std::max( 0, _bufferRangeStart );
    _readPointerF         = ( float ) _readPointer;
    _lastPlaybackPosition = _bufferRangeStart;

    BaseAudioEvent::play();
}

void SampleEvent::setEventLength( int value )
{
    _eventLength = value;

    if ( _loopeable ) {

        // loopeable-events differ from non-loopable events in that
        // we allow the events end to exceed the range of its start
        // plus the total sample duration (the delta will be filled with
        // looped content)

        if ( _eventEnd <= _eventStart ||
             _eventEnd <  ( _eventStart + _eventLength ))
        {
            _eventEnd = _eventStart + ( _eventLength - 1 );
        }

        // update end position in seconds
        _endPosition = BufferUtility::bufferToSeconds( _eventEnd, AudioEngineProps::SAMPLE_RATE );
    }
    else {
        BaseAudioEvent::setEventLength( value );
    }
}

void SampleEvent::setEventStart( int value )
{
    _eventStart = value;

    // assume length remains unchanged (e.g. play full sample)
    _eventEnd = _eventStart + _eventLength;

    // update start and end positions in seconds
    _startPosition = BufferUtility::bufferToSeconds( _eventStart, AudioEngineProps::SAMPLE_RATE );
    _endPosition   = BufferUtility::bufferToSeconds( _eventEnd,   AudioEngineProps::SAMPLE_RATE );
}

void SampleEvent::setEventEnd( int value )
{
    if ( !_loopeable ) {
        BaseAudioEvent::setEventEnd( value );
        return;
    }

    _eventEnd = value;

    // update end position in seconds
    _endPosition = BufferUtility::bufferToSeconds( _eventEnd, AudioEngineProps::SAMPLE_RATE );
}

int SampleEvent::getBufferRangeStart()
{
    return _bufferRangeStart;
}

void SampleEvent::setBufferRangeStart( int value )
{
    _bufferRangeStart = ( _bufferRangeEnd > 0 ) ? std::min( value, _bufferRangeEnd - 1 ) : value;

    // integer used for non altered playback rate

    if ( _rangePointer < _bufferRangeStart )
        _rangePointer = _bufferRangeStart;

    // floating point used for alternate playback rates

    if ( _rangePointerF < _bufferRangeStart )
        _rangePointerF = ( float ) _bufferRangeStart;

    if ( _bufferRangeEnd <= _bufferRangeStart )
        _bufferRangeEnd = std::max( _bufferRangeStart + ( _bufferRangeLength - 1 ), _bufferRangeStart );

    // buffer range may never exceed the length of the source buffer (which can be unequal to the sample length)

    if ( _buffer != nullptr && _bufferRangeEnd >= _buffer->bufferSize )
        setBufferRangeEnd( _buffer->bufferSize - 1 );

    _bufferRangeLength = ( _bufferRangeEnd - _bufferRangeStart ) + 1;
    setRangeBasedPlayback( _bufferRangeLength != _eventLength );
}

int SampleEvent::getBufferRangeEnd()
{
    return ( _playbackRate == 1.f ) ? _bufferRangeEnd : _bufferRangeStart + getBufferRangeLength();
}

void SampleEvent::setBufferRangeEnd( int value )
{
    // buffer range may never exceed the length of the source buffer (which can be unequal to the sample length)
    _bufferRangeEnd = ( _buffer != nullptr ) ? std::min( value, _buffer->bufferSize - 1 ): value;

    if ( _rangePointer > _bufferRangeEnd )
        _rangePointer = _bufferRangeEnd;

    if ( _rangePointerF > getBufferRangeEnd() )
        _rangePointerF = ( float ) getBufferRangeEnd();

    if ( _bufferRangeStart >= _bufferRangeEnd )
        _bufferRangeStart = std::max( _bufferRangeEnd - 1, 0 );

    _bufferRangeLength = ( _bufferRangeEnd - _bufferRangeStart ) + 1;
    setRangeBasedPlayback( getBufferRangeLength() != getEventLength() );
}

int SampleEvent::getBufferRangeLength()
{
    return ( _playbackRate == 1.f ) ? _bufferRangeLength : ( int )(( float ) _bufferRangeLength / _playbackRate );
}

/**
 * return the current playback position for a live event
 * e.g. a SampleEvent triggered by play()
 */
int SampleEvent::getPlaybackPosition()
{
    return _lastPlaybackPosition;
}

unsigned int SampleEvent::getSampleRate()
{
    return _sampleRate;
}

/**
 * only invoked for a live event (see sequencer.cpp and audioengine.cpp)
 * or a SampleEvent triggered by play()
 */
AudioBuffer* SampleEvent::synthesize( int aBufferLength )
{
    if ( _liveBuffer == nullptr )
        _liveBuffer = new AudioBuffer( AudioEngineProps::OUTPUT_CHANNELS, AudioEngineProps::BUFFER_SIZE );
    else
        _liveBuffer->silenceBuffers();  // clear previous contents

    // write sample contents into live buffer
    // we specify the maximum buffer position as the full sample playback range
    mixBuffer( _liveBuffer, _lastPlaybackPosition, 0, getBufferRangeLength(), false, 0, false );

    if (( _lastPlaybackPosition += aBufferLength ) >= getBufferRangeEnd() )
    {
        // if this is a one-shot SampleEvent, remove it from the sequencer when we have exceeded
        // the sample length (e.g. played it in its entirety)

        if ( !_loopeable )
            removeLiveEvent();
        else
            _lastPlaybackPosition = std::max( _bufferRangeStart, _lastPlaybackPosition - getBufferRangeLength());
    }
    return _liveBuffer;
}

void SampleEvent::setSample( AudioBuffer* sampleBuffer )
{
    setSample( sampleBuffer, AudioEngineProps::SAMPLE_RATE );
}

void SampleEvent::setSample( AudioBuffer* sampleBuffer, unsigned int sampleRate )
{
    // make sure we lock read/write operations as setting a sample
    // while the engine is running (thus reading from the current one) is a tad dangerous ;)

    bool wasLocked = _locked;
    _locked        = true;

    int sampleLength = sampleBuffer->bufferSize;

    // delete previous contents
    if ( _eventLength != sampleLength )
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

    // in case the given event has a sample rate that differs from the engine
    // adjust the playback rate of the sample accordingly

    _sampleRate = sampleRate;
    if ( _sampleRate != AudioEngineProps::SAMPLE_RATE ) {
        setPlaybackRate( _playbackRate / ( float ) AudioEngineProps::SAMPLE_RATE * ( float ) _sampleRate );
    }

    // when switching samples, existing buffer ranges are reset

    _bufferRangeStart  = 0;
    setBufferRangeEnd( _bufferRangeStart + ( _eventLength - 1 )); // also updates range length
    setRangeBasedPlayback( false );

    _updateAfterUnlock = false; // unnecessary

    if ( !wasLocked )
        _locked = false;
}

float SampleEvent::getPlaybackRate()
{
    return _playbackRate;
}

void SampleEvent::setPlaybackRate( float value )
{
    // allow only 100x slowdown and speed up
    _playbackRate = std::max( 0.01f, std::min( 100.f, value ));
}

bool SampleEvent::isLoopeable()
{
    return _loopeable;
}

void SampleEvent::setLoopeable( bool value )
{
    _loopeable = value;

    if ( _buffer != nullptr )
        _buffer->loopeable = _loopeable;
}

int SampleEvent::getReadPointer()
{
    return _readPointer;
}

int SampleEvent::getLoopStartOffset()
{
    return _loopStartOffset;
}

void SampleEvent::setLoopStartOffset( int value )
{
    _loopStartOffset = std::min( value, std::max( 0, _eventLength - 1 ));
}

int SampleEvent::getEventLength()
{
    return ( _playbackRate == 1.f || _loopeable ) ? _eventLength : ( int )(( float ) _eventLength / _playbackRate );
}

int SampleEvent::getOriginalEventLength()
{
    return _eventLength;
}

int SampleEvent::getEventEnd()
{
    return ( _playbackRate == 1.f || _loopeable ) ? _eventEnd : _eventStart + getEventLength();
}

void SampleEvent::mixBuffer( AudioBuffer* outputBuffer, int bufferPosition,
                             int minBufferPosition, int maxBufferPosition,
                             bool loopStarted, int loopOffset, bool useChannelRange )
{
    if ( !hasBuffer() )
        return;

    // if we have a range length that is unequal to the total sample duration, read from the range
    // otherwise invoke the base mixBuffer method

    if ( _useBufferRange ) {
        getBufferForRange( outputBuffer, bufferPosition );
        return;
    }

    int bufferSize = outputBuffer->bufferSize;

    // if the buffer channel amount differs from the output channel amount, we might
    // potentially have a bad time (e.g. engine has mono output while this event is stereo)
    // ideally events should never hold more channels than AudioEngineProps::OUTPUT_CHANNELS

    int outputChannels = outputBuffer->amountOfChannels;

    // but mixing mono events into multichannel output is OK
    bool mixMono = _buffer->amountOfChannels < outputChannels;

    if ( _playbackRate == 1.f )
    {
        // use BaseAudioEvent behaviour if no custom playback rate nor looping is set

        if ( !_loopeable ) {
            BaseAudioEvent::mixBuffer( outputBuffer, bufferPosition, minBufferPosition,
                                       maxBufferPosition, loopStarted, loopOffset, useChannelRange );
        }
        else
        {
            // loopeable events mix their buffer contents using an internal read pointer

            int maxBufPos = _buffer->bufferSize - 1;
            int bufferPointer, i, c, ca;
            SAMPLE_TYPE* srcBuffer;
            SAMPLE_TYPE* tgtBuffer;

            for ( i = 0; i < bufferSize; ++i )
            {
                bufferPointer = ( loopStarted && i >= loopOffset ) ? minBufferPosition + ( i - loopOffset ) : i + bufferPosition;

                // read sample when the read pointer is within event start and end points

                if ( bufferPointer >= _eventStart && bufferPointer <= _eventEnd )
                {
                    // when playing event from the beginning, ensure that its looped sample is playing from the beginning

                    if ( bufferPointer == _eventStart && !_livePlayback )
                        _readPointer = 0;

                    // use range pointers to read within the specific buffer ranges
                    for ( c = 0, ca = outputChannels; c < ca; ++c )
                    {
                        srcBuffer = _buffer->getBufferForChannel( mixMono ? 0 : c );

                        tgtBuffer       = outputBuffer->getBufferForChannel( c );
                        tgtBuffer[ i ] += ( srcBuffer[ _readPointer ] * _volume );
                    }
                    // this is a loopeable event (thus using internal read pointer)
                    // set the internal read pointer to the loop start so it keeps playing indefinitely

                    if ( ++_readPointer > maxBufPos )
                        _readPointer = _loopStartOffset;
                }
            }
        }
        return;
    }

    // custom playback rate

    int i, t, t2, c, ca;
    float frac;
    SAMPLE_TYPE* srcBuffer;
    SAMPLE_TYPE* tgtBuffer;
    SAMPLE_TYPE s1, s2;

    // maximum allowed source buffer read offset
    // due to rounding of floating point increments, we need
    // to ensure we remain in range otherwise we will overflow allocated memory ranges

    int maxPos = _buffer->bufferSize - 1;

    // at custom playback rate we require floating point precision for these properties
    // also we translate the values relative to the playback speed

    float fEventStart        = ( float ) _eventStart;
    float fEventEnd          = ( float ) getEventEnd();
    float fMinBufferPosition = ( float ) minBufferPosition;
    float fMaxBufferPosition = ( float ) maxBufferPosition * _playbackRate;
    float fLoopOffset        = ( float ) loopOffset * _playbackRate;

    // iterator that increments by the playback rate
    float fi = 0.f;

    // take sequencer playhead position and determine what the position
    // should be relative to this events playback rate
    float fBufferPosition = fEventStart + ((( float ) bufferPosition - fEventStart ) * _playbackRate );
    float fBufferPointer, fReadPointer;

    // non-loopeable event whose playback is tied to the Sequencer

    if ( !_loopeable )
    {
        fEventEnd = _eventEnd; // use unstretched end (see below fBufferPointer calculation)

        for ( i = 0; i < bufferSize; ++i, fi += _playbackRate )
        {
            // NOTE buffer pointer progresses by the playback rate

            fBufferPointer = ( loopStarted && fi >= fLoopOffset ) ? fMinBufferPosition + ( fi - fLoopOffset ) : fBufferPosition + fi;

            // over the max position ? read from the start ( implies that sequence has started loop )
            if ( fBufferPointer > fMaxBufferPosition )
            {
                if ( useChannelRange )
                    fBufferPointer -= fMaxBufferPosition;
                else
                    break;
            }

            if ( fBufferPointer >= fEventStart && fBufferPointer <= fEventEnd )
            {
                // mind the offset ! ( source buffer starts at 0 while
                // the eventStart defines where the event is positioned )
                // subtract it from current sequencer position to get the
                // offset relative to the source buffer

                fReadPointer = fBufferPointer - fEventStart;

                t    = ( int ) fReadPointer;
                frac = fReadPointer - t; // between 0 - 1 range

                for ( c = 0; c < outputChannels; ++c )
                {
                    srcBuffer = _buffer->getBufferForChannel( mixMono ? 0 : c );
                    tgtBuffer = outputBuffer->getBufferForChannel( c );

                    t2 = t + 1;

                    if ( t2 > maxPos )
                        break;

                    s1 = srcBuffer[ t ];
                    s2 = srcBuffer[ t2 ];

                    tgtBuffer[ i ] += (( s1 + ( s2 - s1 ) * frac ) * _volume );
                }
            }
        }
    }
    else
    {
        // loopeable events mix their buffer contents using an internal read pointer

        float fMaxPos = ( float ) _buffer->bufferSize - 1.f;

        if ( _livePlayback )
            fBufferPosition = _readPointerF; // use internal read pointer when reading loopeable content

        for ( i = 0; i < bufferSize; ++i, fi += _playbackRate )
        {
            // when playing event from the beginning (e.g. "(re)trigger"), ensure that its looped
            // sample is playing from the beginning too. We use the non-rate adjusted iterators
            // to determine this, as they are locked to the Sequencer which is responsible
            // for these (re)triggers

            if (( loopStarted && i >= loopOffset )) {
                _readPointerF   = 0.0f;
                fBufferPosition = 0.0f;
                fi              = 0.0f;
            }
            else if ( !_livePlayback && ( bufferPosition + i ) == _eventStart ) {
                _readPointerF = 0.0f;
                fi            = 0.0f;
            }

            // NOTE buffer pointer progresses by the playback rate

            fBufferPointer = fBufferPosition + fi;

            // read sample when the read pointer is within event start and end points
            if ( fBufferPointer >= fEventStart && fBufferPointer <= fEventEnd )
            {
                fReadPointer = fBufferPointer - fEventStart;

                // max pos describes the max position within the source buffer
                // when looping, we start reading from the loop start offset again

                if ( fReadPointer > fMaxPos ) {
                    fReadPointer = + ( float ) _loopStartOffset + fmod( fReadPointer, fMaxPos - ( float ) _loopStartOffset );
                }

                t    = ( int ) fReadPointer;
                frac = fReadPointer - t; // between 0 - 1 range

                // use range pointers to read within the specific buffer ranges
                for ( c = 0, ca = outputChannels; c < ca; ++c )
                {
                    srcBuffer = _buffer->getBufferForChannel( mixMono ? 0 : c );
                    tgtBuffer = outputBuffer->getBufferForChannel( c );

                    t2 = t + 1;

                    if ( t2 > maxPos )
                        break;

                    s1 = srcBuffer[ t ];
                    s2 = srcBuffer[ t2 ];

                    tgtBuffer[ i ] += (( s1 + ( s2 - s1 ) * frac ) * _volume );
                }

                // this is a loopeable event (thus using internal read pointer)
                // set the internal read pointer to the loop start so it keeps playing indefinitely

                if (( _readPointerF += _playbackRate ) > fMaxPos )
                    _readPointerF = ( float ) _loopStartOffset;
            }
        }
    }
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

    int eventStart = _eventStart;
    int eventEnd   = getEventEnd();

    SAMPLE_TYPE* srcBuffer;

    if ( _playbackRate == 1.f )
    {
        for ( int i = 0; i < bufferSize; ++i )
        {
            // read sample when the read pointer is within sample start and end points
            if ( readPos >= eventStart && readPos <= eventEnd )
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

            if ( ++readPos > eventEnd && _loopeable )
                readPos = eventStart;
        }
    }
    else {

        // custom playback speed

        int t;
        SAMPLE_TYPE s1, s2;
        float bufferRangeEnd = ( float ) getBufferRangeEnd();
        float frac;

        for ( int i = 0; i < bufferSize; ++i )
        {
            // read sample when the read pointer is within sample start and end points
            if ( readPos >= eventStart && readPos <= eventEnd )
            {
                t    = ( int ) _rangePointerF;
                frac = _rangePointerF - t; // between 0 - 1 range

                // use range pointers to read within the specific sample ranges
                for ( int c = 0; c < amountOfChannels; ++c )
                {
                    // this sample might have less channels than the output buffer
                    if ( !monoCopy )
                        srcBuffer = _buffer->getBufferForChannel( c );
                    else
                        srcBuffer = _buffer->getBufferForChannel( 0 );

                    s1 = srcBuffer[ t ];
                    s2 = srcBuffer[ t + 1 ];

                    SAMPLE_TYPE* targetBuffer = buffer->getBufferForChannel( c );
                    targetBuffer[ i ]        += (( s1 + ( s2 - s1 ) * frac ) * _volume );
                }

                if (( _rangePointerF += _playbackRate ) > bufferRangeEnd )
                    _rangePointerF = ( float ) _bufferRangeStart;

                gotBuffer = true;
            }

            // if this is a loopeable sample (thus using internal read pointer)
            // set the read pointer to the sample start so it keeps playing indefinitely

            if ( ++readPos > eventEnd && _loopeable )
                readPos = eventStart;
        }
    }

    if ( useInternalPointer )
        _readPointer = readPos;

    return gotBuffer;
}

/* protected methods */

void SampleEvent::init( BaseInstrument* instrument )
{
    _bufferRangeStart      = 0;
    _bufferRangeEnd        = 0;
    _bufferRangeLength     = 0;
    _loopeable             = false;
    _readPointer           = 0;
    _loopStartOffset       = 0;
    _rangePointer          = 0;     // integer for non altered playback rates
    _rangePointerF         = 0.f;   // floating point for alternate playback rates
    _lastPlaybackPosition  = 0;
    _playbackRate          = 1.f;
    _readPointerF          = 0.f;
    _destroyableBuffer     = false; // is referenced via SampleManager !
    _useBufferRange        = false;
    _instrument            = instrument;
    _liveBuffer            = nullptr;
    _sampleRate            = ( unsigned int ) AudioEngineProps::SAMPLE_RATE;
}
