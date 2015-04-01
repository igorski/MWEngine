/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Igor Zinken - http://www.igorski.nl
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
#include <utilities/utils.h>

/* constructors / destructor */

ADSR::ADSR()
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

ADSR::~ADSR()
{

}

/* public methods */

void ADSR::setBufferLength( int aBufferLength )
{
    if ( aBufferLength != _bufferLength )
    {
        _bufferLength = aBufferLength;
        invalidateEnvelopes();
    }
}

int ADSR::getBufferLength()
{
    return _bufferLength;
}

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
    return apply( inputBuffer, 0 );
}

SAMPLE_TYPE ADSR::apply( AudioBuffer* inputBuffer, int eventOffset )
{
    // nothing to do
    if ( eventOffset > _bufferLength && _lastEnvelope == MAX_PHASE )
        return _lastEnvelope;

    int bufferSize     = inputBuffer->bufferSize;
    int eventEndOffset = eventOffset + bufferSize;
    
    // check which envelopes have something to do for the given eventOffset
    int decayEndOffset = _decayStart + _decayDuration;

    bool applyAttack  = _attack > 0  && eventOffset < _attackDuration;
    bool applyDecay   = _decay  > 0  && eventOffset < decayEndOffset && eventEndOffset >= _decayStart;
    bool applySustain = _sustain > 0 && eventOffset < ( _sustainStart + _sustainDuration ) && eventEndOffset >= _sustainStart;
    bool applyRelease = _release > 0 && eventOffset < ( _releaseStart + _releaseDuration ) && eventEndOffset >= _releaseStart;

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
                _lastEnvelope = MAX_PHASE - ( SAMPLE_TYPE ) readOffset * _decayIncrement;

            // sustain envelope (takes volume from last amplitude envelope and thus requires no action)

            // TODO : release (requires "noteOff" to be part of the given AudioBuffer)...
            //else if ( applyRelease && readOffset >= _releaseStart )

            // apply the calculated amplitude envelope onto the sample

            if ( _lastEnvelope < 0.0 )
                _lastEnvelope = 0.0;

            targetBuffer[ i ] *= _lastEnvelope;
        }
    }
    return _lastEnvelope;
}

ADSR* ADSR::clone()
{
    ADSR* out = new ADSR();

    out->setBufferLength( getBufferLength() );
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

    _attack = attack;

    if ( _attack > MAX_PHASE )
        _attack = MAX_PHASE;

    // no attack set ? WRONG! let's open a very minimal
    // one to prevent popping during sound start ;)

    else if ( _attack == 0 && _bufferLength > 0 )
        _attack = ( DEFAULT_FADE_DURATION / _bufferLength );

    _attackDuration  = ( int ) ( _bufferLength * _attack );
    _attackIncrement = MAX_PHASE / ( SAMPLE_TYPE ) _attackDuration;

    // 2. DECAY which can now be calculated as it takes its start
    // offset relative from the attack...

    _decay = decay;

    if ( _decay > MAX_PHASE )
    {
        _decay = MAX_PHASE;
    }
    // no decay and release set ? WRONG! we create a very minimal
    // decay to prevent "popping" during sound end by making sure all sound fades out
    /* // actually no, this mutes live synthesized notes we might want to let ring forever!
    else if ( _decay <= 0 && _release == 0 )
    {
        _decay      = ( SAMPLE_TYPE ) DEFAULT_FADE_DURATION / ( SAMPLE_TYPE ) _bufferLength;
        _decayStart = _bufferLength;
    }
    */
    _decayDuration = ( int ) ( _bufferLength * _decay );
    _decayStart    = _bufferLength - _decayDuration; // TODO: use attack end?

    int delta = _bufferLength - _decayStart;

    if ( delta > 0 )
        _decayIncrement = MAX_PHASE / ( SAMPLE_TYPE ) delta;
    else
        _decayIncrement = 0.0;

    // 3. SUSTAIN which can now be calculated as it takes its start
    // offset relative from the decay...

    _sustain = sustain;

    if ( _sustain > MAX_PHASE )
        _sustain = MAX_PHASE;

    bool hasDecay = _decay > 0;
    _sustainStart = hasDecay ? ( _decayStart + _decayDuration ) : _attackDuration;

    // TODO: here we start the sustain as soon as the decay (or attack if no decay was set) has ended
    // while we should alter the decay time to the sustain (to not "fade out" completely)
    // we disregard the release as it is part of a "noteOff" and we need to overthink this ;)
    _sustainDuration  = _bufferLength - _sustainStart;
    _sustainIncrement = MAX_PHASE / ( SAMPLE_TYPE ) _sustainDuration;

    // 4. RELEASE TODO : do we update the release envelope relative to the sustain ?
    _release = release;

    if ( _release > MAX_PHASE )
        _release = MAX_PHASE;

    // TODO : finish this implementation
}

void ADSR::invalidateEnvelopes()
{
    // resetting the attack will recursively update the remaining envelopes internally
    setAttack( getAttack() );

    _lastEnvelope = MAX_PHASE;
}
