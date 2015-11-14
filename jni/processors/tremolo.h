/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Igor Zinken - http://www.igorski.nl
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
#ifndef __TREMOLO_H_INCLUDED__
#define __TREMOLO_H_INCLUDED__

#include "baseprocessor.h"
#include "../audiobuffer.h"
#include "../wavetable.h"
#include <events/sampleevent.h>

class Tremolo : public BaseProcessor
{
    public:

        // Tremolo can work with two distinct channels, if the left or right channel
        // have a different wave form or frequency, Tremolo functions as a stereo effect

        Tremolo( int aLeftWaveForm, int aRightWaveForm, float aLeftFrequency, float aRightFrequency );
        ~Tremolo();

        // aChannelNum 0 = left channel table, aChannelNum 1 = right channel table

        int  getWaveFormForChannel   ( int aChannelNum );
        void setWaveFormForChannel   ( int aChannelNum, int aWaveForm );
        float getFrequencyForChannel ( int aChannelNum );
        void  setFrequencyForChannel ( int aChannelNum, float aFrequency );
        WaveTable* getTableForChannel( int aChannelNum );

        bool isStereo();
        void process( AudioBuffer* sampleBuffer, bool isMonoSource );

    protected:
        WaveTable* _tables[ 2 ];
        int        _waveforms[ 2 ];
};

#endif
