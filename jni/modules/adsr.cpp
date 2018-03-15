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
    setEnvelopesInternal( getAttackTime(), getDecayTime(), getSustainLevel(), getReleaseTime() );
}

ADSR::ADSR( float attackTime, float decayTime, float sustainLevel, float releaseTime )
{
    construct();
    setEnvelopesInternal( attackTime, decayTime, sustainLevel, releaseTime );
}

ADSR::~ADSR()
{

}

/* public methods */

float ADSR::getAttackTime()
{
    return _attackTime;
}

void ADSR::setAttackTime( float aValue )
{
    setEnvelopesInternal( aValue, getDecayTime(), getSustainLevel(), getReleaseTime() );
}

float ADSR::getDecayTime()
{
    return _decayTime;
}

void ADSR::setDecayTime( float aValue )
{
    setEnvelopesInternal( getAttackTime(), aValue, getSustainLevel(), getReleaseTime() );
}

float ADSR::getSustainLevel()
{
    return _sustainLevel;
}

void ADSR::setSustainLevel( float aValue )
{
    setEnvelopesInternal( getAttackTime(), getDecayTime(), aValue, getReleaseTime() );
}

float ADSR::getReleaseTime()
{
    return _releaseTime;
}

void ADSR::setReleaseTime( float aValue )
{
    setEnvelopesInternal( getAttackTime(), getDecayTime(), getSustainLevel(), aValue );
}

int ADSR::getReleaseStartOffset()
{
    return _releaseStart;
}

int ADSR::getReleaseDuration()
{
    return _releaseDuration;
}

ADSR* ADSR::clone()
{
    ADSR* out = new ADSR();

    out->_bufferLength = _bufferLength;
    out->setEnvelopesInternal( getAttackTime(), getDecayTime(), getSustainLevel(), getReleaseTime() );

    return out;
}

void ADSR::cloneEnvelopes( ADSR* source )
{
    setEnvelopesInternal(
        source->getAttackTime(), source->getDecayTime(),
        source->getSustainLevel(), source->getReleaseTime()
    );
}

void ADSR::apply( AudioBuffer* inputBuffer, BaseSynthEvent* synthEvent, int writeOffset )
{
    SAMPLE_TYPE lastEnvelope = synthEvent->cachedProps.envelope;
    int eventDuration        = synthEvent->getEventLength();

    // the events lifetime is actually extended by the release phase of this ADSR envelope
    int eventDurationWithRelease = eventDuration + _releaseDuration;

    // nothing to do
    if ( writeOffset > eventDurationWithRelease && lastEnvelope == MAX_PHASE )
        return;

    // cache envelopes for given event duration
    if ( eventDuration != _bufferLength ) {
        _bufferLength = eventDuration;
        invalidateEnvelopes();
    }

    int bufferSize     = inputBuffer->bufferSize;
    int writeEndOffset = writeOffset + bufferSize; // for the current cycle
    float sustainLevel = _sustainLevel;
    
    bool applyAttack  = _attackDuration  > 0 && writeOffset < _decayStart;
    bool applyDecay   = _decayDuration   > 0 && writeEndOffset >= _decayStart   && writeOffset < _sustainStart;
    bool applySustain = _sustainDuration > 0 && writeEndOffset >= _sustainStart && writeOffset < _releaseStart;
    bool applyRelease = synthEvent->isSequenced && _releaseDuration > 0 &&
                        writeEndOffset >= _releaseStart && writeOffset < eventDurationWithRelease;

    // early release can be forced if event has received noteOff (e.g. key up)
     // this is necessary because resulting event duration might be shorter than
     // the attack or decay phases

    if ( synthEvent->released ) {
        applyAttack  =
        applyDecay   =
        applySustain = false;
        applyRelease = true;

        writeOffset  = synthEvent->cachedProps.envelopeOffset;
        sustainLevel = synthEvent->cachedProps.releaseLevel;
    }

    // no envelope update operations ? mix in at last envelope amplitude and return
    // (this could for instance be the sustain phase)
    if ( !applyAttack  &&
         !applyDecay   &&
         !applyRelease )
    {
        if ( lastEnvelope < MAX_PHASE )
            inputBuffer->adjustBufferVolumes( lastEnvelope );

        return;
    }

    int readOffset;
    for ( int cn = 0, ca = inputBuffer->amountOfChannels; cn < ca; ++cn )
    {
        SAMPLE_TYPE* targetBuffer = inputBuffer->getBufferForChannel( cn );
        readOffset = writeOffset;

        for ( int i = 0; i < bufferSize; ++i, ++readOffset )
        {
            // attack envelope
            if ( applyAttack && readOffset < _attackDuration )
                lastEnvelope = ( SAMPLE_TYPE ) readOffset * _attackIncrement;

            // decay envelope
            else if ( applyDecay && readOffset >= _decayStart && readOffset <= _sustainStart )
                lastEnvelope = MAX_PHASE - ( SAMPLE_TYPE ) ( readOffset - _decayStart ) * _decayDecrement;

            // sustain envelope (keeps at last envelope value which is the last decay phase value)

            else if ( applySustain && readOffset >= _sustainStart && readOffset <= _releaseStart )
                lastEnvelope = _sustainLevel;

            // release envelope

            else if ( applyRelease && readOffset >= _releaseStart )
                lastEnvelope = std::max(
                    sustainLevel - ( SAMPLE_TYPE ) ( readOffset - _releaseStart ) * _releaseDecrement,
                    ( SAMPLE_TYPE ) 0.0
                );

            // apply the calculated amplitude envelope onto the sample

            targetBuffer[ i ] *= lastEnvelope;
        }
    }

    // store the current envelope into the events cached properties
    synthEvent->cachedProps.envelope = lastEnvelope;

    // when rendering the released envelope we must cache
    // the offset within the release phase so event
    // can calculate when it actually stops playing
    if ( synthEvent->released ) {
        synthEvent->cachedProps.envelopeOffset = readOffset;
    }
}

void ADSR::setDurations( int attackDuration, int decayDuration, int releaseDuration, int bufferLength )
{
    _attackDuration  = attackDuration;
    _decayDuration   = decayDuration;
    _sustainDuration = std::max( 0, bufferLength - ( attackDuration + decayDuration ));
    _releaseDuration = releaseDuration;

    // update start offsets for DSR stages
    _decayStart     = _attackDuration;
    _sustainStart   = _decayStart + _decayDuration;
    _releaseStart   = bufferLength;

    // update increments for the envelope stages
    _attackIncrement  = MAX_PHASE     / ( SAMPLE_TYPE ) std::max( 1, _attackDuration );
    _decayDecrement   = ( MAX_PHASE - _sustainLevel ) / _decayDuration;                  // move to sustain phase amplitude
    _releaseDecrement = _sustainLevel / ( SAMPLE_TYPE ) std::max( 1, _releaseDuration ); // release from sustain phase amp

    _bufferLength = bufferLength;
}

/* protected methods */

void ADSR::setEnvelopesInternal( float attackTime, float decayTime, float sustainLevel, float releaseTime )
{
    // 1. ATTACK
    // note that the minimum allowed value is DEFAULT_FADE_DURATION to prevent popping during sound start
    int DEFAULT_FADE_DURATION = 8;

    _attackTime = attackTime;
    int attackDuration = std::max(
        DEFAULT_FADE_DURATION,
        BufferUtility::millisecondsToBuffer( _attackTime * 1000, AudioEngineProps::SAMPLE_RATE )
    );

    // 2. DECAY takes its start offset relative from the attack...

    _decayTime = decayTime;
    int decayDuration  = BufferUtility::millisecondsToBuffer( _decayTime * 1000, AudioEngineProps::SAMPLE_RATE );

    // 3. SUSTAIN level should be between 0 - 1

    _sustainLevel = std::max(( SAMPLE_TYPE ) 0, std::min(( SAMPLE_TYPE ) sustainLevel, MAX_PHASE ));

    // 4. RELEASE

    _releaseTime = releaseTime;
    int releaseDuration  = BufferUtility::millisecondsToBuffer( _releaseTime * 1000, AudioEngineProps::SAMPLE_RATE );

    // commit changes

    setDurations( attackDuration, decayDuration, releaseDuration, _bufferLength );
}

void ADSR::invalidateEnvelopes()
{
    setEnvelopesInternal( getAttackTime(), getDecayTime(), getSustainLevel(), getReleaseTime() );
}

void ADSR::construct()
{
    _bufferLength    = 0;
    _attackTime      = 0;
    _decayTime       = 0;
    _sustainLevel    = ( float ) MAX_PHASE;
    _releaseTime     = 0;
    _decayStart      = 0;
    _sustainStart    = 0;
    _releaseStart    = 0;
    _attackDuration  = 0;
    _decayDuration   = 0;
    _releaseDuration = 0;
}
