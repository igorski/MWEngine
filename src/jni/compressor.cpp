#include "compressor.h"
#include "global.h"
#include "utils.h"
#include <algorithm>
#include <cstdlib>
#include <math.h>

/**
 * @param aThreshold  {float} threshold in dB before the compression kicks in, the range is
 *                             -THRESHOLD_MAX_NEGATIVE_VALUE - THRESHOLD_MAX_POSITIVE_VALUE
 * @param aAttack     {float} attack time, in milliseconds, 0.1 - 100 ms
 * @param aRelease    {float} release time, in milliseconds, 200 - 2000 ms
 * @param aRatio      {float} ratio > 0 ratio (compression: < 1 ; expansion: > 1)
 */
Compressor::Compressor( float aThreshold, float aAttack, float aRelease, float aRatio )
{
    _sc = new chunkware_simple::SimpleComp();

    setSampleRate( audio_engine::SAMPLE_RATE );
    setThreshold ( aThreshold );
    setAttack    ( aAttack );
    setRelease   ( aRelease );
    setRatio     ( aRatio );

    _sc->initRuntime(); // TODO : is this right ?
}

Compressor::~Compressor()
{
    delete _sc;
}

/* public methods */

void Compressor::process( float* sampleBuffer, int sampleLength )
{
    for ( int i = 0; i < sampleLength; i++ )
        _sc->process( sampleBuffer[ i ], sampleBuffer[ i ]);    // TODO : assumes stereo
}

float Compressor::getAttack()
{
    return _sc->getAttack();
}

void Compressor::setAttack( float value )
{
    _sc->setAttack( value );
}

float Compressor::getRelease()
{
    return _sc->getRelease();
}

void Compressor::setRelease( float value )
{
    _sc->setRelease( value );
}

float Compressor::getRatio()
{
    return _sc->getRatio();
}

void Compressor::setRatio( float value )
{
    _sc->setRatio( value );
}

float Compressor::getThreshold()
{
    return _sc->getThresh();
}

void Compressor::setThreshold( float value )
{
    _sc->setThresh( value );
}

void Compressor::setSampleRate( int aSampleRate )
{
    _sc->setSampleRate(( float ) aSampleRate );
}
