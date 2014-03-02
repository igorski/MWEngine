#ifndef __SYNTHINSTRUMENT_H_INCLUDED__
#define __SYNTHINSTRUMENT_H_INCLUDED__

#include "audiochannel.h"
#include "processingchain.h"
#include "routeable_oscillator.h"
#include "baseaudioevent.h"
#include "basecacheableaudioevent.h"

class SynthInstrument
{
    public:
        SynthInstrument();
        ~SynthInstrument();

        int waveform;
        int octave;
        int keyboardOctave;
        float attack;
        float release;
        float volume;
        float keyboardVolume;
        RouteableOscillator *rOsc;
        ProcessingChain *processingChain;
        AudioChannel *audioChannel;

        bool osc2active;
        int osc2waveform;
        float osc2detune;
        int osc2octaveShift;
        int osc2fineShift;

        std::vector<BaseCacheableAudioEvent*>* audioEvents;
        std::vector<BaseAudioEvent*>* liveEvents;

    protected:
        void init();
};

#endif
