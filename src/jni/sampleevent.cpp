#include "sampleevent.h"
#include "global.h"
#include "utils.h"

/* constructor / destructor */

SampleEvent::SampleEvent()
{
    position = 0;
    _locked  = false;

    init();
}

SampleEvent::SampleEvent( int aPosition )
{
    position = aPosition;
}

SampleEvent::~SampleEvent()
{

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

bool SampleEvent::deletable()
{
    return _deleteMe;
}

void SampleEvent::setDeletable( bool value )
{
    _deleteMe = value;
}

float* SampleEvent::getBuffer()
{
    return _buffer;
}

float* SampleEvent::synthesize( int aBufferLength )
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

void SampleEvent::setSample( float* sampleBuffer, int sampleLength )
{
    // make sure we lock read/write operations as setting a large sample
    // while the engine is running is a tad dangerous ;)
    bool wasLocked = _locked;
    _locked        = true;

    if ( _sampleLength != sampleLength || _buffer == 0 )
    {
        destroyBuffer(); // delete previous contents
        _buffer = BufferUtil::generateSilentBuffer( sampleLength );
    }
    // copy buffer contents
    int i = 0;
    for ( i; i < sampleLength; i++ )
        _buffer[ i ] = sampleBuffer[ i ];

    _sampleLength     = sampleLength;
    _cachingCompleted = true;

    _sampleStart  = position * bytes_per_tick;
    _sampleEnd    = _sampleStart + _sampleLength;

    _updateAfterUnlock = false; // unnecessary

    if ( !wasLocked )
        _locked = false;
}

void SampleEvent::cache( bool doCallback )
{
    // nowt... nothing to cache
}

/* protected methods */

void SampleEvent::init()
{
    _deleteMe         = false;
    _cancel           = false;
    _buffer           = 0;
    _sampleLength     = 0;
}
