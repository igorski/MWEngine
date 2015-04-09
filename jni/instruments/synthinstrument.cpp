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
    init();

    registerInSequencer(); // registers instrument inside the sequencer
}

SynthInstrument::~SynthInstrument()
{
    delete adsr;
    delete rOsc;
    delete arpeggiator;
    delete synthesizer;

    while ( audioEvents->size() > 0 )
        delete audioEvents->back();

    while ( liveAudioEvents->size() > 0 )
        delete liveAudioEvents->back();

    while ( oscillators.size() > 0 )
        delete oscillators.back();

    delete audioEvents;
    delete liveAudioEvents;
}

/* public methods */

bool SynthInstrument::hasEvents()
{
    return audioEvents->size() > 0;
}

bool SynthInstrument::hasLiveEvents()
{
    return liveAudioEvents->size() > 0;
}

std::vector<BaseAudioEvent*>* SynthInstrument::getEvents()
{
    return audioEvents;
}

std::vector<BaseAudioEvent*>* SynthInstrument::getLiveEvents()
{
    return liveAudioEvents;
}

void SynthInstrument::updateEvents()
{
    for ( int i = 0, l = audioEvents->size(); i < l; ++i )
    {
        BaseSynthEvent* event = ( BaseSynthEvent* ) ( audioEvents->at( i ) );
        event->invalidateProperties( event->position, event->length, this );
    }
    for ( int i = 0, l = liveAudioEvents->size(); i < l; ++i )
    {
        BaseSynthEvent* event = ( BaseSynthEvent* ) ( liveAudioEvents->at( i ) );
        event->invalidateProperties( event->position, event->length, this );
    }
    synthesizer->updateProperties();
}

void SynthInstrument::clearEvents()
{
    if ( audioEvents != 0 )
        audioEvents->clear();

    if ( liveAudioEvents != 0 )
        liveAudioEvents->clear();
}

bool SynthInstrument::removeEvent( BaseAudioEvent* aEvent )
{
    // when using JNI, we let SWIG invoke destructors when Java references are finalized
    // otherwise we delete and dispose the events directly from this instrument
#ifndef USE_JNI
    delete aEvent;
    aEvent = 0;
    return true;
#endif
    return false;
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
            oscillators.push_back( new OscillatorTuning( WaveForms::SINE, 0.0, 0, 0 ) );
    }
    else {
        while ( oscillators.size() > aAmount ) {
            delete oscillators.back();
            oscillators.pop_back();
        }
    }
}

OscillatorTuning* SynthInstrument::getTuningForOscillator( int aOscillatorNum )
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
    volume          = 0.8;
    keyboardVolume  = 0.5;

    // modules

    rOsc              = new RouteableOscillator();
    audioChannel      = new AudioChannel( volume );
    synthesizer       = new Synthesizer( this, 0 );
    arpeggiator       = new Arpeggiator();
    arpeggiatorActive = false;

    // start out with a single oscillator

    setOscillatorAmount( 1 );

    // events

    audioEvents       = new std::vector<BaseAudioEvent*>();
    liveAudioEvents   = new std::vector<BaseAudioEvent*>();
}
