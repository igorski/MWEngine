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
#include "sampleevent.h"
#include "../audioengine.h"
#include "../sequencer.h"
#include "../utilities/utils.h"
#include <instruments/sampledinstrument.h>

/* constructor / destructor */

SampleEvent::SampleEvent()
{
    init( 0 );
}

SampleEvent::SampleEvent( BaseInstrument* aInstrument )
{
    init( aInstrument );
}

SampleEvent::~SampleEvent()
{
    removeFromSequencer();
}

/* public methods */

int SampleEvent::getBufferRangeStart()
{
    return _bufferRangeStart;
}

void SampleEvent::setBufferRangeStart( int value )
{
    _bufferRangeStart = value;

    if ( _rangePointer < _bufferRangeStart )
        _rangePointer = _bufferRangeStart;
}

int SampleEvent::getBufferRangeEnd()
{
    return _bufferRangeEnd;
}

void SampleEvent::setBufferRangeEnd( int value )
{
    _bufferRangeEnd = value;

    if ( _rangePointer > _bufferRangeEnd )
        _rangePointer = _bufferRangeEnd;
}

int SampleEvent::getBufferRangeLength()
{
    return _bufferRangeLength;
}

void SampleEvent::setBufferRangeLength( int value )
{
    _bufferRangeLength = value;
}

int SampleEvent::getReadPointer()
{
    return _readPointer;
}

void SampleEvent::playNow()
{
    // loopeable samples play from their range start, whereas
    // non-loopeable samples are enqueued to the current sequencer position

    if ( _loopeable )
        _rangePointer = _bufferRangeStart;
    else
        _sampleStart = AudioEngine::bufferPosition;

    setEnabled( true );
}

AudioBuffer* SampleEvent::synthesize( int aBufferLength )
{
    // nowt... no live synthesis as sample contains a finite buffer
    return _buffer;
}

void SampleEvent::setSample( AudioBuffer* sampleBuffer )
{
    // make sure we lock read/write operations as setting a large sample
    // while the engine is running is a tad dangerous ;)
    bool wasLocked = _locked;
    _locked        = true;

    int sampleLength = sampleBuffer->bufferSize;

    // delete previous contents
    if ( _sampleLength != sampleLength || _buffer == 0 )
        destroyBuffer();

    // is this events buffer destroyable ? then clone
    // the input buffer, if not, merely point to it to
    // minimize memory consumption when re-using existing samples

    if ( _destroyableBuffer )
        _buffer = sampleBuffer->clone();
    else
        _buffer = sampleBuffer;

    _buffer->loopeable = _loopeable;
    _sampleLength      = sampleLength;

    _sampleEnd         = _sampleStart + _sampleLength;
    _bufferRangeStart  = _sampleStart;
    _bufferRangeEnd    = _sampleEnd;
    _bufferRangeLength = _sampleLength;
    _rangePointer      = _bufferRangeStart;

    _updateAfterUnlock = false; // unnecessary

    if ( !wasLocked )
        _locked = false;
}

void SampleEvent::addToSequencer()
{
    if ( !_addedToSequencer )
    {
        (( SampledInstrument* ) _instrument )->audioEvents->push_back( this );
    }
    _addedToSequencer = true;
}

void SampleEvent::removeFromSequencer()
{
    if ( _addedToSequencer )
    {
        SampledInstrument* sampler = (( SampledInstrument* ) _instrument );

        for ( int i; i < sampler->audioEvents->size(); i++ )
        {
            if ( sampler->audioEvents->at( i ) == this )
            {
                sampler->audioEvents->erase( sampler->audioEvents->begin() + i );
                break;
            }
        }
    }
    _addedToSequencer = false;
}

void SampleEvent::swapInstrument( BaseInstrument* aInstrument )
{
    // swap instrument if new one is different to existing reference
    if ( _instrument != aInstrument )
    {
        bool readdToSequencer = _addedToSequencer;
        removeFromSequencer();

        _instrument = aInstrument;

        // if event was added to the Sequencer, re-add it so
        // it can now playback over the new instrument

        if ( readdToSequencer )
            addToSequencer();
    }
}

void SampleEvent::mixBuffer( AudioBuffer* outputBuffer, int bufferPos, int minBufferPosition, int maxBufferPosition,
                             bool loopStarted, int loopOffset, bool useChannelRange )
{
    // if we have a range length that is unequal to the total sample duration, read from the range
    // otherwise invoke the base mixBuffer method

    if ( _loopeable || _bufferRangeLength != _sampleLength )
        getBufferForRange( outputBuffer, bufferPos );
    else
        BaseAudioEvent::mixBuffer( outputBuffer, bufferPos, minBufferPosition, maxBufferPosition,
                                   loopStarted, loopOffset, useChannelRange );
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
        if ( readPos >= _sampleStart && readPos <= _sampleEnd )
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
                targetBuffer[ i ]        += srcBuffer[ _rangePointer ];
            }

            if ( ++_rangePointer > _bufferRangeEnd )
                _rangePointer = _bufferRangeStart;

            gotBuffer = true;
        }

        // if this is a loopeable sample (thus using internal read pointer)
        // set the read pointer to the sample start so it keeps playing indefinetely

        if ( ++readPos > _sampleEnd && _loopeable )
            readPos = _sampleStart;
    }

    if ( useInternalPointer )
        _readPointer = readPos;

    return gotBuffer;
}

/* protected methods */

void SampleEvent::init( BaseInstrument* instrument )
{
    _deleteMe          = false;
    _buffer            = 0;
    _sampleStart       = 0;
    _sampleEnd         = 0;
    _sampleLength      = 0;
    _bufferRangeLength = 0;
    _locked            = false;
    _addedToSequencer  = false;
    _readPointer       = 0;
    _rangePointer      = 0;
    _loopeable         = false;
    _destroyableBuffer = false; // is referenced via SampleManager !
    _instrument        = instrument;
}
