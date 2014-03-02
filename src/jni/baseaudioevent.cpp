#include "baseaudioevent.h"
#include "utils.h"

// constructor / destructor

BaseAudioEvent::BaseAudioEvent()
{
    _buffer = 0;
    _locked = false;
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

int BaseAudioEvent::getSampleStart()
{
    return _sampleStart;
}

int BaseAudioEvent::getSampleEnd()
{
    return _sampleEnd;
}

bool BaseAudioEvent::deletable()
{
    return _deleteMe;
}

void BaseAudioEvent::setDeletable( bool value )
{
    _deleteMe = value;
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

float* BaseAudioEvent::getBuffer()
{
    return _buffer;
}

float* BaseAudioEvent::synthesize( int aBufferLength )
{
    // override in subclass as this requires memory cleanup (and is basically silence ;-) ... ) !
    return BufferUtil::generateSilentBuffer( aBufferLength );
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
