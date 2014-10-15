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

    sequencer::synthesizers.push_back( this );  // register instrument inside the sequencer
}

SynthInstrument::~SynthInstrument()
{
    DebugTool::log( "SynthInstrument::DESTRUCT" );

    delete adsr;
    delete rOsc;
    delete audioEvents;
    delete liveEvents;

   for ( int i; i < sequencer::synthesizers.size(); i++ )
   {
        if ( sequencer::synthesizers.at( i ) == this )
        {
            sequencer::synthesizers.erase( sequencer::synthesizers.begin() + i );
            break;
        }
    }
}

bool SynthInstrument::hasEvents()
{
    return audioEvents->size() > 0 || liveEvents->size() > 0;
}

void SynthInstrument::updateEvents()
{
    for ( int i = 0, l = audioEvents->size(); i < l; ++i )
    {
        BaseSynthEvent* event = ( BaseSynthEvent* ) ( audioEvents->at( i ) );
        event->invalidateProperties( event->position, event->length, this );
    }
    for ( int i = 0, l = liveEvents->size(); i < l; ++i )
    {
        BaseSynthEvent* event = ( BaseSynthEvent* ) ( liveEvents->at( i ) );
        event->invalidateProperties( event->position, event->length, this );
    }
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
    audioChannel    = new AudioChannel( this->volume );

    audioEvents = new std::vector<BaseCacheableAudioEvent*>();
    liveEvents  = new std::vector<BaseAudioEvent*>();

    arpeggiator       = new Arpeggiator();
    arpeggiatorActive = false;
}
