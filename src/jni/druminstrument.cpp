#include "druminstrument.h"
#include "global.h"
#include "sequencer.h"
#include "utils.h"
#include <cstddef>

/* constructor / destructor */

DrumInstrument::DrumInstrument()
{
    rOsc = 0;//rOsc            = new RouteableOscillator();  // currently unused...
    processingChain = new ProcessingChain();
    audioChannel    = new AudioChannel( processingChain, this->volume, bytes_per_bar );

    volume    = .5;
    drumTimbre = DrumSynthTimbres::LIGHT;

    sequencer::drummachine = this;  // register instrument inside the sequencer
}

DrumInstrument::~DrumInstrument()
{
    DebugTool::log( "DrumInstrument::DESTRUCT" );

    sequencer::drummachine = 0;

    delete rOsc;
    delete audioChannel;
    delete processingChain;
}
