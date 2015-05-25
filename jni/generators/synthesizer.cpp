/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Igor Zinken - http://www.igorski.nl
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
#include "synthesizer.h"
#include "../global.h"
#include <definitions/waveforms.h>
#include <instruments/synthinstrument.h>
#include <utilities/bufferpool.h>
#include <utilities/bufferutility.h>
#include <utilities/utils.h>

/* constructors / destructor */

Synthesizer::Synthesizer( SynthInstrument* aInstrument, int aOscillatorNum )
{
    TWO_PI_OVER_SR = TWO_PI / ( SAMPLE_TYPE ) AudioEngineProps::SAMPLE_RATE;
    _pwr           = PI / 1.05;
    _pwAmp         = 0.075;
    _pwmValue      = 0.0;

    _instrument = aInstrument;

    _oscillatorNum = aOscillatorNum;
    hasParent      = aOscillatorNum > 0;

    // starting/stopping a waveform mid cycle can cause nasty pops, this is used for a smoother inaudible fade in
    _fadeInDuration  = BufferUtility::millisecondsToBuffer( 20, AudioEngineProps::SAMPLE_RATE );
    _fadeOutDuration = BufferUtility::millisecondsToBuffer( 30, AudioEngineProps::SAMPLE_RATE );
}

Synthesizer::~Synthesizer()
{
    for ( int i = 0; i < _oscillators.size(); ++i )
        destroyOscillator( i );
}

/* public methods */

void Synthesizer::render( AudioBuffer* aOutputBuffer, BaseSynthEvent* aEvent )
{
    int bufferLength = aOutputBuffer->bufferSize;
    int type         = _instrument->getTuningForOscillator( _oscillatorNum )->waveform;

    if ( !hasParent ) aOutputBuffer->silenceBuffers(); // unset previous buffer contents
    SAMPLE_TYPE amp = 0.0;
    SAMPLE_TYPE tmp, dpw, pmv;

    bool doAddOSCs = !hasParent && _instrument->getOscillatorAmount() > 1;

    // attenuate oscillators
    SAMPLE_TYPE volume = aEvent->getVolume() / ( SAMPLE_TYPE ) _instrument->getOscillatorAmount();

    int renderStartOffset = 0;
    int maxSampleIndex    = aEvent->getSampleLength() - 1;    // max index possible for the events length
    int renderEndOffset   = renderStartOffset + bufferLength; // max buffer index to be written to in this cycle

    // keep in bounds of event duration
    if ( renderEndOffset > maxSampleIndex )
        renderEndOffset = maxSampleIndex;

    // cache event properties for this render cycle

    SAMPLE_TYPE phase         = aEvent->getPhaseForOscillator( _oscillatorNum );
    SAMPLE_TYPE frequency     = aEvent->getFrequency();
    SAMPLE_TYPE baseFrequency = aEvent->getBaseFrequency();
    bool isSequenced          = aEvent->isSequenced;
    int bufferWriteIndex      = aEvent->lastWriteIndex;

    // modules

    bool doArpeggiator       = _instrument->arpeggiatorActive;
    Arpeggiator* arpeggiator = _instrument->arpeggiator;

    if ( doArpeggiator )
    {
        arpeggiator->setBufferPosition( aEvent->cachedProps.arpeggioPosition );
        arpeggiator->setStep          ( aEvent->cachedProps.arpeggioStep );

        frequency = arpeggiator->getPitchForStep( aEvent->cachedProps.arpeggioStep,
                                                  !hasParent ? baseFrequency : frequency );
        aEvent->setFrequency( frequency, false );
    }
    // envelopes

    _instrument->adsr->setBufferLength( aEvent->getSampleLength() );

    // Karplus-Strong specific

    RingBuffer* ringBuffer;
    if ( type == WaveForms::KARPLUS_STRONG ) ringBuffer = getRingBuffer( aEvent, frequency );

    for ( int i = renderStartOffset; i < renderEndOffset; ++i )
    {
        switch ( type )
        {
            case WaveForms::SINE:

                // ---- Sine wave

                if ( phase < .5 )
                {
                    tmp = ( phase * 4.0 - 1.0 );
                    amp = ( 1.0 - tmp * tmp );
                }
                else {
                    tmp = ( phase * 4.0 - 3.0 );
                    amp = ( tmp * tmp - 1.0 );
                }

                // sines need some extra love :
                // for one: a fade-in and fade-out at the start to prevent POP!ping...

                if ( isSequenced && !hasParent ) // unless this isn't the main oscillator
                {
                    int curWritePos = bufferWriteIndex + i;

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
                amp = ( phase < 0 ) ? phase - ( int )( phase - 1 ) : phase - ( int )( phase );

                break;

            case WaveForms::SQUARE:

                // ---- Square wave
                if ( phase < .5 )
                {
                    tmp = TWO_PI * ( phase * 4.0 - 1.0 );
                    amp = ( 1.0 - tmp * tmp );
                }
                else {
                    tmp = TWO_PI * ( phase * 4.0 - 3.0 );
                    amp = ( tmp * tmp - 1.0 );
                }
                amp *= .01; // these get loud !
                break;

            case WaveForms::TRIANGLE:

                // ---- triangle
                if ( phase < .5 )
                {
                    tmp = ( phase * 4.0 - 1.0 );
                    amp = ( 1.0 - tmp * tmp );
                }
                else {
                    tmp = ( phase * 4.0 - 3.0 );
                    amp = ( tmp * tmp - 1.0 );
                }
                // the actual triangulation function
                amp = amp < 0 ? -amp : amp;

                break;

            case WaveForms::PWM:

                // --- pulse width modulation
                pmv = i + ( ++_pwmValue ); // i + event position

                dpw = sin( pmv / 0x4800 ) * _pwr; // LFO -> PW
                amp = phase < PI - dpw ? _pwAmp : -_pwAmp;

                // PWM has its own phase update operation
                phase = phase + ( TWO_PI_OVER_SR * frequency );
                phase = phase > TWO_PI ? phase - TWO_PI : phase;

                // we multiply the amplitude as PWM results in a "quieter" wave
                amp *= 4;

                break;

            case WaveForms::NOISE:

                // --- noise
                if ( phase < .5 )
                {
                    tmp = ( phase * 4.0 - 1.0 );
                    amp = ( 1.0 - tmp * tmp );
                }
                else {
                    tmp = ( phase * 4.0 - 3.0 );
                    amp = ( tmp * tmp - 1.0 );
                }
                // above we calculated pitch, now we add some
                // randomization to the signal for the actual noise
                amp *= randomFloat();
                break;

            case WaveForms::KARPLUS_STRONG:

                // --- Karplus-Strong algorithm for plucked string-sound (0.990f being energy decay factor)
                ringBuffer->enqueue(( 0.990f * (( ringBuffer->dequeue() + ringBuffer->peek() ) / 2 ) ));
                amp = ringBuffer->peek();

                break;
        }

        // --- phase update operations
        if ( type != WaveForms::PWM && type != WaveForms::KARPLUS_STRONG )
        {
            phase += aEvent->cachedProps.phaseIncr;

            // keep phase within range (-MAX_PHASE to +MAX_PHASE)
            if ( phase > MAX_PHASE )
                phase -= MAX_PHASE;
        }

        // update modules
        if ( doArpeggiator )
        {
            // step the arpeggiator to the next position
            if ( arpeggiator->peek() )
            {
                frequency = arpeggiator->getPitchForStep( arpeggiator->getStep(), baseFrequency );
                aEvent->setFrequency( frequency, false );
                initializeEventProperties( aEvent, true ); // force update of ring buffers where applicable
                if ( type == WaveForms::KARPLUS_STRONG ) ringBuffer = getRingBuffer( aEvent, frequency );
            }
        }

        // events frequency updated from outside (e.g. user shifted event note ?)
        // update the cached frequency

        if ( aEvent->getFrequency() != frequency )
            frequency = aEvent->getFrequency();

        // -- write the output into the buffers channels

        for ( int c = 0, ca = aOutputBuffer->amountOfChannels; c < ca; ++c )
            aOutputBuffer->getBufferForChannel( c )[ i ] += ( amp * volume );
    }

    // additional oscillators ? render their contents into the output buffer

    if ( doAddOSCs )
    {
        float eventFreq = aEvent->getFrequency(); // store current freq

        for ( int o = 0, oa = _oscillators.size(); o < oa; ++o )
        {
            // temporarily tune event to oscillator tuning (use base frequency)
            // note we add 1 to the oscillator number (parent oscillator == 0)

            aEvent->setFrequency( tuneOscillator( o + 1, baseFrequency ), false );
            _oscillators.at( o )->render( aOutputBuffer, aEvent );
        }
        aEvent->setFrequency( eventFreq, false ); // restore freq for original oscillator
    }

    // apply ADSR envelopes and update cacheWriteIndex for next render cycle

    if ( !hasParent )
    {
        _instrument->adsr->setLastEnvelope( aEvent->cachedProps.ADSRenvelope );
        _instrument->adsr->setBufferLength( aEvent->getSampleLength() );
        aEvent->cachedProps.ADSRenvelope = _instrument->adsr->apply( aOutputBuffer, bufferWriteIndex );

        aEvent->cachedProps.arpeggioPosition = arpeggiator->getBufferPosition();
        aEvent->cachedProps.arpeggioStep     = arpeggiator->getStep();

        aEvent->lastWriteIndex = bufferWriteIndex + renderEndOffset;
    }

    // commit the updated event properties

    aEvent->setPhaseForOscillator( _oscillatorNum, phase );
}

void Synthesizer::updateProperties()
{
    // update the state of the additional oscillators
    bool addOSC = !hasParent && _instrument->getOscillatorAmount() > 1;

    if ( addOSC )
    {
        // first oscillator is this Synthesizer instance
        for ( int i = 1; i < _instrument->getOscillatorAmount(); ++i )
            createOscillator( i );
    }
}

void Synthesizer::initializeEventProperties( BaseSynthEvent* aEvent, bool initializeBuffers )
{
    if ( !initializeBuffers )
        return;

    for ( int i = 0, l = _instrument->getOscillatorAmount(); i < l; ++i )
    {
        // in case of Karplus Strong synthesis ensure the ring buffers
        // are filled with noise (this caters for the "pluck" of the sound)

        if ( _instrument->getTuningForOscillator( i )->waveform == WaveForms::KARPLUS_STRONG )
        {
            SAMPLE_TYPE frequency  = tuneOscillator( i, aEvent->getFrequency() );
            RingBuffer* ringBuffer = getRingBuffer( aEvent, frequency );
            initKarplusStrong( ringBuffer );
        }
    }
}

/* protected methods */

/**
 * creates a new/updates an existing additional oscillator
 *
 * @param aOscillatorNum index for the additional oscillator
 * @param aInstrument the synth instrument whose properties are used for synthesis
 */
void Synthesizer::createOscillator( int aOscillatorNum )
{
    if ( _instrument->getOscillatorAmount() > 1 &&
         _oscillators.size() < ( _instrument->getOscillatorAmount() - 1 ))
    {
        _oscillators.push_back( new Synthesizer( _instrument, aOscillatorNum ));
    }
}

void Synthesizer::destroyOscillator( int aOscillatorNum )
{
    if ( _oscillators.size() < aOscillatorNum )
        return;

    Synthesizer* osc = _oscillators.at( aOscillatorNum );
    _oscillators.erase( _oscillators.begin() + aOscillatorNum );

    if ( osc != 0 )
        delete osc;
}

float Synthesizer::tuneOscillator( int aOscillatorNum, float aFrequency )
{
    OscillatorTuning* tuning = _instrument->getTuningForOscillator( aOscillatorNum );

    float lfo2Tmpfreq = aFrequency + ( aFrequency / 1200 * tuning->detune ); // 1200 cents == octave
    float lfo2freq    = lfo2Tmpfreq;

    // octave shift ( -2 to +2 )
    if ( tuning->octaveShift != 0 )
    {
        if ( tuning->octaveShift < 0 )
            lfo2freq = lfo2Tmpfreq / std::abs(( float ) ( tuning->octaveShift * 2 ));
        else
            lfo2freq += ( lfo2Tmpfreq * std::abs(( float ) ( tuning->octaveShift * 2 ) - 1 ));
    }

    // fine shift ( -7 to +7 )
    float fineShift = ( lfo2Tmpfreq / 12 * std::abs( tuning->fineShift ));

    if ( tuning->fineShift < 0 )
        lfo2freq -= fineShift;
     else
        lfo2freq += fineShift;

    return lfo2freq;
}

RingBuffer* Synthesizer::getRingBuffer( BaseSynthEvent* aEvent, float aFrequency )
{
    return BufferPool::getRingBufferForEvent( aEvent, aFrequency );
}

void Synthesizer::initKarplusStrong( RingBuffer* ringBuffer )
{
    ringBuffer->flush();

    // fill the ring buffer with noise (the initial "pluck" of the string)
    for ( int i = 0, l = ringBuffer->getBufferLength(); i < l; ++i )
        ringBuffer->enqueue( randomFloat() );
}
