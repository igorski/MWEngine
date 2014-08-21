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
    _buffer  = 0;
    _enabled = true;
    _locked  = false;
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
    if ( _buffer != 0 )
    {
        delete _buffer;
        _buffer = 0;
    }
}
