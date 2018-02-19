/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2018 Igor Zinken - http://www.igorski.nl
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
#include <modules/adsr.h>
#include <utilities/bufferutility.h>
#include <algorithm>

/* constructors / destructor */

ADSR::ADSR()
{
    construct();
    setEnvelopesInternal( 0, 0, 0, 0 );
}

ADSR::ADSR( float attack, float decay, float sustain, float release )
{
    construct();
    setEnvelopesInternal( attack, decay, sustain, release );
}

ADSR::~ADSR()
{

}

/* public methods */

float ADSR::getAttack()
{
    return _attack;
}

void ADSR::setAttack( float aValue )
{
    setEnvelopesInternal( aValue, getDecay(), getSustain(), getRelease() );
}

float ADSR::getDecay()
{
    return _decay;
}

void ADSR::setDecay( float aValue )
{
    setEnvelopesInternal( getAttack(), aValue, getSustain(), getRelease() );
}

float ADSR::getSustain()
{
    return _sustain;
}

void ADSR::setSustain( float aValue )
{
    setEnvelopesInternal( getAttack(), getDecay(), aValue, getRelease() );
}

float ADSR::getRelease()
{
    return _release;
}

void ADSR::setRelease( float aValue )
{
    setEnvelopesInternal( getAttack(), getDecay(), getSustain(), aValue );
}

SAMPLE_TYPE ADSR::getLastEnvelope()
{
    return _lastEnvelope;
}

void ADSR::setLastEnvelope( SAMPLE_TYPE aEnvelope )
{
    _lastEnvelope = aEnvelope;
}

SAMPLE_TYPE ADSR::apply( AudioBuffer* inputBuffer )
{
    return apply( inputBuffer, inputBuffer->bufferSize, 0 );
}

SAMPLE_TYPE ADSR::apply( AudioBuffer* inputBuffer, int eventDuration, int eventOffset )
{
    // nothing to do
    if ( eventOffset > eventDuration && _lastEnvelope == MAX_PHASE )
        return _lastEnvelope;

    // cache envelopes for given event duration
    if ( eventDuration !== _bufferLength ) {
        invalidateEnvelopes();
        _bufferLength = eventDuration;
    }

    int bufferSize     = inputBuffer->bufferSize;
    int writeEndOffset = eventOffset + bufferSize; // for the current cycle
    
    bool applyAttack  = _attack > 0  && eventOffset < _decayStart;
    bool applyDecay   = _decay  > 0  && writeEndOffset >= _decayStart   && eventOffset < _sustainStart;
    bool applySustain = _sustain > 0 && writeEndOffset >= _sustainStart && eventOffset < _releaseStart;
    bool applyRelease = _release > 0 && writeEndOffset >= _releaseStart && eventOffset < _bufferLength;

    // no envelope update operations ? mix in at last envelope amplitude and return
    if ( !applyAttack  &&
         !applyDecay   &&
         !applySustain &&
         !applyRelease )
    {
        if ( _lastEnvelope < MAX_PHASE )
            inputBuffer->adjustBufferVolumes( _lastEnvelope );

        return _lastEnvelope;
    }

    for ( int cn = 0, ca = inputBuffer->amountOfChannels; cn < ca; ++cn )
    {
        SAMPLE_TYPE* targetBuffer = inputBuffer->getBufferForChannel( cn );
        int readOffset = eventOffset;

        for ( int i = 0; i < bufferSize; ++i, ++readOffset )
        {
            // attack envelope
            if ( applyAttack && readOffset < _attackDuration )
                _lastEnvelope = ( SAMPLE_TYPE ) readOffset * _attackIncrement;

            // decay envelope
            else if ( applyDecay && readOffset >= _decayStart && readOffset < decayEndOffset )
                _lastEnvelope = MAX_PHASE - ( SAMPLE_TYPE ) ( readOffset - _decayStart ) * _decayIncrement;

            // sustain envelope (takes volume from last amplitude envelope and thus requires no action)
            else if ( applySustain && readOffset >= _sustainStart && readOffset < releaseStart )
                _lastEnvelope = HALF_PHASE;

            else if ( applyRelease && readOffset >= _releaseStart )
                _lastEnvelope = HALF_PHASE - ( SAMPLE_TYPE ) ( readOffset - _releaseStart ) * _releaseIncrement;

            // apply the calculated amplitude envelope onto the sample

            targetBuffer[ i ] *= std::max( 0.0, _lastEnvelope );
        }
    }
    return _lastEnvelope;
}

ADSR* ADSR::clone()
{
    ADSR* out = new ADSR();

    out->_bufferLength = _bufferLength;
    out->setEnvelopesInternal( getAttack(), getDecay(), getSustain(), getRelease() );

    return out;
}

void ADSR::cloneEnvelopes( ADSR* source )
{
    setEnvelopesInternal( source->getAttack(), source->getDecay(), source->getSustain(), source->getRelease() );

    _lastEnvelope = MAX_PHASE;
}

/* protected methods */

void ADSR::setEnvelopesInternal( float attack, float decay, float sustain, float release )
{
    // 1. ATTACK
    // note that the minimum allowed value is DEFAULT_FADE_DURATION to prevent popping during sound start

    _attack         = attack;
    _attackDuration = std::max(
        DEFAULT_FADE_DURATION,
        BufferUtility::millisecondsToBuffer( _attack * 1000, AudioEngineProps::SAMPLE_RATE
    );
    _attackIncrement = MAX_PHASE / ( SAMPLE_TYPE ) _attackDuration;

    // 2. DECAY takes its start offset relative from the attack...

    _decay          = decay;
    _decayDuration  = BufferUtility::millisecondsToBuffer( _decay * 1000, AudioEngineProps::SAMPLE_RATE );
    _decayStart     = _attackDuration;
    _decayIncrement = HALF_PHASE / ( SAMPLE_TYPE ) _decayDuration; // move to 50% of amplitude for sustain phase

    // 3. SUSTAIN takes its start offset relative from the decay...

    _sustain         = sustain;
    _sustainStart    = _decayStart + _decayDuration;
    _sustainDuration = BufferUtility::millisecondsToBuffer( _sustain * 1000, AudioEngineProps::SAMPLE_RATE );

    // 4. RELEASE
    _release          = release;
    _releaseStart     = ( _release > 0 ) ? _bufferLength - _sustainDuration : _bufferLength;
    _releaseDuration  = BufferUtility::millisecondsToBuffer( _release * 1000, AudioEngineProps::SAMPLE_RATE );
    _releaseIncrement = HALF_PHASE / ( SAMPLE_TYPE ) _releaseDuration; // move from 50% of amplitude of sustain phose
}

void ADSR::invalidateEnvelopes()
{
    // resetting the attack will recursively update the remaining envelopes internally
    setAttack( getAttack() );

    _lastEnvelope = MAX_PHASE;
}

void ADSR::construct()
{
    _bufferLength    = 0;
    _decayStart      = 0;
    _sustainStart    = 0;
    _releaseStart    = 0;
    _attackDuration  = 0;
    _decayDuration   = 0;
    _sustainDuration = 0;
    _releaseDuration = 0;
    _lastEnvelope    = MAX_PHASE;
}
