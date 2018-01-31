/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2016 Igor Zinken - http://www.igorski.nl
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
#include "synthinstrument.h"
#include "../global.h"
#include "../sequencer.h"
#include <definitions/waveforms.h>
#include <events/basesynthevent.h>
#include <utilities/utils.h>
#include <cstddef>

/* constructor / destructor */

SynthInstrument::SynthInstrument()
{
    construct();
    init();
}

SynthInstrument::~SynthInstrument()
{
    delete adsr;
    delete rOsc;
    delete arpeggiator;
    delete synthesizer;

    // when using JNI, we let SWIG invoke destructors when Java references are finalized
    // otherwise we delete and dispose the events directly from this instrument
#ifndef USE_JNI

    while ( !_audioEvents->empty() )
    {
        delete _audioEvents->back();
        _audioEvents->pop_back();
    }

    while ( !_liveAudioEvents->empty() )
    {
        delete _liveAudioEvents->back();
        _liveAudioEvents->pop_back();
    }

#endif

    while ( oscillators.size() > 0 )
        delete oscillators.back();
}

/* public methods */

void SynthInstrument::updateEvents()
{
    // SynthEvents are mapped to position relative to the measure's subdivisions
    // as such we don't require to invoke the BaseInstrument::updateEvents() method
    // to resync the offsets on a tempo change

    for ( int i = 0, l = _audioEvents->size(); i < l; ++i )
    {
        BaseSynthEvent* event = ( BaseSynthEvent* ) ( _audioEvents->at( i ) );
        event->invalidateProperties( event->position, event->length, this );
    }
    for ( int i = 0, l = _liveAudioEvents->size(); i < l; ++i )
    {
        BaseSynthEvent* event = ( BaseSynthEvent* ) ( _liveAudioEvents->at( i ) );
        event->invalidateProperties( event->position, event->length, this );
    }
    synthesizer->updateProperties();
}

bool SynthInstrument::removeEvent( BaseAudioEvent* audioEvent, bool isLiveEvent )
{
    bool removed = BaseInstrument::removeEvent( audioEvent, isLiveEvent );

    // when using JNI, we let SWIG invoke destructors when Java references are finalized
    // otherwise we delete and dispose the events directly from this instrument
#ifndef USE_JNI
    delete audioEvent;
    audioEvent = 0;
#endif
    return removed;
}

int SynthInstrument::getOscillatorAmount()
{
    return oscAmount;
}

void SynthInstrument::setOscillatorAmount( int aAmount )
{
    oscAmount = aAmount;

    if ( oscillators.size() < oscAmount )
        reserveOscillators( oscAmount );

    synthesizer->updateProperties();
}

void SynthInstrument::reserveOscillators( int aAmount )
{
    if ( oscillators.size() < aAmount )
    {
        while ( oscillators.size() < aAmount )
            oscillators.push_back( new OscillatorProperties( WaveForms::SINE, 0.0, 0, 0 ) );
    }
    else {
        while ( oscillators.size() > aAmount ) {
            delete oscillators.back();
            oscillators.pop_back();
        }
    }
}

OscillatorProperties* SynthInstrument::getOscillatorProperties( int aOscillatorNum )
{
    return oscillators.at( aOscillatorNum );
}

/* protected methods */

void SynthInstrument::init()
{
    adsr = new ADSR();
    adsr->setAttack( 0.01 );
    adsr->setDecay ( 0.01 );

    // default values
    octave          = 4;
    keyboardOctave  = 4;
    keyboardVolume  = 0.5;

    // modules

    rOsc              = new RouteableOscillator();
    audioChannel      = new AudioChannel( 0.8 );
    synthesizer       = new Synthesizer( this, 0 );
    arpeggiator       = new Arpeggiator();
    arpeggiatorActive = false;

    // start out with a single oscillator

    setOscillatorAmount( 1 );
}
