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
#include "filter.h"
#include "global.h"
#include <math.h>

/**
 * @param aCutoffFrequency {float} desired cutoff frequency in Hz
 * @param aResonance {float} resonance
 * @param aMinFreq {float} minimum cutoff frequency in Hz, required for LFO automation
 * @param aMaxFreq {float} maximum cutoff frequency in Hz, required for LFO automation
 * @param aLfoRate {float} LFO speed in Hz, 0 for OFF
 * @param numChannels {int} amount of output channels
 */
Filter::Filter( float aCutoffFrequency, float aResonance, float aMinFreq, float aMaxFreq, float aLfoRate, int numChannels )
{
    fs            = audio_engine::SAMPLE_RATE;
    _tempCutoff   = 0;    // used for reading when automating via LFO

    _resonance = aResonance;
    setLFORate( aLfoRate );
    _lfo       = 0;
    _hasLFO    = false;

    minFreq    = aMinFreq;
    maxFreq    = aMaxFreq;
    lfoRange   = ( maxFreq / 2 ) - minFreq;

    in1  = new float[ numChannels ];
    in2  = new float[ numChannels ];
    out1 = new float[ numChannels ];
    out2 = new float[ numChannels ];

    for ( int i = 0; i < numChannels; ++i )
    {
        in1 [ i ] = 0.0;
        in2 [ i ] = 0.0;
        out1[ i ] = 0.0;
        out2[ i ] = 0.0;
    }
    setCutoff( aCutoffFrequency );
}

Filter::~Filter()
{
    //delete _lfo; // nope... belongs to routeable oscillator in the instrument

    delete[] in1;
    delete[] in2;
    delete[] out1;
    delete[] out2;
}

/* public methods */

void Filter::process( AudioBuffer* sampleBuffer, bool isMonoSource )
{
    int bufferSize       = sampleBuffer->bufferSize;
    int initialLFOoffset = _hasLFO ? _lfo->getReadOffset() : 0;
    float orgCutoff     = _tempCutoff;

    for ( int i = 0, l = sampleBuffer->amountOfChannels; i < l; ++i )
    {
        SAMPLE_TYPE* channelBuffer = sampleBuffer->getBufferForChannel( i );

        // each channel needs the same offset to get the same LFO movement ;)
        if ( _hasLFO && i > 0 )
        {
            _lfo->setReadOffset( initialLFOoffset );
            _tempCutoff = orgCutoff;
            calculateParameters();
        }

        for ( int j = 0; j < bufferSize; ++j )
        {
            float input = channelBuffer[ j ];
            output       = a1 * input + a2 * in1[ i ] + a3 * in2[ i ] - b1 * out1[ i ] - b2 * out2[ i ];

            in2 [ i ] = in1[ i ];
            in1 [ i ] = input;
            out2[ i ] = out1[ i ];
            out1[ i ] = output;

            // oscillator attached to Filter ? travel the cutoff values
            // between the minimum and half way the maximum frequencies, as
            // defined by lfoRange in the class constructor

            if ( _hasLFO )
            {
                _tempCutoff = _cutoff + ( lfoRange * _lfo->peek() );

                if ( _tempCutoff > maxFreq )
                    _tempCutoff = maxFreq;

                else if ( _tempCutoff < minFreq )
                    _tempCutoff = minFreq;

                calculateParameters();
            }
            // commit the effect
            channelBuffer[ j ] = output;
        }

        // save CPU cycles when source is mono
        if ( isMonoSource )
        {
            sampleBuffer->applyMonoSource();
            break;
        }
    }
}

bool Filter::isCacheable()
{
    return !hasLFO();
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

    if ( hasLFO() )
        _lfo->setRate( rate );
}

/* private methods */

void Filter::calculateParameters()
{
    c  = 1 / tan( PI * _tempCutoff / fs );
    a1 = 1.0 / ( 1.0 + _resonance * c + c * c );
    a2 = 2 * a1;
    a3 = a1;
    b1 = 2.0 * ( 1.0 - c * c ) * a1;
    b2 = ( 1.0 - _resonance * c + c * c ) * a1;
}
