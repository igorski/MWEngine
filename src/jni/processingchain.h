#ifndef __PROCESSINGCHAIN_H_INCLUDED__
#define __PROCESSINGCHAIN_H_INCLUDED__

#include <vector>
#include "baseprocessor.h"
#include "basebusprocessor.h"
#include "bitcrusher.h"
#include "compressor.h"
#include "decimator.h"
#include "delay.h"
#include "fm.h"
#include "formant_filter.h"
#include "filter.h"
#include "lpfhpfilter.h"
#include "phaser.h"
#include "pitchshifter.h"
#include "waveshaper.h"

class ProcessingChain
{
    public:
        ProcessingChain();
        ~ProcessingChain();

        bool fmActive;
        FrequencyModulator *fm;

        /* filter properties */

        float filterCutoff;
        float filterResonance;
        bool filterActive;
        Filter *filter;

        /* formant filter properties */

        float filterFormant;
        bool formantActive;
        FormantFilter *formant;

        /* phaser properties */

        float phaserRate;
        float phaserFeedback;
        float phaserDepth;
        bool phaserActive;
        Phaser *phaser;

        /* distortion properties */

        float distortion;
        float distortionLevel;
        bool waveshaperActive;
        WaveShaper *waveShaper;

        bool bitCrusherActive;
        BitCrusher *bitCrusher;

        float decimatorDistortion;
        float decimatorDistortionLevel;
        bool decimatorActive;
        Decimator *decimator;

        /* delay properties */

        float delayTime;
        float delayMix;
        float delayFeedback;
        bool delayActive;
        Delay *delay;

        /* compressor properties */

        float cAttack;
        float cRelease;
        float cThreshold;
        float cGain;
        float cRatio;
        bool compressorActive;
        Compressor *compressor;

        /* pitchshifter properties */

        PitchShifter *pitchShifter;

        /* LPF / HPF filter properties */

        float lpfCutoff;
        float hpfCutoff;
        bool lpfHpfActive;
        LPFHPFilter *lpfhpf;

        std::vector<BaseProcessor*>   getActiveProcessors();
        std::vector<BaseBusProcessor*> getActiveBusProcessors();

        void cacheActiveProcessors();
        void reset();

    private:

        /* cached chains */
       std::vector<BaseProcessor*>    _activeProcessors;
       std::vector<BaseBusProcessor*> _activeBusProcessors;
};

#endif
