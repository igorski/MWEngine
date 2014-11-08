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
#include "../utils.h"
#include <events/basesynthevent.h>
#include <cstddef>

// constructor

SynthInstrument::SynthInstrument()
{
    init();

    registerInSequencer(); // registers instrument inside the sequencer
}

SynthInstrument::~SynthInstrument()
{
    DebugTool::log( "SynthInstrument::DESTRUCT" );

    delete adsr;
    delete rOsc;
    delete audioEvents;
    delete liveAudioEvents;
}

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
    delete audioEvent;
    audioEvent = 0;
    return true;
#endif
    return false;
}

/* protected methods */

void SynthInstrument::init()
{
    adsr = new ADSR();
    adsr->setAttack ( 0.01 );
    adsr->setDecay  ( 0.01 );

    // default values
    octave          = 4;
    keyboardOctave  = 4;
    volume          = 0.8;
    keyboardVolume  = 0.5;
    waveform        = WaveForms::SAWTOOTH;

    osc2active      = false;
    osc2waveform    = WaveForms::SINE;
    osc2octaveShift = 0;
    osc2fineShift   = 0;
    osc2detune      = 0.0;

    rOsc            = new RouteableOscillator();
    audioChannel    = new AudioChannel( volume );

    audioEvents       = new std::vector<BaseAudioEvent*>();
    liveAudioEvents   = new std::vector<BaseAudioEvent*>();

    arpeggiator       = new Arpeggiator();
    arpeggiatorActive = false;
}
