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
#include "global.h"
#include "sequencer.h"
#include "utils.h"

/* constructor / destructor */

SampleEvent::SampleEvent()
{
    position = 0;

    init();
}

SampleEvent::SampleEvent( int aPosition )
{
    position = aPosition;

    init();
}

SampleEvent::~SampleEvent()
{
    removeFromSequencer();
}

/* public methods */

int SampleEvent::getSampleLength()
{
    return _sampleLength;
}

int SampleEvent::getSampleStart()
{
    return _sampleStart;
}

int SampleEvent::getSampleEnd()
{
    return _sampleEnd;
}

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

    if ( getLoopeable() )
        _rangePointer = _bufferRangeStart;
    else
        _sampleStart = bufferPosition;

    setEnabled( true );
}

bool SampleEvent::getLoopeable()
{
    return _loopeable;
}

void SampleEvent::setLoopeable( bool value )
{
    _loopeable = value;

    if ( _buffer != 0 )
        _buffer->loopeable = _loopeable;
}

bool SampleEvent::deletable()
{
    return _deleteMe;
}

void SampleEvent::setDeletable( bool value )
{
    _deleteMe = value;
}

AudioBuffer* SampleEvent::getBuffer()
{
    return _buffer;
}

AudioBuffer* SampleEvent::synthesize( int aBufferLength )
{
    // nowt... no live synthesis as sample contains a finite buffer
    return _buffer;
}

bool SampleEvent::isCached()
{
    return _cachingCompleted;
}

void SampleEvent::setAutoCache( bool aValue )
{
    // nowt... sample is always a finite buffer
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

    // copy buffer contents
    _buffer = sampleBuffer->clone();

    _buffer->loopeable = _loopeable;
    _sampleLength      = sampleLength;
    _cachingCompleted  = true;

    _sampleStart       = position * bytes_per_tick;
    _sampleEnd         = _sampleStart + _sampleLength;
    _bufferRangeStart  = _sampleStart;
    _bufferRangeEnd    = _sampleEnd;
    _bufferRangeLength = _sampleLength;
    _rangePointer      = _bufferRangeStart;

    _updateAfterUnlock = false; // unnecessary

    if ( !wasLocked )
        _locked = false;
}

void SampleEvent::cache()
{
    // nowt... nothing to cache
}

void SampleEvent::addToSequencer( int samplerNum )
{
    _samplerNum = samplerNum;

    if ( !_addedToSequencer )
    {
        sequencer::samplers.at( _samplerNum )->audioEvents->push_back( this );
    }
    _addedToSequencer = true;
}

void SampleEvent::removeFromSequencer()
{
    if ( _addedToSequencer )
    {
        SampledInstrument* sampler = sequencer::samplers.at( _samplerNum );

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

bool SampleEvent::getBufferForRange( AudioBuffer* buffer, int readPos )
{
    int bufferSize          = buffer->bufferSize;
    int amountOfChannels    = buffer->amountOfChannels;
    bool gotBuffer          = false;
    bool monoCopy           = _buffer->amountOfChannels < amountOfChannels;

    bool useInternalPointer = _loopeable;

    if ( useInternalPointer )
        readPos = _readPointer;

    float* srcBuffer;

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

                float* targetBuffer = buffer->getBufferForChannel( c );
                targetBuffer[ i ]   += srcBuffer[ _rangePointer ];
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

void SampleEvent::init()
{
    _deleteMe          = false;
    _cancel            = false;
    _buffer            = 0;
    _sampleLength      = 0;
    _bufferRangeLength = 0;
    _locked            = false;
    _addedToSequencer  = false;
    _readPointer       = 0;
    _rangePointer      = 0;
    _loopeable         = false;
}
