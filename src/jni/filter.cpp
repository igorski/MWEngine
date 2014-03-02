#include "filter.h"
#include "global.h"
#include <math.h>

/**
 * @param aCutoffFrequency {float} desired cutoff frequency in Hz
 * @param aResonance {float} resonance
 * @param aMinFreq {float} minimum cutoff frequency in Hz, required for LFO automation
 * @param aMaxFreq {float} maximum cutoff frequency in Hz, required for LFO automation
 * @param aLfoRate {float} LFO speed in Hz, 0 for OFF
 */
Filter::Filter( float aCutoffFrequency, float aResonance, float aMinFreq, float aMaxFreq, float aLfoRate )
{
    fs            = audio_engine::SAMPLE_RATE;
    _tempCutoff   = 0;    // used for reading when automating via LFO

    _resonance = aResonance;
    setCutoff ( aCutoffFrequency );
    setLFORate( aLfoRate );
    _lfo       = 0;
    _hasLFO    = false;

    minFreq    = aMinFreq;
    maxFreq    = aMaxFreq;
    lfoRange   = ( maxFreq * .5 ) - minFreq;

    in1 = in2 = out1 = out2 = 0;

    calculateParameters();
}

Filter::~Filter()
{
    //delete _lfo; // nope... belongs to routeable oscillator in the instrument
}

/* public methods */

void Filter::process( float* sampleBuffer, int bufferLength )
{
    for ( int i = 0; i < bufferLength; ++i )
    {
        float input = sampleBuffer[ i ];
        output = a1 * input + a2 * in1 + a3 * in2 - b1 * out1 - b2 * out2;

        in2  = in1;
        in1  = input;
        out2 = out1;
        out1 = output;

        // oscillator attached to Filter ? travel the cutoff values
        // between the minimum and half way the maximum frequencies, as
        // defined by lfoRange in the class constructor

        if ( _hasLFO )
        {
            _tempCutoff   = _cutoff + ( lfoRange * _lfo->peek() );

            if ( _tempCutoff > maxFreq )
                _tempCutoff = maxFreq;

            else if ( _tempCutoff < minFreq )
                _tempCutoff = minFreq;

            calculateParameters();
        }
        // commit the effect
        sampleBuffer[ i ] = output;
    }
}

void Filter::setCutoff( float frequency )
{
    _cutoff = frequency;

    if ( _cutoff >= fs * .5 )
        _cutoff = fs * .5 - 1;

    if ( _cutoff < minFreq )
        _cutoff = minFreq;

    _tempCutoff = _cutoff;

    calculateParameters();
}

float Filter::getCutoff()
{
    return _cutoff;
}

void Filter::setResonance( float resonance )
{
    _resonance = resonance;
    calculateParameters();
}

float Filter::getResonance()
{
    return _resonance;
}

float Filter::getLFO()
{
    if ( _lfo != 0 )
        return _lfo->getRate();

    return 0;
}

void Filter::setLFO( LFO *lfo )
{
    _lfo      = lfo;

    // no LFO ? make sure the filter returns to its default parameters
    if ( lfo == 0 )
    {
        _hasLFO     = false;
        _tempCutoff = _cutoff;
        calculateParameters();
    }
    else {
        _hasLFO = true;
    }
}

// LFO might be a fixed pointer, but we
// can turn it on/off without deleting pointer for Object pooling
bool Filter::hasLFO()
{
    return _hasLFO;
}

void Filter::hasLFO( bool value )
{
    _hasLFO = value;

    // no LFO ? make sure the filter returns to its default parameters
    if ( !_hasLFO )
    {
        _tempCutoff = _cutoff;
        calculateParameters();
    }
}

void Filter::setLFORate( float rate )
{
    if ( rate == 0 )
    {
        _hasLFO = false;
        _tempCutoff = _cutoff;
        return;
    }
    else {
        _hasLFO = true;
    }
    _lfo->setRate( rate );
}

/* private methods */

void Filter::calculateParameters()
{
    c  = 1 / tan(( atan( 1 ) * 4 ) * _tempCutoff / fs );
    a1 = 1.0 / ( 1.0 + _resonance * c + c * c);
    a2 = 2 * a1;
    a3 = a1;
    b1 = 2.0 * ( 1.0 - c * c ) * a1;
    b2 = ( 1.0 - _resonance * c + c * c) * a1;
}
