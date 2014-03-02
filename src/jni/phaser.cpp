#include "phaser.h"
#include "global.h"
#include <math.h>

/* constructor / destructor */

/**
 * @param aRate     {float} desired LFO rate in Hz
 * @param aFeedback {float} 0 - 1
 * @param aDepth    {float} 0 - 1
 * @param aMinFreq  {float} minimum frequency value in Hz allowed for the filters range
 * @param aMaxFreq  {float} maxiumum frequency value in Hz allowed for the filters range
 */
Phaser::Phaser( float aRate, float aFeedback, float aDepth, float aMinFreq, float aMaxFreq )
{
    _lfoPhase = 0.0;
    _zm1      = 0.0;

    setRange( aMinFreq, aMaxFreq );
    setRate( aRate );

    _fb       = aFeedback;
    _depth    = aDepth;

    int stages = 6; // six-stage phaser

    for ( int i = 0; i < stages; ++i )
        _alps[ i ] = new AllPassDelay();
}

Phaser::~Phaser()
{
    delete[] _alps;
}

/* public methods */

/**
 * set the filter range
 * @param aMin {float} lowest allowed value in Hz
 * @param aMax {float} highest allowed value in Hz
 */
void Phaser::setRange( float aMin, float aMax )
{
    _dmin = aMin / ( audio_engine::SAMPLE_RATE / 2.0 );
    _dmax = aMax / ( audio_engine::SAMPLE_RATE / 2.0 );
}

float Phaser::getRate()
{
    return _rate;
}

/**
 * @param aRate {float} in Hz
 */
void Phaser::setRate( float aRate )
{
    _rate   = aRate;
    _lfoInc = 2.0 * 3.14159f * ( _rate / audio_engine::SAMPLE_RATE );
}

float Phaser::getFeedback()
{
    return _fb;
}

void Phaser::setFeedback( float fb )
{
    _fb = fb;
}

float Phaser::getDepth()
{
    return _depth;
}

void Phaser::setDepth( float depth )
{
    _depth = depth;
}

void Phaser::process( float* sampleBuffer, int bufferLength )
{
    float maxPhase = 3.14159f * 2.f;
    int i = 0;
    int j;

    for ( i; i < bufferLength; ++i )
    {
        // calculate and update phaser sweep LFO...
        float d   = _dmin + ( _dmax - _dmin ) * (( sin( _lfoPhase ) + 1.0 ) / 2.0 );
        _lfoPhase += _lfoInc;

        if ( _lfoPhase >= maxPhase )
            _lfoPhase -= maxPhase;

        // update filter coeffs
        for ( j = 0; j < 6; ++j )
            _alps[ j ]->delay( d );

        // calculate output
        float y = _alps[ 0 ]->update(
                   _alps[ 1 ]->update(
                   _alps[ 2 ]->update(
                   _alps[ 3 ]->update(
                   _alps[ 4 ]->update(
                   _alps[ 5 ]->update( sampleBuffer[ i ] + _zm1 * _fb ))))));

        _zm1 = y;

        sampleBuffer[ i ] += ( y * _depth );
    }
}

/* "private" class */

AllPassDelay::AllPassDelay()
{
    _a1  = 0.0;
    _zm1 = 0.0;
}

/* public methods */

// sample delay time
void AllPassDelay::delay( float aDelay )
{
    _a1 = ( 1.0 - aDelay ) / ( 1.0 + aDelay );
}

float AllPassDelay::update( float aSample )
{
    float y = aSample * - _a1 + _zm1;
    _zm1 = y * _a1 + aSample;

    return y;
}
