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
#include "sequencer.h"
#include "synthevent.h"
#include "utils.h"
#include "global.h"
#include <cmath>

/* constructor / destructor */

/**
 * initializes an SynthEvent with very definitive properties to be precached
 * for use in a sequencer context
 *
 * @param aFrequency   {int}    the frequency of the note to synthesize in Hz
 * @param aPosition    {int}    the step position in the sequencer
 * @param aLength      {float} the length in steps the note lasts
 * @param aInstrument  {SynthInstrument} the instruments properties
 * @param aAutoCache   {bool} whether to start caching automatically
 */
SynthEvent::SynthEvent( float aFrequency, int aPosition, float aLength,
                        SynthInstrument *aInstrument, bool aAutoCache )
{
    init( aInstrument, aFrequency, aPosition, aLength, false, false );
    setAutoCache( aAutoCache );
}

/**
 * initializes an SynthEvent with very definitive properties to be precached
 * for use in a sequencer context
 *
 * @param aFrequency   {int}    the frequency of the note to synthesize in Hz
 * @param aPosition    {int}    the step position in the sequencer
 * @param aLength      {float} the length in steps the note lasts
 * @param aInstrument  {SynthInstrument} the instruments properties
 * @param aAutoCache   {bool} whether to start caching automatically
 * @param hasParent    {bool} whether to omit creation of OSC2 (blocks endless recursion)
 */
SynthEvent::SynthEvent( float aFrequency, int aPosition, float aLength,
                        SynthInstrument *aInstrument, bool aAutoCache, bool hasParent )
{
    init( aInstrument, aFrequency, aPosition, aLength, hasParent, false );
    setAutoCache( aAutoCache );
}

/**
 * initializes an SynthEvent to be synthesized live, for a live instrument context
 *
 * @param aFrequency  {int} the frequency of the note to synthesize in Hz
 * @param aInstrument {SynthInstrument}
 */
SynthEvent::SynthEvent( float aFrequency, SynthInstrument *aInstrument )
{
    init( aInstrument, aFrequency, 0, 1, false, true );
}

/**
 * initializes an SynthEvent to be synthesized live, for a live instrument context
 *
 * @param aFrequency  {int} the frequency of the note to synthesize in Hz
 * @param aInstrument {SynthInstrument}
 * @param hasParent    {bool} whether to omit creation of OSC2 (blocks endless recursion)
 */
SynthEvent::SynthEvent( float aFrequency, SynthInstrument *aInstrument, bool hasParent )
{
    init( aInstrument, aFrequency, 0, 1, hasParent, true );
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

    // secondary oscillator
    destroyOSC2();

    // remove AudioEvent from sequencer
    if ( liveSynthesis )
    {
        for ( int i; i < _instrument->liveEvents->size(); i++ )
        {
            if ( _instrument->liveEvents->at( i ) == this )
            {
                _instrument->liveEvents->erase( _instrument->liveEvents->begin() + i );
                break;
            }
        }
    }
    else
    {
        for ( int i; i < _instrument->audioEvents->size(); i++ )
        {
            if ( _instrument->audioEvents->at( i ) == this )
            {
                _instrument->audioEvents->erase( _instrument->audioEvents->begin() + i );
                break;
            }
        }
    }
    destroyLiveBuffer();
}

/* public methods */

AudioBuffer* SynthEvent::getBuffer()
{
    // if caching hasn't completed, fill cache fragment
    if ( !_cachingCompleted )
        doCache();

    return _buffer;
}

float SynthEvent::getFrequency()
{
    return _frequency;
}

void SynthEvent::setFrequency( float aFrequency )
{
    _phase         = 0.0f;
    _phaseIncr     = aFrequency / audio_engine::SAMPLE_RATE;
    _frequency     = aFrequency;
    _baseFrequency = aFrequency; // reference for arpeggiator / pitch shift modules

    if ( /*liveSynthesis &&*/ _type == WaveForms::KARPLUS_STRONG )
        initKarplusStrong();

    if ( _osc2 != 0 )
        _osc2->setFrequency( aFrequency );
}

/**
 * @param aPosition position in the sequencer where this event starts playing
 * @param aLength length (in sequencer steps) of this event
 * @param aInstrument the SynthInstrument whose properties will be used for synthesizing this event
 * @param aState which oscillator(s) to update 0 = all, 1 = oscillator 1, 2 = oscillator 2
 *               this is currently rudimentary as both oscillators are rendered and merged into one
 *               this is here for either legacy purposes or when performance can be gained
 */
void SynthEvent::updateProperties( int aPosition, float aLength, SynthInstrument *aInstrument, int aState )
{
    bool updateLFO1 = true;//( aState == 0 || aState == 1 );
    bool updateOSC2 = true;//( aState == 0 || aState == 2 );

    _type    = aInstrument->waveform;
    position = aPosition;
    length   = aLength;

    _attack  = aInstrument->attack;
    _release = aInstrument->release;

    //_rOsc    = aInstrument->rOsc->getLinkedOscillator();

    // secondary oscillator

    if ( updateOSC2 && aInstrument->osc2active )
        createOSC2( aPosition, aLength, aInstrument );
    else
        destroyOSC2();

    // modules

    applyModules( aInstrument );

    if ( updateLFO1 )
    {
        if ( _caching /*&& !_cachingCompleted */)
        {
            if ( _osc2 != 0 && _osc2->_caching )
                _osc2->_cancel = true;

            _cancel = true;
        }
        else {
            calculateBuffers();
        }
    }
}

void SynthEvent::unlock()
{
    _locked = false;

    if ( _updateAfterUnlock )
        calculateBuffers();

    _updateAfterUnlock = false;
}

void SynthEvent::calculateBuffers()
{
    if ( _locked )
    {
        _updateAfterUnlock = true;
        return;
    }
    if ( _caching )
        _cancel = true;

    int oldLength = _sampleLength;
    _sampleLength = ( int )( length * ( float ) bytes_per_tick );
    _sampleStart  = position * bytes_per_tick;
    _sampleEnd    = _sampleStart + _sampleLength;

    resetEnvelopes();

    // sample length changed (f.i. tempo change) or buffer not yet created ?
    // create buffer for (new) sample length
    if ( _sampleLength != oldLength || _buffer == 0 )
    {
        destroyBuffer(); // clear previous buffer contents

        // OSC2 generates no buffer (writes into parent buffer, saves memory)
        if ( !hasParent )
            _buffer = new AudioBuffer( audio_engine::OUTPUT_CHANNELS, _sampleLength );
     }

    if ( _type == WaveForms::KARPLUS_STRONG )
        initKarplusStrong();

    resetCache(); // yes here, not in cache()-invocation as cancels might otherwise remain permanent (see BulkCacher)

    // (re)cache (unless this event is OSC2 as only the parent event can invoke the render)
    if ( _autoCache && !hasParent )
    {
        if ( !_caching )
            cache( false );
        else
            _cancel = true;
    }
}

/**
 * synthesize is invoked by the Sequencer for rendering a live
 * SynthEvent into a single buffer
 *
 * aBufferLength {int} length of the buffer to synthesize
 */
AudioBuffer* SynthEvent::synthesize( int aBufferLength )
{
    if ( aBufferLength != audio_engine::BUFFER_SIZE )
    {
        // clear previous buffer contents
        destroyLiveBuffer();
        _liveBuffer = new AudioBuffer( audio_engine::OUTPUT_CHANNELS, aBufferLength );
    }
    render( _liveBuffer ); // overwrites old buffer contents

    // keep track of the rendered bytes, in case of a key up event
    // we still want to have the sound ring for the minimum period
    // defined in the constructor instead of cut off immediately

    if ( _queuedForDeletion && _minLength > 0 )
        _minLength -= aBufferLength;

    if ( _minLength <= 0 )
    {
        _hasMinLength = true;
        setDeletable( _queuedForDeletion );

        // event is about to be the deleted, apply a tiny fadeout
        if ( _queuedForDeletion )
        {
            int amt = ceil( aBufferLength / 4 );

            float envIncr = 1.0 / amt;
            float amp     = 1.0;

           for ( int i = aBufferLength - amt; i < aBufferLength; ++i )
           {
               for ( int c = 0, nc = _liveBuffer->amountOfChannels; c < nc; ++c )
               {
                   _liveBuffer->getBufferForChannel( c )[ i ] *= amp;
               }
               amp -= envIncr;
           }
        }
    }
    return _liveBuffer;
}

/* used for threading */

void *po( void* )
{

}

/**
 * (pre-)cache the contents of the SynthEvent in its entirety
 * this can be done in idle time to make optimum use of resources
 */
void SynthEvent::cache( bool doCallback )
{
    if ( _buffer == 0 ) // this cache request was invoked after destruction
        return;

    _caching = true;

    pthread_t t1;

    bool doThread  = false; // NO, seems dangerous (crash when rapidly re-triggering a recache) TODO check if true though ;)

    // we improve the use of the CPU resources
    // by creating a small local thread
    if ( doThread )
        pthread_create( &t1, NULL, &po, NULL );

    doCache();

    void* result;

    if ( doThread )
        pthread_join( t1, &result );

    // TODO: FUGLY! integrate callbacks more elegantly
    if ( doCallback )
        sequencer::bulkCacher->cacheQueue();
}

/* getters / setters */

float SynthEvent::getAttack()
{
    return _attack;
}

void SynthEvent::setAttack( float aValue )
{
    _attack = aValue;

    if ( _attack > 1 )
        _attack = 1;

    // no attack set ? WRONG! let's open a very minimal
    // one to prevent popping during sound start

    else if ( _attack == 0 )
        _attack = ( DEFAULT_FADE_DURATION / _sampleLength );

    attackIncr = 1 / ( _sampleLength * _attack );
    attackEnv  = 0.0;

    // update release envelope as it takes parameters from the attack envelope
    setRelease( _release );
}

int SynthEvent::getDecay()
{
    return ( int ) round( _decay / DECAY_MULTIPLIER );
}

void SynthEvent::setDecay( int aValue )
{
    if ( aValue == 0 )
        aValue = 105;

    _decay = round( aValue * DECAY_MULTIPLIER );

    // some waveforms ( sine, triangle ) can have popping occurring
    // at the end when the sample is cut off at an unfortunate point
    // we prevent this pop occurring by decreasing the decay a few
    // samples before the end of the current waveform

    int fadeLength = 4096;
    decayStart     = _sampleLength - fadeLength;
    decayIncr      = ( int ) round( _decay / fadeLength );
}

float SynthEvent::getRelease()
{
    return _release;
}

/*
 * release is calculated backwards from the total sample length, by
 * default we set the release at a few samples before the end to
 * prevent a pop occurring when audio suddenly stops / starts */

void SynthEvent::setRelease( float aValue )
{
    _release = aValue;

    // keep release in bounds
    if ( _release > 1.0 )
    {
        _release = 1.0;
    }
    // no release set ? WRONG! we create a very minimal one to prevent
    // "popping" during sound end by making sure all sound fades out
    else if ( _release <= 0 )
    {
        _release     = DEFAULT_FADE_DURATION / _sampleLength;
        releaseStart = _sampleLength;
    }
    releaseStart = ( int ) ( _sampleLength - ( _sampleLength * _release ));

    /*
     * if an attack envelope has been set, set the
     * release value to the attack envelope amount
     * for a gradual fade in and out */

    if ( _release > DEFAULT_FADE_DURATION && _attack > DEFAULT_FADE_DURATION )
    {
        _release     = _attack;
        releaseStart = ( int ) ( _sampleLength - ( _sampleLength * _release ));
        attackIncr   = 1.0 / ( float )( releaseStart );
    }
    float delta = _sampleLength - releaseStart;

    if ( delta > 0 )
        releaseIncr = 1.0 / delta;
    else
        releaseIncr = 0.0;

    releaseEnv = 1.0;
}

float SynthEvent::getVolume()
{
    return _volume;
}

void SynthEvent::setVolume( float aValue )
{
    _volume = aValue;
}

/* private methods */

void SynthEvent::initKarplusStrong()
{
    // reset previous _ringBuffer data
    int prevRingBufferSize = _ringBufferSize;
    _ringBufferSize        = ( int ) ( audio_engine::SAMPLE_RATE / _frequency );
    bool newSize           = _ringBufferSize != prevRingBufferSize;

    if ( _ringBuffer != 0 && newSize )
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

/**
 * the actual synthesizing of the audio
 *
 * @param aOutputBuffer {AudioBuffer*} the buffer to write into
 */
void SynthEvent::render( AudioBuffer* aOutputBuffer )
{
    int i;
    int bufferLength = aOutputBuffer->bufferSize;

    SAMPLE_TYPE amp = 0.0;
    SAMPLE_TYPE tmp, am, dpw, pmv;

    // following waveforms require alternate volume multipliers
    SAMPLE_TYPE sawAmp = liveSynthesis ? .7  : 1.0;
    SAMPLE_TYPE swAmp  = liveSynthesis ? /*.25*/ .005 : .005;

    bool applyRelease = _release > 0 && !liveSynthesis;

    int maxIndex = _sampleLength - 1;              // max index possible for this events length
    int cycleMax = _lastWriteIndex + bufferLength; // max buffer index to be written to in this cycle

    // keep in bounds
    if ( cycleMax > maxIndex )
        cycleMax = maxIndex;

    for ( i = _lastWriteIndex; i < cycleMax; ++i )
    {
        switch ( _type )
        {
            case WaveForms::SINE_WAVE:

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
                amp *= .7; // sines tend to distort easily when overlapping multi timbral parts

                break;

            case WaveForms::SAWTOOTH:

                // ---- Sawtooth
                amp = ( _phase < 0 ) ? _phase - ( int )( _phase - 1 ) : _phase - ( int )( _phase );
                amp *= sawAmp;

                break;

            case WaveForms::SQUARE_WAVE:

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
                amp *= swAmp; // these get loud
                break;

            case WaveForms::TRIANGLE:

                // ---- triangle
                if ( _phase < .5 )
                {
                    tmp = ( _phase * 4.0 - 1.0 );
                    amp = ( 1.0 - tmp * tmp ) * .75;
                }
                else {
                    tmp = ( _phase * 4.0 - 3.0 );
                    amp = ( tmp * tmp - 1.0 ) * .75;
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
                am = sin( pmv / 0x1000 ); // LFO -> AM

                // we multiply the amplitude as PWM results in a "quieter" wave
                amp *= ( am * 3 );
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

                // --- Karplus-Strong algorithm for plucked string-sound
                _ringBuffer->enqueue(( EnergyDecayFactor * (( _ringBuffer->dequeue() + _ringBuffer->peek()) / 2 )));
                amp = _ringBuffer->peek() * .7; // get the level down a bit, 'tis loud

                break;
        }
        // -- general wave operations:
        // envelopes
        if ( _attack > 0 )
        {
            if ( attackEnv < 1 )
            {
                attackEnv += attackIncr;
                amp       *= attackEnv;
            }
        }
        if ( applyRelease )
        {
            if ( i >= releaseStart )
            {
                releaseEnv -= releaseIncr;

                if ( releaseEnv < 0 )
                    releaseEnv = 0.0;

                amp *= releaseEnv;
            }
        }

        // --- _phase update operations
        if ( _type != WaveForms::PWM || ( liveSynthesis && _type != WaveForms::PWM ))
        {
            _phase += _phaseIncr;

            // restore _phase, max range is 0 - 1 ( float )
            if ( _phase > 1.0 )
                _phase -= 1.0;
        }
        if ( liveSynthesis )
            amp *= .5; // in case of multi-timbral fun

        // update modules
        if ( _arpeggiator != 0 )
        {
            if ( _arpeggiator->peek())
            {
                float baseFreq = _baseFrequency;
                setFrequency( _arpeggiator->getPitchForStep( _arpeggiator->getStep(), baseFreq ));
                _baseFrequency = baseFreq; // restore base freq for next arpeggiator step
            }
        }

        // stop caching when cancel is requested
        if ( _cancel )
            break;

        // -- write the output into the buffers channels
        for ( int c = 0, ca = aOutputBuffer->amountOfChannels; c < ca; ++c )
            aOutputBuffer->getBufferForChannel( c )[ i ] = amp * _volume * VOLUME_CORRECTION;
    }

    // secondary oscillator ? render its contents into this (parent) buffer

    if ( _osc2 != 0 && !_cancel )
    {
        // create a temporary buffer (this prevents writing to deleted buffers
        // when the parent event changes its _buffer properties (f.i. tempo change)
        int tempLength = liveSynthesis ? bufferLength : ( cycleMax - _lastWriteIndex );
        AudioBuffer* tempBuffer = new AudioBuffer( _buffer->amountOfChannels, tempLength );
        _osc2->render( tempBuffer );
        aOutputBuffer->mergeBuffers( tempBuffer, 0, _lastWriteIndex, 1.0f );

        delete tempBuffer; // free allocated memory
    }

    // update write index for next cycle
    _lastWriteIndex = ( !liveSynthesis && !hasParent ) ? i : 0;

    if ( !liveSynthesis )
    {
        _caching = false;

        // was a cancel requested ? re-cache to
        // match the new properties (cancel may only
        // be requested when changing properties!)
        if ( _cancel )
        {
            _cancel = false;
            calculateBuffers();
        }
        else
        {
            if ( i == maxIndex )
                _cachingCompleted = true;

            if ( _bulkCacheable )
                _autoCache = true;
        }
    }
}

void SynthEvent::setDeletable( bool value )
{
    // pre buffered event or rendered min length ? schedule for immediate deletion

    if ( !liveSynthesis || _hasMinLength )
        _deleteMe = value;
    else
        _queuedForDeletion = value;

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
 * @param aLiveSynthesis whether this event will be rendered live (synthesize) or can be pre cached
 */
void SynthEvent::init( SynthInstrument *aInstrument, float aFrequency, int aPosition,
                       int aLength, bool aHasParent, bool aLiveSynthesis )
{
    _instrument     = aInstrument;
    _buffer         = 0;
    _ringBuffer     = 0;
    _ringBufferSize = 0;
    _liveBuffer     = 0;
    _locked         = false;

    _frequency      = aFrequency;
    _baseFrequency  = aFrequency;
    position        = aPosition;
    length          = aLength;
    hasParent       = aHasParent;

    liveSynthesis          = aLiveSynthesis;
    _queuedForDeletion     = false;
    _deleteMe              = false;
    _cancel                = false; // whether we should cancel caching
    _caching               = false;
    _cachingCompleted      = false; // whether we're done caching
    _autoCache             = false; // we'll cache sequentially instead
    _type                  = aInstrument->waveform;
    _osc2                  = 0;
    //_rOsc                  = aInstrument->rOsc->getLinkedOscillator();
    _volume                = aInstrument->volume;
    _attack                = aInstrument->attack;
    _release               = aInstrument->release;
    _decay                 = 0.0;
    _sampleLength          = 0;
    _lastWriteIndex        = 0;

    // constants used by waveform generators

    PI                     = ( atan( 1 ) * 4 );
    TWO_PI                 = PI * 2;
    TWO_PI_OVER_SR         = TWO_PI / audio_engine::SAMPLE_RATE;
    pwr                    = PI / 1.05;
    pwAmp                  = 0.075;
    EnergyDecayFactor      = 0.990f; // TODO make this settable ?
    _pwmValue              = 0.0;
    _phase                 = 0.0;

    // secondary oscillator, note different constructor
    // to omit going into recursion!

    if ( !hasParent && aInstrument->osc2active )
       createOSC2( position, length, aInstrument );

    // modules

    _arpeggiator = 0;
    applyModules( aInstrument );

    if ( liveSynthesis )
    {
        // quick releases of the key should at least ring for a 32nd note
        _minLength    = bytes_per_bar / 32;
        _sampleLength = bytes_per_bar;                  // important for amplitude swell in
        _hasMinLength = false;                          // keeping track if the min length has been rendered

        setAttack   ( aInstrument->attack );
        //setRelease( aInstrument->release ); // no release envelopes for live synth!

        setFrequency( aFrequency );

        _liveBuffer = new AudioBuffer( audio_engine::OUTPUT_CHANNELS, audio_engine::BUFFER_SIZE );
    }
    else
    {
        _hasMinLength   = true; // a (pre-)cached event has no early cancel
        calculateBuffers();
    }

    // add the event to the sequencer so it can be heard
    // note that OSC2 contents aren't added to the sequencer
    // individually as their render is invoked by their parent,
    // writing directly into their parent buffer (saves memory overhead)

    if ( liveSynthesis )
    {
        if ( !hasParent )
            aInstrument->liveEvents->push_back( this );
    }
    else
    {
        if ( !hasParent )
            aInstrument->audioEvents->push_back( this );
    }
}

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
        // note no auto caching for non-live synthesized OSC2, its render is invoked by its parent (=this) event!
        if ( _osc2 == 0 )
        {
            if ( liveSynthesis )
                _osc2 = new SynthEvent( _frequency, aInstrument, true );
            else
                _osc2 = new SynthEvent( _frequency, aPosition, aLength, aInstrument, false, true );
        }
        // seems verbose, but in case of updating an existing OSC2, necessary
        _osc2->_type    = aInstrument->osc2waveform;
        _osc2->position = aPosition;
        _osc2->length   = aLength;
        _osc2->_attack  = _attack;
        _osc2->_release = _release;

        float lfo2Tmpfreq = _frequency + ( _frequency / 1200 * aInstrument->osc2detune );      // detune (1200 cents == octave)
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

        if ( _osc2->_caching /*&& !_osc2->_cachingCompleted */)
            _osc2->_cancel = true;
    }
}

void SynthEvent::destroyLiveBuffer()
{
    if ( _liveBuffer != 0 )
    {
        delete _liveBuffer;
        _liveBuffer = 0;
    }
}

void SynthEvent::destroyOSC2()
{
    if ( _osc2 != 0 )
    {
        if ( _osc2->_caching /*&& !_osc2->_cachingCompleted */)
            _osc2->_cancel = true;

        delete _osc2;
        _osc2 = 0;
    }
}

void SynthEvent::applyModules( SynthInstrument* instrument )
{
    if ( _arpeggiator != 0 )
    {
        delete _arpeggiator;
        _arpeggiator = 0;
    }

    if ( instrument->arpeggiatorActive )
        _arpeggiator = instrument->arpeggiator->clone();

    if ( _osc2 != 0 )
        _osc2->applyModules( instrument );
}

/**
 * render the event into the buffer and cache its
 * contents for the given bufferLength (cache buffer
 * fragments on demand, or all at once)
 */
void SynthEvent::doCache()
{
    bool wasLocked = _locked;

    // no lock required as the buffer can be read/written to simultaneously...
//    lock();

    render( _buffer );

//    if ( !wasLocked )
//    {
//        _updateAfterUnlock = false;
//        unlock();
//    }
}

void SynthEvent::resetCache()
{
    BaseCacheableAudioEvent::resetCache();
    resetEnvelopes();

    if ( _osc2 != 0 )
        _osc2->resetCache();
}

void SynthEvent::resetEnvelopes()
{
    releaseStart = _sampleLength;

    setDecay  ( 70 );
    setAttack ( _attack );
    setRelease( _release );
}
