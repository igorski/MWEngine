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
#include "synthevent.h"
#include "../audioengine.h"
#include "../sequencer.h"
#include "../global.h"
#include <definitions/waveforms.h>
#include <utilities/utils.h>
#include <utilities/bufferutility.h>
#include <cmath>

/* constructor / destructor */

/**
 * initializes an SynthEvent for use in a sequenced context
 *
 * @param aFrequency   {int}    the frequency of the note to synthesize in Hz
 * @param aPosition    {int}    the step position in the sequencer
 * @param aLength      {float} the length in steps the note lasts
 * @param aInstrument  {SynthInstrument} the instruments properties
 * @param aAutoCache   {bool} whether to start caching automatically, this is
 *                     only available if AudioEngineProps::EVENT_CACHING is true
 */
SynthEvent::SynthEvent( float aFrequency, int aPosition, float aLength,
                        SynthInstrument *aInstrument, bool aAutoCache )
{
    init( aInstrument, aFrequency, aPosition, aLength, true, false );
    setAutoCache( aAutoCache );
}

/**
 * initializes an SynthEvent for use in a sequenced context
 *
 * @param aFrequency   {int}    the frequency of the note to synthesize in Hz
 * @param aPosition    {int}    the step position in the sequencer
 * @param aLength      {float} the length in steps the note lasts
 * @param aInstrument  {SynthInstrument} the instruments properties
 * @param aAutoCache   {bool} whether to start caching automatically, this is
 *                     only available if AudioEngineProps::EVENT_CACHING is true
 * @param hasParent    {bool} whether to omit creation of OSC2 (blocks endless recursion)
 */
SynthEvent::SynthEvent( float aFrequency, int aPosition, float aLength,
                        SynthInstrument *aInstrument, bool aAutoCache, bool hasParent )
{
    init( aInstrument, aFrequency, aPosition, aLength, true, hasParent );
    setAutoCache( aAutoCache );
}

/**
 * initializes an SynthEvent to be synthesized at once, for a live instrument context
 *
 * @param aFrequency  {int} the frequency of the note to synthesize in Hz
 * @param aInstrument {SynthInstrument}
 */
SynthEvent::SynthEvent( float aFrequency, SynthInstrument *aInstrument )
{
    init( aInstrument, aFrequency, 0, 1, false, false );
}

/**
 * initializes an SynthEvent to be synthesized at once, for a live instrument context
 *
 * @param aFrequency  {int} the frequency of the note to synthesize in Hz
 * @param aInstrument {SynthInstrument}
 * @param hasParent    {bool} whether to omit creation of OSC2 (blocks endless recursion)
 */
SynthEvent::SynthEvent( float aFrequency, SynthInstrument *aInstrument, bool hasParent )
{
    init( aInstrument, aFrequency, 0, 1, false, hasParent );
}

SynthEvent::~SynthEvent()
{
    if ( _ringBuffer != 0 )
    {
        delete _ringBuffer;
        _ringBuffer = 0;
    }

    if ( _arpeggiator != 0 )
    {
        delete _arpeggiator;
        _arpeggiator = 0;
    }

    destroyOSC2();
}

/* public methods */

void SynthEvent::setFrequency( float aFrequency )
{
    setFrequency( aFrequency, true, true );
}

void SynthEvent::setFrequency( float aFrequency, bool allOscillators, bool storeAsBaseFrequency )
{
    _frequency = aFrequency;
    _phaseIncr = aFrequency / AudioEngineProps::SAMPLE_RATE;

    // store as base frequency (acts as a reference "return point" for pitch shifting modules)
    if ( storeAsBaseFrequency )
        _baseFrequency = aFrequency;

    if ( _type == WaveForms::KARPLUS_STRONG )
        initKarplusStrong();

    // update properties of secondary oscillator, note that OSC2 can
    // have a pitch that deviates from the first oscillator
    // as such we multiply it by the deviation of the new frequency
    if ( allOscillators && _osc2 != 0 )
    {
        //float multiplier = aFrequency / currentFreq;
        //_osc2->setFrequency( _osc2->_frequency * multiplier, true, storeAsBaseFrequency );
        createOSC2( position, length, _instrument );
    }
}

void SynthEvent::enqueueFrequency( float aFrequency )
{
    if ( !_rendering )
        setFrequency( aFrequency );
    else
        _queuedFrequency = aFrequency;
}

float SynthEvent::getBaseFrequency()
{
    return _baseFrequency;
}

/**
 * @param aPosition position in the sequencer where this event starts playing
 * @param aLength length (in sequencer steps) of this event
 * @param aInstrument the SynthInstrument whose properties will be used for synthesizing this event
 *                    can be NULL to keep the current SynthInstrument, if not null the current
 *                    SynthInstrument is replaced
 */
void SynthEvent::invalidateProperties( int aPosition, float aLength, SynthInstrument *aInstrument )
{
    // additional magic for secondary oscillator
    if ( _osc2 != 0 )
    {
        if ( AudioEngineProps::EVENT_CACHING && !_osc2->_cachingCompleted )
            _osc2->_cancel = true;
    }
    BaseSynthEvent::invalidateProperties( aPosition, aLength, aInstrument );
}

void SynthEvent::calculateBuffers()
{
    // we override the entire function body as we need some
    // oscillator 2-specific operations in here

    if ( _locked )
    {
        _updateAfterUnlock = true;
        return;
    }
    int oldLength;

    if ( isSequenced )
    {
        _cancel = true;

        oldLength     = _sampleLength;
        _sampleLength = ( int )( length * ( float ) AudioEngine::bytes_per_tick );
        _sampleStart  = position * AudioEngine::bytes_per_tick;
        _sampleEnd    = _sampleStart + _sampleLength;
    }
    else {
        // quick releases of a noteOn-instruction should ring for at least a 64th note
        _minLength    = AudioEngine::bytes_per_bar / 64;
        _sampleLength = AudioEngine::bytes_per_bar;     // important for amplitude swell in
        oldLength     = AudioEngineProps::BUFFER_SIZE;  // buffer is as long as the engine's buffer size
        _hasMinLength = false;                          // keeping track if the min length has been rendered
    }

    _adsr->setBufferLength( _sampleLength );

    // sample length changed (f.i. tempo change) or buffer not yet created ? create buffer for (new) length

    if ( _sampleLength != oldLength )
    {
        if ( !hasParent ) // OSC2 generates no buffer (writes into parent buffer, saves memory)
        {
            // note that when event caching is enabled, the buffer is as large as
            // the total event length requires

            if ( AudioEngineProps::EVENT_CACHING && isSequenced )
            {
                destroyBuffer(); // clear previous buffer contents
                _buffer = new AudioBuffer( AudioEngineProps::OUTPUT_CHANNELS, _sampleLength );
            }
            else
                _buffer = new AudioBuffer( AudioEngineProps::OUTPUT_CHANNELS, AudioEngineProps::BUFFER_SIZE );

            // though this event manages its child oscillator, the render method needs these properties
            if ( _osc2 != 0 ) {
                _osc2->_sampleLength = _sampleLength;
                _osc2->_sampleStart  = _sampleStart;
                _osc2->_sampleEnd    = _sampleEnd;
            }
        }
    }

    if ( isSequenced )
    {
        if ( _type == WaveForms::KARPLUS_STRONG )
            initKarplusStrong();

        if ( AudioEngineProps::EVENT_CACHING )
        {
            resetCache(); // yes here, not in cache()-invocation as cancels might otherwise remain permanent (see BulkCacher)

            if ( _autoCache && !_caching ) // re-cache
                cache( false );
        }
    }
}

/**
 * (pre-)cache the contents of the SynthEvent in its entirety
 * this can be done in idle time to make optimum use of resources
 */
void SynthEvent::cache( bool doCallback )
{
    if ( _buffer == 0 ) return; // cache request was invoked after destruction
    if ( hasParent )    return; // OSC2 is rendered via parents render

    _caching = true;

    doCache();

    if ( doCallback )
        sequencer::bulkCacher->cacheQueue();
}

/* protected methods */

void SynthEvent::updateProperties()
{
    _type = _instrument->waveform;

    bool doOSC2 = !hasParent && _instrument->osc2active; // multi oscillator ?

    if ( doOSC2 ) // note we don't dispose osc2 during this events lifetime
        createOSC2( position, length, _instrument );

    applyModules( _instrument );        // modules
    BaseSynthEvent::updateProperties(); // base method

    if ( doOSC2 )
        _osc2->_cancel = false;
}

/**
 * the actual synthesizing of the audio
 *
 * @param aOutputBuffer {AudioBuffer*} the buffer to write into
 */
void SynthEvent::render( AudioBuffer* aOutputBuffer )
{
    _rendering = true;

    int i;
    int bufferLength = aOutputBuffer->bufferSize;

    SAMPLE_TYPE amp = 0.0;
    SAMPLE_TYPE tmp, am, dpw, pmv;

    bool hasOSC2 = !hasParent && _instrument->osc2active; // no 0-check on _osc2 here as it might just be inactive

    int renderStartOffset = AudioEngineProps::EVENT_CACHING && isSequenced ? _cacheWriteIndex : 0;

    int maxSampleIndex  = _sampleLength - 1;                // max index possible for this events length
    int renderEndOffset = renderStartOffset + bufferLength; // max buffer index to be written to in this cycle

    // keep in bounds of event duration
    if ( renderEndOffset > maxSampleIndex )
    {
        renderEndOffset = maxSampleIndex;
        // silence buffers as we won't overwrite the remainder beyond above offset
        if ( !hasParent ) aOutputBuffer->silenceBuffers(); // only parent buffer is re-used
    }

    for ( i = renderStartOffset; i < renderEndOffset; ++i )
    {
        switch ( _type )
        {
            case WaveForms::SINE:

                // ---- Sine wave

                if ( _phase < .5 )
                {
                    tmp = ( _phase * 4.0 - 1.0 );
                    amp = ( 1.0 - tmp * tmp );
                }
                else {
                    tmp = ( _phase * 4.0 - 3.0 );
                    amp = ( tmp * tmp - 1.0 );
                }

                // sines need some extra love :
                // for one: a fade-in and fade-out at the start to prevent POP!ping...

                if ( isSequenced && !hasParent ) // unless this isn't the main oscillator
                {
                    int curWritePos = _cacheWriteIndex + i;

                    if ( curWritePos < _fadeInDuration )
                        amp *= ( SAMPLE_TYPE ) curWritePos / ( SAMPLE_TYPE ) _fadeInDuration;

                    else if ( curWritePos >= ( maxSampleIndex - _fadeOutDuration ))
                        amp *= ( SAMPLE_TYPE ) ( maxSampleIndex - ( curWritePos  )) / ( SAMPLE_TYPE ) _fadeOutDuration;
                }

                if ( !isSequenced )
                    amp *= .7;  // we're anticipating multi timbral use, bring down the level a tad.

                break;

            case WaveForms::SAWTOOTH:

                // ---- Sawtooth
                amp = ( _phase < 0 ) ? _phase - ( int )( _phase - 1 ) : _phase - ( int )( _phase );

                break;

            case WaveForms::SQUARE:

                // ---- Square wave
                if ( _phase < .5 )
                {
                    tmp = TWO_PI * ( _phase * 4.0 - 1.0 );
                    amp = ( 1.0 - tmp * tmp );
                }
                else {
                    tmp = TWO_PI * ( _phase * 4.0 - 3.0 );
                    amp = ( tmp * tmp - 1.0 );
                }
                amp *= .01; // these get loud !
                break;

            case WaveForms::TRIANGLE:

                // ---- triangle
                if ( _phase < .5 )
                {
                    tmp = ( _phase * 4.0 - 1.0 );
                    amp = ( 1.0 - tmp * tmp );
                }
                else {
                    tmp = ( _phase * 4.0 - 3.0 );
                    amp = ( tmp * tmp - 1.0 );
                }
                // the actual triangulation function
                amp = amp < 0 ? -amp : amp;

                break;

            case WaveForms::PWM:

                // --- pulse width modulation
                pmv = i + ( ++_pwmValue ); // i + event position

                dpw = sin( pmv / 0x4800 ) * pwr; // LFO -> PW
                amp = _phase < PI - dpw ? pwAmp : -pwAmp;

                // PWM has its own phase update operation
                _phase = _phase + ( TWO_PI_OVER_SR * _frequency );
                _phase = _phase > TWO_PI ? _phase - TWO_PI : _phase;

                // we multiply the amplitude as PWM results in a "quieter" wave
                amp *= 4;

                /* // OLD: oscillation modulating the PW wave
                am   = sin( pmv / 0x1000 ); // LFO -> AM
                amp *= am;
                */
                break;

            case WaveForms::NOISE:

                // --- noise
                if ( _phase < .5 )
                {
                    tmp = ( _phase * 4.0 - 1.0 );
                    amp = ( 1.0 - tmp * tmp );
                }
                else {
                    tmp = ( _phase * 4.0 - 3.0 );
                    amp = ( tmp * tmp - 1.0 );
                }
                // above we calculated pitch, now we add some
                // randomization to the signal for the actual noise
                amp *= randomFloat();
                break;

            case WaveForms::KARPLUS_STRONG:

                // --- Karplus-Strong algorithm for plucked string-sound (0.990f being energy decay factor)
                _ringBuffer->enqueue(( 0.990f * (( _ringBuffer->dequeue() + _ringBuffer->peek()) / 2 )));
                amp = _ringBuffer->peek();

                break;
        }

        // --- _phase update operations
        if ( _type != WaveForms::PWM )
        {
            _phase += _phaseIncr;

            // restore _phase, max range is 0 - 1 ( float )
            if ( _phase > MAX_PHASE )
                _phase -= MAX_PHASE;
        }

        // update modules
        if ( _arpeggiator != 0 )
        {
            // step the arpeggiator to the next position
            if ( _arpeggiator->peek()) {
                SAMPLE_TYPE arpeggioBase = _queuedFrequency != 0 ? _queuedFrequency : _baseFrequency;
                setFrequency( _arpeggiator->getPitchForStep( _arpeggiator->getStep(), _baseFrequency ), true, false );
            }
        }

        // frequency update operations (KP-buffer might be re-initialized, hence outside write loop)
        if ( _queuedFrequency != 0 )
        {
            setFrequency( _queuedFrequency, true, true );
            _queuedFrequency = 0;
        }

        if ( _update ) updateProperties(); // if an update was requested, do it now (prior to committing to buffer)
        if ( _cancel ) break;              // if a caching cancel was requested, cancel now

        // -- write the output into the buffers channels
        if ( hasOSC2 ) amp *= .5;

        for ( int c = 0, ca = aOutputBuffer->amountOfChannels; c < ca; ++c )
            aOutputBuffer->getBufferForChannel( c )[ i ] = amp * _volume;
    }

    // secondary oscillator ? render its contents into this (parent) buffer
    if ( hasOSC2 && !_cancel )
    {
        // create a temporary buffer (this prevents writing to deleted buffers
        // when the parent event changes its _buffer properties (f.i. tempo change)
        int tempLength = ( AudioEngineProps::EVENT_CACHING && isSequenced ) ? ( renderEndOffset - _cacheWriteIndex ) : bufferLength;
        AudioBuffer* tempBuffer = new AudioBuffer( aOutputBuffer->amountOfChannels, tempLength );
        _osc2->render( tempBuffer );
        aOutputBuffer->mergeBuffers( tempBuffer, 0, renderStartOffset, MAX_PHASE * .5 );

        delete tempBuffer; // free allocated memory
    }

    // apply envelopes and update cacheWriteIndex for next render cycle
    if ( !hasParent )
    {
        _adsr->apply( aOutputBuffer, _cacheWriteIndex );
        _cacheWriteIndex += i;
    }

    if ( AudioEngineProps::EVENT_CACHING )
    {
        if ( isSequenced )
        {
            _caching = false;

            // was a cancel requested ? re-cache to match the new instrument properties (cancel
            // may only be requested during the changing of properties!)
            if ( _cancel )
            {
                //calculateBuffers(); // no, use a manual invocation (BulkCacher)
            }
            else
            {
                if ( i == maxSampleIndex )
                    _cachingCompleted = true;

                if ( _bulkCacheable )
                    _autoCache = true;
            }
        }
    }
    _cancel    = false; // ensure we can render for the next iteration
    _rendering = false;
}

void SynthEvent::setDeletable( bool value )
{
    BaseSynthEvent::setDeletable( value );

    // secondary oscillator too

    if ( _osc2 != 0 )
        _osc2->setDeletable( value );
}

/**
 * @param aInstrument pointer to the SynthInstrument containing the rendering properties for the SynthEvent
 * @param aFrequency  frequency in Hz for the note to be rendered
 * @param aPosition   offset in the sequencer where this event starts playing / becomes audible
 * @param aLength     length of the event (in sequencer steps)
 * @param aHasParent  when true, this SynthEvent will be merged into the buffer of its parent instead of being
 *                    added to the Sequencer as an individual event, this makes rendering of its parent event
 *                    draw more CPU as its rendering multiple buffers, but after merging it consumes less memory
 *                    than two individual buffers would, it also omits the need of having float SynthEvents
 *                    to be mixed by the Sequencer
 * @param aIsSequenced whether this event is sequenced and only audible in a specific sequence range
 */
void SynthEvent::init( SynthInstrument *aInstrument, float aFrequency, int aPosition,
                       int aLength, bool aIsSequenced, bool aHasParent )
{
    _ringBuffer      = 0;
    _ringBufferSize  = 0;
    _osc2            = 0;
    _frequency       = aFrequency;
    _baseFrequency   = aFrequency;
    _queuedFrequency = 0;
    hasParent        = aHasParent;

    // constants used by waveform generators

    TWO_PI_OVER_SR         = TWO_PI / AudioEngineProps::SAMPLE_RATE;
    pwr                    = PI / 1.05;
    pwAmp                  = 0.075;
    _pwmValue              = 0.0;
    _phase                 = 0.0;
    _type                  = aInstrument->waveform;

    // starting/stopping a waveform mid cycle can cause nasty pops, this is used for a smoother inaudible fade in
    _fadeInDuration  = BufferUtility::millisecondsToBuffer( 20, AudioEngineProps::SAMPLE_RATE );
    _fadeOutDuration = BufferUtility::millisecondsToBuffer( 30, AudioEngineProps::SAMPLE_RATE );

    // create the secondary oscillator

    if ( !hasParent && aInstrument->osc2active )
       createOSC2( position, length, aInstrument );

    // base class invocation
    BaseSynthEvent::init( aInstrument, aFrequency, aPosition, aLength, aIsSequenced );

    // modules

    _arpeggiator = 0;
    applyModules( aInstrument );
}

void SynthEvent::addToSequencer()
{
    // OSC2 contents aren't added to the sequencer
    // individually as their render is invoked by their parent,
    // writing directly into their parent buffer (saves memory overhead)

    if ( hasParent )
        return;

    BaseSynthEvent::addToSequencer();
}

void SynthEvent::resetCache()
{
    BaseCacheableAudioEvent::resetCache();

    if ( _osc2 != 0 )
        _osc2->resetCache();
}

/* private methods */

/**
 * creates a new/updates an existing secondary oscillator
 *
 * @param aPosition {int} sequencer position where the event starts playing
 * @param aLength   {int} duration (in sequencer steps) the event keeps playing
 * @param aInstrument {SynthInstrument} the synth instrument whose properties are used for synthesis
 */
void SynthEvent::createOSC2( int aPosition, int aLength, SynthInstrument *aInstrument )
{
    if ( aInstrument->osc2active )
    {
        // note no auto caching for a sequenced OSC2, its render is invoked by its parent (=this) event!
        if ( _osc2 == 0 )
        {
            // note the alternate constructors (avoids recursion into nested oscillators)
            if ( !isSequenced )
                _osc2 = new SynthEvent( _frequency, aInstrument, true );
            else
                _osc2 = new SynthEvent( _frequency, aPosition, aLength, aInstrument, false, true );
        }
        _osc2->_type    = aInstrument->osc2waveform;
        _osc2->position = aPosition;
        _osc2->length   = aLength;

        float lfo2Tmpfreq = _frequency + ( _frequency / 1200 * aInstrument->osc2detune ); // 1200 cents == octave
        float lfo2freq    = lfo2Tmpfreq;

        // octave shift ( -2 to +2 )
        if ( aInstrument->osc2octaveShift != 0 )
        {
            if ( aInstrument->osc2octaveShift < 0 )
                lfo2freq = lfo2Tmpfreq / std::abs(( float ) ( aInstrument->osc2octaveShift * 2 ));
            else
                lfo2freq += ( lfo2Tmpfreq * std::abs(( float ) ( aInstrument->osc2octaveShift * 2 ) - 1 ));
        }
        // fine shift ( -7 to +7 )
        float fineShift = ( lfo2Tmpfreq / 12 * std::abs( aInstrument->osc2fineShift ));

        if ( aInstrument->osc2fineShift < 0 )
            lfo2freq -= fineShift;
         else
            lfo2freq += fineShift;

        _osc2->setFrequency( lfo2freq );
    }
}

void SynthEvent::destroyOSC2()
{
    if ( _osc2 != 0 )
    {
        if ( _osc2->_rendering )
            _osc2->_cancel = true;

        delete _osc2;
        _osc2 = 0;
    }
}

void SynthEvent::applyModules( SynthInstrument* instrument )
{
    bool hasOSC2   = _osc2 != 0;
    float OSC2freq = hasOSC2 ? _osc2->_baseFrequency : _baseFrequency;

    if ( instrument->arpeggiatorActive )
    {
        if ( _arpeggiator == 0 )
            _arpeggiator = instrument->arpeggiator->clone();
        else
            instrument->arpeggiator->cloneProperties( _arpeggiator );
    }
    else if ( _arpeggiator != 0 )
    {
        delete _arpeggiator;
        _arpeggiator = 0;
    }

    if ( hasOSC2 )
        _osc2->applyModules( instrument );

    // pitch shift module active ? make sure current frequency
    // matches the current arpeggiator step
    if ( instrument->arpeggiatorActive )
    {
        setFrequency( _arpeggiator->getPitchForStep( _arpeggiator->getStep(), _baseFrequency ), true, false );
    }
    else
    {
        // restore base frequency upon deactivation of pitch shift modules
        setFrequency( _baseFrequency, false, true );

        if ( hasOSC2 )
            _osc2->setFrequency( OSC2freq, false, true );
    }
}

void SynthEvent::initKarplusStrong()
{
    // reset previous _ringBuffer data
    int prevRingBufferSize = _ringBufferSize;
    _ringBufferSize        = ( int ) ( AudioEngineProps::SAMPLE_RATE / _frequency );
    bool newSize           = _ringBufferSize != prevRingBufferSize;

    if ( isSequenced && ( _ringBuffer != 0 && newSize ))
    {
        delete _ringBuffer;
        _ringBuffer = 0;
    }

    if ( _ringBuffer == 0 )
        _ringBuffer = new RingBuffer( _ringBufferSize );
    else
        _ringBuffer->flush();

    // fill the ring buffer with noise ( initial "pluck" of the "string" )
    for ( int i = 0; i < _ringBufferSize; i++ )
        _ringBuffer->enqueue( randomFloat());
}
