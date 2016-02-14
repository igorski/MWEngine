/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2016 Igor Zinken - http://www.igorski.nl
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
#ifndef __SYNTHINSTRUMENT_H_INCLUDED__
#define __SYNTHINSTRUMENT_H_INCLUDED__

#include "baseinstrument.h"
#include "../audiochannel.h"
#include <instruments/oscillatorproperties.h>
#include <events/baseaudioevent.h>
#include <generators/synthesizer.h>
#include <modules/adsr.h>
#include <modules/arpeggiator.h>
#include <modules/routeableoscillator.h>

class SynthInstrument : public BaseInstrument
{
    public:
        SynthInstrument();
        ~SynthInstrument();

        int octave;
        int keyboardOctave;
        float keyboardVolume;

        Synthesizer* synthesizer;

        // amount of oscillators

        int getOscillatorAmount ();
        void setOscillatorAmount( int aAmount );
        void reserveOscillators ( int aAmount );
        OscillatorProperties* getOscillatorProperties( int aOscillatorNum );

        // modules

        Arpeggiator* arpeggiator;
        bool arpeggiatorActive;

        RouteableOscillator *rOsc;
        ADSR* adsr;

        // overrides

        void updateEvents();  // updates all events after changing synth properties
        bool removeEvent( BaseAudioEvent* audioEvent, bool isLiveEvent );

    protected:

        int oscAmount;      // amount of oscillators, minimum == 1
        std::vector<OscillatorProperties*> oscillators;

        void init();
};

#endif
