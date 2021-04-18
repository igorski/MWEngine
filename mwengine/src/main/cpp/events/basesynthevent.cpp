/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2021 Igor Zinken - https://www.igorski.nl
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

namespace MWEngine {

unsigned int BaseSynthEvent::INSTANCE_COUNT = 0;

/* constructors / destructor */

BaseSynthEvent::BaseSynthEvent()
{
    _synthInstrument = nullptr;
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
    cachedProps.envelope       = ( _synthInstrument->adsr->getAttackTime() > 0 ) ? 0.0 : 1.0;

    BaseAudioEvent::play();
}

void BaseSynthEvent::stop()
{
    triggerRelease();

    if ( !isSequenced ) {
        // live events must play their full release envelope before removal
        // if there is no positive release envelope, apply a short decay
        // so we know events are audible for at least a 64th
        _minLength = std::max(
            _synthInstrument->adsr->getReleaseDuration(),
            AudioEngine::samples_per_bar / 64
        );
        _hasMinLength = false;

        setDeletable( true );
    }
}

int BaseSynthEvent::getEventEnd()
{
    // SynthEvents might have a longer duration if they have a positive release envelope
    return BaseAudioEvent::getEventEnd() + _synthInstrument->adsr->getReleaseDuration();
}

bool BaseSynthEvent::isQueuedForDeletion()
{
    return _queuedForDeletion;
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

void BaseSynthEvent::repositionToTempoChange( float ratio )
{
    auto orgStart  = ( float ) _eventStart;
    auto orgLength = ( float ) _eventLength;

    setEventStart(( int )( orgStart  * ratio ));

    // for synthesized events, we adjust the event duration in relation to the tempo change

    setEventLength(( int )( orgLength * ratio ));
}

void BaseSynthEvent::calculateBuffers()
{
    if ( _locked )
    {
        _updateAfterUnlock = true;
        return;
    }

    if ( isSequenced )
    {
        setEventStart( position * AudioEngine::samples_per_step );
        setEventLength(( int )( length * AudioEngine::samples_per_step ));
    }
    else {
        // quick releases of a noteOn-instruction should ring for at least a 64th note
        setEventLength( AudioEngine::samples_per_bar );     // important for amplitude swell in
        _minLength    = AudioEngine::samples_per_bar / 64;
        _hasMinLength = false;                          // keeping track if the min length has been rendered
    }

    // buffer is only instantiated once as it is the size of the engines BUFFER_SIZE
    // (this event will not be cached in its entirety but will repeatedly render snippets into its buffer)

    if ( _buffer == nullptr )
        _buffer = new AudioBuffer( AudioEngineProps::OUTPUT_CHANNELS, AudioEngineProps::BUFFER_SIZE );

    if ( isSequenced && _synthInstrument != nullptr )
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

    int bufferEndPos = bufferPos + outputBuffer->bufferSize;

    // we use the overridden getter method as the event length can be
    // extended by a positive release time on the instruments ADSR envelope
    int eventEnd = getEventEnd();

    if (( bufferPos >= _eventStart || bufferEndPos > _eventStart ) &&
          bufferPos < eventEnd )
    {
        lastWriteIndex  = _eventStart > bufferPos ? 0 : bufferPos - _eventStart;
        int writeOffset = _eventStart > bufferPos ? _eventStart - bufferPos : 0;

        // if we're mixing into the ADSR release tail, make sure release envelope is triggered

        if ( bufferPos > _eventEnd ) {
            if ( !released ) {
                triggerRelease();
            }
        }
        else {
            released = false;
        }

        // unset previous buffer contents
        _buffer->silenceBuffers();

        // render the snippet
        _synthInstrument->synthesizer->render( _buffer, this );

        // note we merge using 1.0 as mix volume (event volume was applied during synthesis)
        outputBuffer->mergeBuffers( _buffer, 0, writeOffset, 1.0 );

        // reset of event properties at end of write
        if ( lastWriteIndex >= _eventLength )
            calculateBuffers();
    }

    if ( loopStarted )
    {
        // Sequencer has started its loop, generate a new render from the beginning of
        // the Sequencer position for the total amount of loop frames in length

        int totalSamplesToWrite = outputBuffer->bufferSize - loopOffset;

        if (( minBufferPosition >= _eventStart || ( minBufferPosition + totalSamplesToWrite ) > _eventStart ) &&
              minBufferPosition < eventEnd )
        {
            // render the snippet from the starts
            calculateBuffers();
            lastWriteIndex = 0;

            // TODO: specify total render range in ::render method ? this would avoid unnecessary buffer merging ;)
            // also synthesizer now renders a full output buffer size (wasteful)

            _synthInstrument->synthesizer->render( _buffer, this ); // overwrites previous buffer contents

            // note we merge using 1.0 as mix volume (event volume was applied during synthesis)
            outputBuffer->mergeBuffers( _buffer, 0, loopOffset, 1.0 );

            // update the last write index so the next iteration can pick up
            // rendering from the last audible sample
            lastWriteIndex = totalSamplesToWrite;
        }
    }
    unlock();
}

/**
 * Invoked by the Sequencer in case this event isn't sequenced
 * but triggered manually via a "noteOn" / "noteOff" operation for instant "live" playback
 */
void BaseSynthEvent::mixBuffer( AudioBuffer* outputBuffer )
{
    lock();

    int bufferSize = outputBuffer->bufferSize;
    _synthInstrument->synthesizer->render( outputBuffer, this );

    // keep track of the rendered samples, in case of a key up event
    // we still want to have the sound ring for the minimum period
    // defined in the constructor instead of cut off immediately

    if ( _queuedForDeletion )
    {
        _minLength -= bufferSize;

        if ( _minLength <= 0 )
        {
            _hasMinLength = true;
            // we can now remove this event from the Sequencer
            setDeletable( _queuedForDeletion );
            BaseAudioEvent::stop();

            // this event is about to be deleted, apply a tiny fadeout
            if ( _queuedForDeletion )
            {
                int amt = ( int ) ceil( bufferSize / 4 );

                SAMPLE_TYPE envIncr = 1.0 / amt;
                SAMPLE_TYPE amp     = 1.0;

                for ( int i = bufferSize - amt; i < bufferSize; ++i )
                {
                    for ( int c = 0, nc = _buffer->amountOfChannels; c < nc; ++c )
                        _buffer->getBufferForChannel( c )[ i ] *= amp;

                    amp -= envIncr;
                }
            }
        }
    }
    unlock();
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
    cachedProps.envelope     = ( _synthInstrument->adsr->getAttackTime() > 0 ) ? 0.0 : 1.0;
    cachedProps.releaseLevel = ( SAMPLE_TYPE ) _synthInstrument->adsr->getSustainLevel();

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

void BaseSynthEvent::triggerRelease()
{
    released = true;

    // it is possible the ADSR module was going through its attack or decay
    // phases, let the release envelope operate from the current level
    cachedProps.releaseLevel   = cachedProps.envelope;
    cachedProps.envelopeOffset = _synthInstrument->adsr->getReleaseStartOffset();
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

} // E.O namespace MWEngine
