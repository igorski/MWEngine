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

    // integer used for non altered playback rate

    if ( _rangePointer < _bufferRangeStart )
        _rangePointer = _bufferRangeStart;

    // floating point used for alternate playback rates

    if ( _rangePointerF < _bufferRangeStart )
        _rangePointerF = ( float ) _bufferRangeStart;

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
    return ( _playbackRate == 1.f ) ? _bufferRangeEnd : _bufferRangeStart + getBufferRangeLength();
}

void SampleEvent::setBufferRangeEnd( int value )
{
    // buffer range may never exceed the length of the source buffer (which can be unequal to the sample length)
    _bufferRangeEnd = ( _buffer != 0 ) ? std::min( value, _buffer->bufferSize - 1 ): value;

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
    if ( _liveBuffer == 0 )
        _liveBuffer = new AudioBuffer(
            ( _buffer != 0 ) ? _buffer->amountOfChannels : AudioEngineProps::OUTPUT_CHANNELS,
            AudioEngineProps::BUFFER_SIZE
        );
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

int SampleEvent::getEventLength()
{
    return ( _playbackRate == 1.f ) ? _eventLength : ( int )(( float ) _eventLength / _playbackRate );
}

int SampleEvent::getEventEnd()
{
    return ( _playbackRate == 1.f ) ? _eventEnd : _eventStart + getEventLength();
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

    if ( _playbackRate == 1.f ) {
        // use BaseAudioEvent behaviour if no custom playback rate is requested
        BaseAudioEvent::mixBuffer( outputBuffer, bufferPosition, minBufferPosition, maxBufferPosition,
                                   loopStarted, loopOffset, useChannelRange );
        return;
    }

    // custom playback rate

    int bufferSize = outputBuffer->bufferSize;

    // if the output channel amount differs from this events channel amount, we might
    // potentially have a bad time (e.g. engine has mono output while this event is stereo)
    // ideally events should never hold more channels than AudioEngineProps::OUTPUT_CHANNELS

    int outputChannels = std::min( _buffer->amountOfChannels, outputBuffer->amountOfChannels );
    int i, t, c, ca;
    float frac;
    SAMPLE_TYPE* srcBuffer;
    SAMPLE_TYPE* tgtBuffer;
    SAMPLE_TYPE s1, s2;

    // at custom playback rate we require floating point precision
    float eventStart = ( float ) _eventStart;
    float eventEnd   = ( float ) getEventEnd();
    float mbp = ( float ) maxBufferPosition;
    float lo  = ( float ) loopOffset;
    // iterator that increments by the playback rate
    float fi = 0.f;

    // take sequencer playhead position and determine what the position
    // should be relative to this events playback rate
    float bufferPointerStart = eventStart + ((( float ) bufferPosition - eventStart ) * _playbackRate );
    float bufferPointer, readPointer;

    // non-loopeable event whose playback is tied to the Sequencer

    if ( !_loopeable )
    {
        for ( i = 0; i < bufferSize; ++i, fi += _playbackRate )
        {
            // buffer pointer progresses by the playback rate
            bufferPointer = bufferPointerStart + fi;

            // over the max position ? read from the start ( implies that sequence has started loop )
            if ( bufferPointer > mbp )
            {
                if ( useChannelRange )
                    bufferPointer -= mbp;

                else if ( !loopStarted )
                    break;
            }

            if ( bufferPointer >= eventStart && bufferPointer <= eventEnd )
            {
                // mind the offset ! ( source buffer starts at 0 while
                // the _eventStart defines where the event is positioned
                // subtract it from current sequencer pointer to get the
                // offset relative to the source buffer

                readPointer = bufferPointer - eventStart;

                t    = ( int ) readPointer;
                frac = readPointer - t; // between 0 - 1 range

                for ( c = 0; c < outputChannels; ++c )
                {
                    srcBuffer = _buffer->getBufferForChannel( c );
                    tgtBuffer = outputBuffer->getBufferForChannel( c );

                    s1 = srcBuffer[ t ];
                    s2 = srcBuffer[ t + 1 ];

                    tgtBuffer[ i ] += (( s1 + ( s2 - s1 ) * frac ) * _volume );
                }
            }
            else if ( loopStarted && fi >= lo )
            {
                bufferPointer = ( float )( minBufferPosition + ( fi - lo ));

                if ( bufferPointer >= eventStart && bufferPointer <= eventEnd )
                {
                    readPointer = bufferPointer - eventStart;

                    t    = ( int ) readPointer;
                    frac = readPointer - t; // between 0 - 1 range

                    for ( c = 0; c < outputChannels; ++c )
                    {
                        srcBuffer = _buffer->getBufferForChannel( c );
                        tgtBuffer = outputBuffer->getBufferForChannel( c );

                        s1 = srcBuffer[ t ];
                        s2 = srcBuffer[ t + 1 ];

                        tgtBuffer[ i ] += (( s1 + ( s2 - s1 ) * frac ) * _volume );
                    }
                }
            }
        }
    }
    else
    {
        // loopeable events mix their buffer contents using an internal read pointer

        bool monoCopy      = _buffer->amountOfChannels < outputBuffer->amountOfChannels;
        float maxBufPos    = ( float ) _buffer->bufferSize - 1.f;
        bufferPointerStart = _readPointerF; // use internal read pointer when reading loopeable content

        for ( i = 0; i < bufferSize; ++i, fi += _playbackRate )
        {
            // buffer pointer progresses by the playback rate
            bufferPointer = bufferPointerStart + fi;

            t    = ( int ) _readPointerF;
            frac = _readPointerF - t; // between 0 - 1 range

            // read sample when the read pointer is within event start and end points
            if ( bufferPointer >= eventStart && bufferPointer <= eventEnd )
            {
                // use range pointers to read within the specific buffer ranges
                for ( c = 0, ca = _buffer->amountOfChannels; c < ca; ++c )
                {
                    // this sample might have less channels than the output buffer
                    if ( !monoCopy )
                        srcBuffer = _buffer->getBufferForChannel( c );
                    else
                        srcBuffer = _buffer->getBufferForChannel( 0 );

                    tgtBuffer = outputBuffer->getBufferForChannel( c );

                    s1 = srcBuffer[ t ];
                    s2 = srcBuffer[ t + 1 ];

                    tgtBuffer[ i ] += (( s1 + ( s2 - s1 ) * frac ) * _volume );
                }

                // this is a loopeable event (thus using internal read pointer)
                // set the internal read pointer to the event start so it keeps playing indefinitely

                if (( _readPointerF += _playbackRate ) > maxBufPos )
                    _readPointerF = 0.f;
            }
            else if ( loopStarted && bufferPointer > mbp )
            {
                // in case the Sequencers read offset exceeds the maximum and the
                // Sequencer is looping, read from start. internal _readPointerF takes care of correct offset
                bufferPointerStart -= lo;

                // decrement iterator as no write occurred in this iteration
                --i;
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
        float bufferRangeEnd = ( float ) getBufferRangeEnd(),
              frac;

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
    _rangePointer          = 0;     // integer for non altered playback rates
    _rangePointerF         = 0.f;   // floating point for alternate playback rates
    _lastPlaybackPosition  = 0;
    _playbackRate          = 1.f;
    _readPointerF          = 0.f;
    _destroyableBuffer     = false; // is referenced via SampleManager !
    _useBufferRange        = false;
    _instrument            = instrument;
    _liveBuffer            = 0;
    _sampleRate            = AudioEngineProps::SAMPLE_RATE;
}
