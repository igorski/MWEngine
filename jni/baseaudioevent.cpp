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
#include "global.h"
#include "utils.h"

// constructor / destructor

BaseAudioEvent::BaseAudioEvent()
{
    _buffer            = 0;
    _enabled           = true;
    _destroyableBuffer = true;
    _locked            = false;
}

BaseAudioEvent::~BaseAudioEvent()
{
    // DebugTool::log( "BaseAudioEvent::destructor" );
    destroy();
}

/* public methods */

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

bool BaseAudioEvent::deletable()
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

void BaseAudioEvent::mixBuffer( AudioBuffer* outputBuffer, int bufferPos, int minBufferPosition, int maxBufferPosition,
                                bool loopStarted, int loopOffset, bool useChannelRange )
{
    // read from the pre-cached buffer for sequenced notes

    int startOffset  = getSampleStart();
    int endOffset    = getSampleEnd();
    int sampleLength = getSampleLength();
    int bufferSize   = outputBuffer->bufferSize;

    for ( int i = 0; i < bufferSize; ++i )
    {
        int readPointer = i + bufferPos;

        // over the max position ? read from the start ( sequence has started loop )
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

            for ( int c = 0, ca = _buffer->amountOfChannels; c < ca; ++c )
            {
                SAMPLE_TYPE* srcBuffer = _buffer->getBufferForChannel( c );
                SAMPLE_TYPE* tgtBuffer = outputBuffer->getBufferForChannel( c );

                tgtBuffer[ i ] += srcBuffer[ readPointer ];
            }
        }
        else
        {
            if ( loopStarted )
            {
                if ( i >= loopOffset )
                {
                    readPointer = minBufferPosition + ( i - loopOffset );

                    if ( readPointer >= startOffset && readPointer <= endOffset )
                    {
                        readPointer -= startOffset;

                        for ( int c = 0, ca = _buffer->amountOfChannels; c < ca; ++c )
                        {
                            SAMPLE_TYPE* srcBuffer = _buffer->getBufferForChannel( c );
                            SAMPLE_TYPE* tgtBuffer = outputBuffer->getBufferForChannel( c );

                            tgtBuffer[ i ] += srcBuffer[ readPointer ];
                        }
                    }
                }
            }
        }
    }
}

AudioBuffer* BaseAudioEvent::getBuffer()
{
    return _buffer;
}

AudioBuffer* BaseAudioEvent::synthesize( int aBufferLength )
{
    // override in subclass as this requires memory cleanup (and is basically silence ;-) ... ) !
    return new AudioBuffer( AudioEngineProps::OUTPUT_CHANNELS, aBufferLength );
}

/* protected methods */

void BaseAudioEvent::destroy()
{
    destroyBuffer();
}

void BaseAudioEvent::destroyBuffer()
{
    if ( _destroyableBuffer && _buffer != 0 )
    {
        delete _buffer;
        _buffer = 0;
    }
}
