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
#ifndef __PROCESSINGCHAIN_H_INCLUDED__
#define __PROCESSINGCHAIN_H_INCLUDED__

#include <vector>
#include "baseprocessor.h"
#include "basebusprocessor.h"
#include "bitcrusher.h"
#include "decimator.h"
#include "delay.h"
#include "fm.h"
#include "formant_filter.h"
#include "filter.h"
#include "lpfhpfilter.h"
#include "phaser.h"
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
