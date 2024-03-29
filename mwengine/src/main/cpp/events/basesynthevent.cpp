/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2022 Igor Zinken - https://www.igorski.nl
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
    // uses BaseAudioEvent constructor
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
    init( aInstrument, aFrequency, 0, 1, false );
}

BaseSynthEvent::~BaseSynthEvent()
{

}

/* public methods */

void BaseSynthEvent::setSequencePosition( int positionInSequencerSteps )
{
    _position = positionInSequencerSteps;
    setEventStart( positionInSequencerSteps * AudioEngine::samples_per_step );
}

int BaseSynthEvent::getSequencePosition()
{
    return _position;
}
void BaseSynthEvent::setSequenceDuration( float durationInSequencerSteps )
{
    _duration = durationInSequencerSteps;
    setEventLength(( int )( durationInSequencerSteps * AudioEngine::samples_per_step ));
}

float BaseSynthEvent::getSequenceDuration()
{
    return _duration;
}

void BaseSynthEvent::play()
{
    released = false;

    enqueueRemoval( false );
    _hasMinLength = false;
    _shouldEnqueueRemoval = false;

    lastWriteIndex             = 0;
    cachedProps.envelopeOffset = 0;
    cachedProps.envelope       = ( getSynthInstrument()->adsr->getAttackTime() > 0 ) ? 0.0 : 1.0;

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
            getSynthInstrument()->adsr->getReleaseDuration(),
            AudioEngine::samples_per_bar / 64
        );
        _hasMinLength = false;

        enqueueRemoval( true );
    }
}

int BaseSynthEvent::getEventEnd()
{
    // SynthEvents might have a longer duration if they have a positive release envelope
    return BaseAudioEvent::getEventEnd() + getSynthInstrument()->adsr->getReleaseDuration();
}

bool BaseSynthEvent::shouldEnqueueRemoval()
{
    return _shouldEnqueueRemoval;
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

    getSynthInstrument()->synthesizer->initializeEventProperties( this, false );
}

SAMPLE_TYPE BaseSynthEvent::getPhaseForOscillator( int aOscillatorNum )
{
    return cachedProps.oscillatorPhases.at( aOscillatorNum );
}

void BaseSynthEvent::setPhaseForOscillator( int aOscillatorNum, SAMPLE_TYPE aPhase )
{
    cachedProps.oscillatorPhases.at( aOscillatorNum ) = aPhase;
}

void BaseSynthEvent::unlock()
{
    _locked = false;

    if ( _updateAfterUnlock ) {
        invalidateProperties();
    }
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

void BaseSynthEvent::invalidateProperties()
{
    if ( _locked )
    {
        _updateAfterUnlock = true;
        return;
    }

    if ( !isSequenced ) {
        // quick releases of a noteOn-instruction should ring for at least a 64th note
        setEventLength( AudioEngine::samples_per_bar ); // important for amplitude swell in
        _minLength    = AudioEngine::samples_per_bar / 64;
        _hasMinLength = false; // keeping track if the min length has been rendered
    }

    if ( isSequenced && _instrument != nullptr ) {
        getSynthInstrument()->synthesizer->initializeEventProperties( this, true );
    }
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
        if ( useChannelRange ) {
            bufferPos -= maxBufferPosition;
        } else if ( !loopStarted ) {
            return;
        }
    }

    int bufferEndPos = bufferPos + outputBuffer->bufferSize;

    // we use the overridden getter method as the event length can be
    // extended by a positive release time on the instruments ADSR envelope
    int eventEnd = getEventEnd();

    ResizableAudioBuffer* tempBuffer = getEmptyTempBuffer( outputBuffer->bufferSize );

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
        } else {
            released = false;
        }

        // render the snippet into the temp buffer
        getSynthInstrument()->synthesizer->render( tempBuffer, this );

        // merge temp buffer into the output buffer
        // note we merge using 1.0 as mix volume as the events volume was applied during synthesis
        outputBuffer->mergeBuffers( tempBuffer, 0, writeOffset, 1.0 );

        // reset of event properties at end of write
        if ( lastWriteIndex >= _eventLength ) {
            invalidateProperties();
        }
    }

    if ( loopStarted )
    {
        // Sequencer has started its loop, generate a new render from the beginning of
        // the Sequencer position for the total amount of loop frames in length

        int totalSamplesToWrite = outputBuffer->bufferSize - loopOffset;

        if (( minBufferPosition >= _eventStart || ( minBufferPosition + totalSamplesToWrite ) > _eventStart ) &&
              minBufferPosition < eventEnd )
        {
            // render the snippet from the start
            invalidateProperties();
            lastWriteIndex = 0;

            // TODO: specify total render range in ::render method ? this would avoid unnecessary buffer merging ;)
            // also synthesizer now renders a full output buffer size (wasteful)

            getSynthInstrument()->synthesizer->render( tempBuffer, this ); // overwrites previous buffer contents

            // note we merge using 1.0 as mix volume (event volume was applied during synthesis)
            outputBuffer->mergeBuffers( tempBuffer, 0, loopOffset, 1.0 );

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
    ResizableAudioBuffer* tempBuffer = getEmptyTempBuffer( bufferSize );
    getSynthInstrument()->synthesizer->render( tempBuffer, this );

    // keep track of the rendered samples, in case of a key up event
    // we still want to have the sound ring for the minimum period
    // defined in the constructor instead of cut off immediately

    if ( _shouldEnqueueRemoval )
    {
        _minLength -= bufferSize;

        if ( _minLength <= 0 )
        {
            _hasMinLength = true;
            // we can now remove this event from the Sequencer
            enqueueRemoval( true );
            BaseAudioEvent::stop();

            // this event is about to be removed, apply a tiny fadeout

            int amt = ( int ) ceil( bufferSize / 4 );

            SAMPLE_TYPE envIncr = 1.0 / amt;
            SAMPLE_TYPE amp     = 1.0;

            for ( int i = bufferSize - amt; i < bufferSize; ++i )
            {
                for ( int c = 0, nc = tempBuffer->amountOfChannels; c < nc; ++c ) {
                    tempBuffer->getBufferForChannel( c )[ i ] *= amp;
                }
                amp -= envIncr;
            }
        }
    }
    // merge temp buffer into output buffer
    outputBuffer->mergeBuffers( tempBuffer, 0, 0, 1.0 );
    unlock();
}

/* protected methods */

SynthInstrument* BaseSynthEvent::getSynthInstrument()
{
    return static_cast<SynthInstrument*>( _instrument ); // NOLINT
}

/**
 * actual updating of the properties, requested by invalidateProperties
 * this operation might potentially delete objects that could be in
 * use during a rendering operation, as such it is invoked from the render
 */
void BaseSynthEvent::updateProperties()
{
    // sync ADSR envelope values
    cachedProps.envelope     = ( getSynthInstrument()->adsr->getAttackTime() > 0 ) ? 0.0 : 1.0;
    cachedProps.releaseLevel = ( SAMPLE_TYPE ) getSynthInstrument()->adsr->getSustainLevel();

    invalidateProperties();
}

void BaseSynthEvent::enqueueRemoval( bool value )
{
    // sequenced event or synthesized event has min length ? schedule for immediate deletion

    if ( isSequenced || _hasMinLength )
        _removalEnqueued = value;
    else
        _shouldEnqueueRemoval = value;
}

void BaseSynthEvent::triggerRelease()
{
    released = true;

    // it is possible the ADSR module was going through its attack or decay
    // phases, let the release envelope operate from the current level
    cachedProps.releaseLevel   = cachedProps.envelope;
    cachedProps.envelopeOffset = getSynthInstrument()->adsr->getReleaseStartOffset();
}

ResizableAudioBuffer* BaseSynthEvent::getEmptyTempBuffer( int bufferSize )
{
    ResizableAudioBuffer* tempBuffer = getSynthInstrument()->synthesizer->getTempBuffer();
    tempBuffer->silenceBuffers();     // unset previous buffer contents
    tempBuffer->resize( bufferSize ); // equalize buffer sizes
    return tempBuffer;
}

/**
 * @param aInstrument pointer to the SynthInstrument containing the rendering properties for the BaseSynthEvent
 * @param aFrequency  frequency in Hz for the note to be rendered
 * @param aPosition   offset in the sequencer where this event starts playing / becomes audible
 * @param aDuration   duration of the event (in sequencer steps)
 * @param isSequenced whether this event is sequenced and only audible in a specific sequence range
 */
void BaseSynthEvent::init( SynthInstrument* aInstrument, float aFrequency,
                           int aPosition, float aDuration, bool isSequenced )
{
    BaseAudioEvent::init();

    instanceId         = ++INSTANCE_COUNT;
    _destroyableBuffer = true;  // synth event buffer is always unique and managed by this instance !
    _instrument        = aInstrument;

    released = false;

    cachedProps.envelopeOffset   = 0;
    cachedProps.arpeggioPosition = 0;
    cachedProps.arpeggioStep     = 0;

    int maxOscillatorAmount = 8; // let's assume a max amount of oscillators of 8 here
    cachedProps.oscillatorPhases.reserve( maxOscillatorAmount );

    for ( int i = 0; i < maxOscillatorAmount; ++i ) {
        cachedProps.oscillatorPhases.push_back( 0.0 );
    }

    this->isSequenced     = isSequenced;
    _shouldEnqueueRemoval = false;
    _removalEnqueued      = false;
    _hasMinLength         = isSequenced; // a sequenced synth event has no "early cancel"
    _eventLength          = 0;
    lastWriteIndex        = 0;

    setSequencePosition( aPosition );
    setSequenceDuration( aDuration );
    setFrequency( aFrequency );

    updateProperties();

    if ( isSequenced ) {
        addToSequencer();
    }
}

} // E.O namespace MWEngine
