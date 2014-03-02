#ifndef __DRUMINSTRUMENT_H_INCLUDED__
#define __DRUMINSTRUMENT_H_INCLUDED__

#include "audiochannel.h"
#include "processingchain.h"
#include "routeable_oscillator.h"

class DrumInstrument
{
    public:
        DrumInstrument();
        ~DrumInstrument();
        float volume;
        int drumTimbre;

        ProcessingChain *processingChain;
        RouteableOscillator *rOsc;
        AudioChannel *audioChannel;
};

#endif
