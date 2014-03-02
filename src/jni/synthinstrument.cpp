#include "synthinstrument.h"
#include "global.h"
#include "sequencer.h"
#include "utils.h"
#include <cstddef>

// constructor

SynthInstrument::SynthInstrument()
{
    init();

    sequencer::synthesizers->push_back( this );  // register instrument inside the sequencer
}

SynthInstrument::~SynthInstrument()
{
    DebugTool::log( "SynthInstrument::DESTRUCT" );

    delete rOsc;
    delete audioChannel;
    delete processingChain;
    delete audioEvents;
    delete liveEvents;

   for ( int i; i < sequencer::synthesizers->size(); i++ )
   {
        if ( sequencer::synthesizers->at( i ) == this )
        {
            sequencer::synthesizers->erase( sequencer::synthesizers->begin() + i );
            break;
        }
    }
}

/* protected methods */

void SynthInstrument::init()
{
    // default values
    octave          = 4;
    keyboardOctave  = 4;
    attack          = 0.01;
    release         = 0.01;
    volume          = 0.8;
    keyboardVolume  = 0.5;
    waveform        = WaveForms::SAWTOOTH;

    osc2active      = false;
    osc2waveform    = WaveForms::SINE_WAVE;
    osc2octaveShift = 0;
    osc2fineShift   = 0;
    osc2detune      = 0.0;

    rOsc            = new RouteableOscillator();
    processingChain = new ProcessingChain();
    audioChannel    = new AudioChannel( processingChain, this->volume );

    audioEvents = new std::vector<BaseCacheableAudioEvent*>();
    liveEvents  = new std::vector<BaseAudioEvent*>();
}
