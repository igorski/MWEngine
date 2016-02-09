/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2016 Igor Zinken - http://www.igorski.nl
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
#include "tremolo.h"
#include "../global.h"
#include <generators/envelopegenerator.h>

/* constructors / destructor */

Tremolo::Tremolo( int aLeftType,  int aLeftAttack,  int aLeftDecay,
                  int aRightType, int aRightAttack, int aRightDecay )
{
    _tables = new std::vector<SAMPLE_TYPE*>( 2 );

    // create stereo tables

    SAMPLE_TYPE* leftTable;
    SAMPLE_TYPE* rightTable;

    if ( aLeftType == LINEAR )
        leftTable = EnvelopeGenerator::generateLinear( ENVELOPE_PRECISION, 0.0, MAX_PHASE );
    else
        leftTable = EnvelopeGenerator::generateExponential( ENVELOPE_PRECISION );

    if ( aRightType == LINEAR )
        rightTable = EnvelopeGenerator::generateLinear( ENVELOPE_PRECISION, 0.0, MAX_PHASE );
    else
        rightTable = EnvelopeGenerator::generateExponential( ENVELOPE_PRECISION );

    _tables->at( 0 ) = leftTable;
    _tables->at( 1 ) = rightTable;

    _leftType  = aLeftType;
    _rightType = aRightType;

    // set envelopes

    setLeftAttack ( aLeftAttack );
    setLeftDecay  ( aLeftDecay );
    setRightAttack( aRightAttack );
    setRightDecay ( aRightDecay );

    _leftTableIndex  = 0.0;
    _rightTableIndex = 0.0;
    _leftState       = 0;
    _rightState      = 0;
}

Tremolo::~Tremolo()
{
    for ( int i = 0; i < _tables->size(); ++i )
        delete _tables->at( i );

    delete _tables;
}

/* public methods */

int Tremolo::getLeftAttack()
{
    return _leftAttack;
}

void Tremolo::setLeftAttack( int aAttack )
{
    _leftAttack     = aAttack;
    _leftAttackIncr = ( SAMPLE_TYPE ) ENVELOPE_PRECISION;

    if ( aAttack > 0 )
        _leftAttackIncr /= (( SAMPLE_TYPE )( aAttack / 1000.0f ) * AudioEngineProps::SAMPLE_RATE );
}

int Tremolo::getRightAttack()
{
    return _rightAttack;
}

void Tremolo::setRightAttack( int aAttack )
{
    _rightAttack     = aAttack;
    _rightAttackIncr = ( SAMPLE_TYPE ) ENVELOPE_PRECISION;

    if ( aAttack > 0 )
        _rightAttackIncr /= (( SAMPLE_TYPE )( aAttack / 1000.0f ) * AudioEngineProps::SAMPLE_RATE );
}

int Tremolo::getLeftDecay()
{
    return _leftDecay;
}

void Tremolo::setLeftDecay( int aDecay )
{
    _leftDecay     = aDecay;
    _leftDecayIncr = ( SAMPLE_TYPE ) ENVELOPE_PRECISION;

    if ( aDecay > 0 )
        _leftDecayIncr /= (( SAMPLE_TYPE )( aDecay / 1000.0f ) * AudioEngineProps::SAMPLE_RATE );
}

int Tremolo::getRightDecay()
{
    return _rightDecay;
}

void Tremolo::setRightDecay( int aDecay )
{
    _rightDecay     = aDecay;
    _rightDecayIncr = ( SAMPLE_TYPE ) ENVELOPE_PRECISION;

    if ( aDecay > 0 )
        _rightDecayIncr /= (( SAMPLE_TYPE )( aDecay / 1000.0f ) * AudioEngineProps::SAMPLE_RATE );
}

SAMPLE_TYPE* Tremolo::getTableForChannel( int aChannelNum )
{
    return _tables->at( aChannelNum );
}

bool Tremolo::isStereo()
{
    if ( _leftType   != _rightType   ||
         _leftAttack != _rightAttack ||
         _leftDecay  != _rightDecay )
    {
        return true;
    }
}

void Tremolo::process( AudioBuffer* sampleBuffer, bool isMonoSource )
{
    int bufferSize = sampleBuffer->bufferSize;
    bool doStereo  = ( sampleBuffer->amountOfChannels > 1 ) && isStereo();

    SAMPLE_TYPE* envelopeTable;
    SAMPLE_TYPE volume;

    for ( int c = 0, ca = sampleBuffer->amountOfChannels; c < ca; ++c )
    {
        envelopeTable = getTableForChannel( c );
        SAMPLE_TYPE* channelBuffer = sampleBuffer->getBufferForChannel( c );
        bool useLeft = !doStereo || c % 2 == 0;

        for ( int i = 0; i < bufferSize; ++i )
        {
            if ( useLeft )
            {
                if ( _leftState == 0 )
                {
                    if (( _leftTableIndex += _leftAttackIncr ) >= ENVELOPE_PRECISION )
                    {
                        _leftTableIndex = ENVELOPE_PRECISION - 1;
                        _leftState      = 1;
                    }
                }
                else if ( _leftState == 1 )
                {
                    if (( _leftTableIndex -= _leftDecayIncr ) <= 0 )
                    {
                        _leftTableIndex = 0;

                        // optional: hold state
                        // if resulting volume is now smaller than sustain amp > increment _leftState
                        _leftState = 0;
                    }
                }
                volume = envelopeTable[( int ) _leftTableIndex ];
            }
            else {

                // right channel

                if ( _rightState == 0 )
                {
                    if (( _rightTableIndex += _rightAttackIncr ) >= ENVELOPE_PRECISION )
                    {
                        _rightTableIndex = ENVELOPE_PRECISION - 1;
                        _rightState      = 1;
                    }
                }
                else if ( _rightState == 1 )
                {
                    if (( _rightTableIndex -= _rightDecayIncr ) <= 0 )
                    {
                        _rightTableIndex = 0;

                        // optional: hold state
                        // if resulting volume is now smaller than sustain amp > increment _rightState
                        _rightState = 0;
                    }
                }
                volume = envelopeTable[( int ) _rightTableIndex ];
            }
            channelBuffer[ i ] *= volume;
        }

        // save CPU cycles when source and output are mono
        if ( isMonoSource && !doStereo )
        {
            sampleBuffer->applyMonoSource();
            break;
        }
    }
}
