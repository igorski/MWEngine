/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2018 Igor Zinken - http://www.igorski.nl
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
#include "basesynthevent.h"
#include "../audioengine.h"
#include "../sequencer.h"
#include "../global.h"
#include <instruments/synthinstrument.h>
#include <cmath>

unsigned int BaseSynthEvent::INSTANCE_COUNT = 0;

/* constructors / destructor */

BaseSynthEvent::BaseSynthEvent()
{
    _synthInstrument = 0;
}

/**
 * initializes an BaseSynthEvent for use in a sequenced context
 *
 * @param aFrequency   {int}    the frequency of the note to synthesize in Hz
 * @param aPosition    {int}    the step position in the sequencer
 * @param aLength      {float} the length in steps the note lasts
 * @param aInstrument  {SynthInstrument} the instruments properties
 */
BaseSynthEvent::BaseSynthEvent( float aFrequency, int aPosition, float aLength,
                                SynthInstrument* aInstrument )
{
    construct();
    init( aInstrument, aFrequency, aPosition, aLength, true );
}

/**
 * initializes an BaseSynthEvent to be synthesized at once, for a live instrument context
 *
 * @param aFrequency  {int} the frequency of the note to synthesize in Hz
 * @param aInstrument {SynthInstrument}
 */
BaseSynthEvent::BaseSynthEvent( float aFrequency, SynthInstrument* aInstrument )
{
    construct();
    init( aInstrument, aFrequency, 0, 1, false );
}

BaseSynthEvent::~BaseSynthEvent()
{
    --INSTANCE_COUNT;
}

/* public methods */

void BaseSynthEvent::play()
{
    released = false;

    setDeletable( false );
    _hasMinLength = false;
    _queuedForDeletion = false;

    lastWriteIndex             = 0;
    cachedProps.envelopeOffset = 0;
    cachedProps.envelope       = ( _synthInstrument->adsr->getAttackTime() > 0 ) ? 0.0 : MAX_PHASE;

    BaseAudioEvent::play();
}

void BaseSynthEvent::stop()
{
    if ( !_livePlayback )
        return;

    released = true;

    if ( !isSequenced ) {
        // live events must play their full release envelope before removal
        // if there is no positive release envelope, apply a short decay
        // so we know events are audible for at least a 64th
        _minLength = std::max(
            _synthInstrument->adsr->getReleaseDuration(),
            AudioEngine::samples_per_bar / 64
        );
        _hasMinLength = false;

        // it is possible the ADSR module was going through its attack or decay
        // phases, let the release envelope operate from the current level
        cachedProps.releaseLevel   = cachedProps.envelope;
        cachedProps.envelopeOffset = _synthInstrument->adsr->getReleaseStartOffset();

        setDeletable( true );
    }
}

int BaseSynthEvent::getEventEnd()
{
    // SynthEvents might have a longer duration if they have a positive release envelope
    return BaseAudioEvent::getEventEnd() + _synthInstrument->adsr->getReleaseDuration();
}

float BaseSynthEvent::getFrequency()
{
    return _frequency;
}

float BaseSynthEvent::getBaseFrequency()
{
    return _baseFrequency;
}

void BaseSynthEvent::setFrequency( float aFrequency )
{
    setFrequency( aFrequency, true );
}

void BaseSynthEvent::setFrequency( float aFrequency, bool storeAsBaseFrequency )
{
    _frequency            = aFrequency;
    cachedProps.phaseIncr = aFrequency / ( SAMPLE_TYPE ) AudioEngineProps::SAMPLE_RATE;

    // store as base frequency (acts as a reference "return point" for pitch shifting modules)
    if ( storeAsBaseFrequency )
        _baseFrequency = aFrequency;

    _synthInstrument->synthesizer->initializeEventProperties( this, false );
}

SAMPLE_TYPE BaseSynthEvent::getPhaseForOscillator( int aOscillatorNum )
{
    return cachedProps.oscillatorPhases.at( aOscillatorNum );
}

void BaseSynthEvent::setPhaseForOscillator( int aOscillatorNum, SAMPLE_TYPE aPhase )
{
    cachedProps.oscillatorPhases.at( aOscillatorNum ) = aPhase;
}

/**
 * @param aPosition position in the sequencer where this event starts playing
 * @param aLength length (in sequencer steps) of this event
 * @param aInstrument the SynthInstrument whose properties will be used for synthesizing this event
 *                    can be NULL to keep the current SynthInstrument, if not null the current
 *                    SynthInstrument is replaced
 */
void BaseSynthEvent::invalidateProperties( int aPosition, float aLength, SynthInstrument* aInstrument )
{
    setInstrument( aInstrument );
    position = aPosition;
    length   = aLength;

    updateProperties(); // instant update as we're not rendering
}

void BaseSynthEvent::unlock()
{
    _locked = false;

    if ( _updateAfterUnlock )
        calculateBuffers();

    _updateAfterUnlock = false;
}

void BaseSynthEvent::calculateBuffers()
{
    if ( _locked )
    {
        _updateAfterUnlock = true;
        return;
    }
    int oldLength;

    if ( isSequenced )
    {
        oldLength = _eventLength;
        setEventStart( position * ( int ) AudioEngine::samples_per_step );
        setEventLength(( int )( length * AudioEngine::samples_per_step ));
        setEventEnd( _eventStart + _eventLength );
    }
    else {
        // quick releases of a noteOn-instruction should ring for at least a 64th note
        setEventLength( AudioEngine::samples_per_bar );     // important for amplitude swell in
        _minLength    = AudioEngine::samples_per_bar / 64;
        oldLength     = AudioEngineProps::BUFFER_SIZE;  // buffer is as long as the engine's buffer size
        _hasMinLength = false;                          // keeping track if the min length has been rendered
    }

    // buffer is only instantiated once as it is the size of the engines BUFFER_SIZE
    // (this event will not be cached in its entirety but will repeatedly render snippets into its buffer)

    if ( _buffer == 0 )
        _buffer = new AudioBuffer( AudioEngineProps::OUTPUT_CHANNELS, AudioEngineProps::BUFFER_SIZE );

    if ( isSequenced && _synthInstrument != 0 )
         _synthInstrument->synthesizer->initializeEventProperties( this, true );
}

/**
 * sequenced BaseSynthEvent
 */
void BaseSynthEvent::mixBuffer( AudioBuffer* outputBuffer, int bufferPos,
                                int minBufferPosition, int maxBufferPosition,
                                bool loopStarted, int loopOffset, bool useChannelRange )
{
    lock();

    // over the max position ? read from the start ( implies that sequence has started loop )
    if ( bufferPos > maxBufferPosition )
    {
        if ( useChannelRange )
            bufferPos -= maxBufferPosition;

        else if ( !loopStarted )
            return;
    }

    int bufferEndPos = bufferPos + AudioEngineProps::BUFFER_SIZE;

    if (( bufferPos >= _eventStart || bufferEndPos > _eventStart ) &&
          bufferPos < _eventEnd )
    {
        lastWriteIndex  = _eventStart > bufferPos ? 0 : bufferPos - _eventStart;
        int writeOffset = _eventStart > bufferPos ? _eventStart - bufferPos : 0;

        // render the snippet
        _synthInstrument->synthesizer->render( _buffer, this );

        // note we merge using MAX_PHASE as mix volume (event volume was applied during synthesis)
        outputBuffer->mergeBuffers( _buffer, 0, writeOffset, MAX_PHASE );

        // reset of event properties at end of write
        if ( lastWriteIndex >= _eventLength )
            calculateBuffers();
    }

    if ( loopStarted && bufferPos >= loopOffset )
    {
        bufferPos = minBufferPosition + loopOffset;

        if ( bufferPos >= _eventStart && bufferPos <= _eventEnd )
        {
            lastWriteIndex = 0; // render the snippet from the start

            // TODO: specify range in ::render method ? this would avoid unnecessary buffer merging ;)

            _synthInstrument->synthesizer->render( _buffer, this );    // overwrites old buffer contents

            // note we merge using MAX_PHASE as mix volume (event volume was applied during synthesis)
            outputBuffer->mergeBuffers( _buffer, 0, loopOffset, MAX_PHASE );

            // reset of event properties at end of write
            if ( lastWriteIndex >= _eventLength )
                calculateBuffers();
        }
    }
    unlock();
}

/**
 * synthesize is invoked by the Sequencer for rendering a non-sequenced
 * BaseSynthEvent into a single buffer
 *
 * aBufferLength {int} length of the buffer to synthesize
 */
AudioBuffer* BaseSynthEvent::synthesize( int aBufferLength )
{
    // in case buffer length is unequal to cached length, create new write buffer
    if ( aBufferLength != AudioEngineProps::BUFFER_SIZE || _buffer == 0 )
    {
        destroyBuffer();
        _buffer = new AudioBuffer( AudioEngineProps::OUTPUT_CHANNELS, aBufferLength );
    }
    lock();

    _synthInstrument->synthesizer->render( _buffer, this );

    // keep track of the rendered samples, in case of a key up event
    // we still want to have the sound ring for the minimum period
    // defined in the constructor instead of cut off immediately

    if ( _queuedForDeletion )
    {
        _minLength -= aBufferLength;

        if ( _minLength <= 0 )
        {
            _hasMinLength = true;
            // we can now remove this event from the Sequencer
            setDeletable( _queuedForDeletion );
            BaseAudioEvent::stop();

            // this event is about to be deleted, apply a tiny fadeout
            if ( _queuedForDeletion )
            {
                int amt = ceil( aBufferLength / 4 );

                SAMPLE_TYPE envIncr = MAX_PHASE / amt;
                SAMPLE_TYPE amp     = MAX_PHASE;

                for ( int i = aBufferLength - amt; i < aBufferLength; ++i )
                {
                    for ( int c = 0, nc = _buffer->amountOfChannels; c < nc; ++c )
                        _buffer->getBufferForChannel( c )[ i ] *= amp;

                    amp -= envIncr;
                }
            }
        }
    }
    unlock();

    return _buffer;
}

/* protected methods */

/**
 * actual updating of the properties, requested by invalidateProperties
 * this operation might potentially delete objects that could be in
 * use during a rendering operation, as such it is invoked from the render
 */
void BaseSynthEvent::updateProperties()
{
    // sync ADSR envelope values
    cachedProps.envelope     = ( _synthInstrument->adsr->getAttackTime() > 0 ) ? 0.0 : MAX_PHASE;
    cachedProps.releaseLevel = _synthInstrument->adsr->getSustainLevel();

    calculateBuffers();
}

void BaseSynthEvent::setDeletable( bool value )
{
    // sequenced event or synthesized event has min length ? schedule for immediate deletion

    if ( isSequenced || _hasMinLength )
        _deleteMe = value;
    else
        _queuedForDeletion = value;
}

/**
 * @param aInstrument pointer to the SynthInstrument containing the rendering properties for the BaseSynthEvent
 * @param aFrequency  frequency in Hz for the note to be rendered
 * @param aPosition   offset in the sequencer where this event starts playing / becomes audible
 * @param aLength     length of the event (in sequencer steps)
 * @param isSequenced whether this event is sequenced and only audible in a specific sequence range
 */
void BaseSynthEvent::init( SynthInstrument* aInstrument, float aFrequency,
                           int aPosition, float aLength, bool isSequenced )
{
    instanceId         = ++INSTANCE_COUNT;
    _destroyableBuffer = true;  // synth event buffer is always unique and managed by this instance !
    _instrument        = aInstrument;
    _synthInstrument   = aInstrument; // convenience reference (typecast to SynthInstrument)

    position           = aPosition;
    length             = aLength;
    released           = false;

    cachedProps.envelopeOffset   = 0;
    cachedProps.arpeggioPosition = 0;
    cachedProps.arpeggioStep     = 0;

    int maxOscillatorAmount = 8; // let's assume a max amount of oscillators of 8 here
    cachedProps.oscillatorPhases.reserve( maxOscillatorAmount );

    for ( int i = 0; i < maxOscillatorAmount; ++i )
        cachedProps.oscillatorPhases.push_back( 0.0 );

    this->isSequenced  = isSequenced;
    _queuedForDeletion = false;
    _deleteMe          = false;
    _hasMinLength      = isSequenced; // a sequenced event has no early cancel
    _eventLength       = 0;
    lastWriteIndex     = 0;

    setFrequency( aFrequency );

    updateProperties();

    if ( isSequenced )
        addToSequencer();
}
