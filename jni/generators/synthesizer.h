/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2015 Igor Zinken - http://www.igorski.nl
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
#ifndef __SYNTHESIZER_H_INCLUDED__
#define __SYNTHESIZER_H_INCLUDED__

#include "../audiobuffer.h"
#include "../ringbuffer.h"
#include <events/basesynthevent.h>
#include <modules/arpeggiator.h>
#include <vector>

class SynthInstrument; // forward declaration, see <instruments/synthinstrument.h>

/**
 * example renderer for a synthesizer.
 * SynthInstrument describes the synthesizers properties
 * Synthesizer renders the properties into audio
 *
 * It is the same class used for the synthesizers in MikroWave and Kosm
 *
 * see: https://play.google.com/store/apps/details?id=nl.igorski.mikrowave&hl=en
 * see: https://play.google.com/store/apps/details?id=nl.igorski.kosm&hl=en
 */
class Synthesizer
{
    public:
        Synthesizer( SynthInstrument* aInstrument, int aOscillatorNum );
        ~Synthesizer();

        void render( AudioBuffer* aOutputBuffer, BaseSynthEvent* aEvent );
        void updateProperties();
        void initializeEventProperties( BaseSynthEvent* aEvent );

    protected:

        int _oscillatorNum;
        SynthInstrument* _instrument;

        // SYNTHESIS VARIABLES ----------------

        SAMPLE_TYPE TWO_PI_OVER_SR;

        int _fadeInDuration, _fadeOutDuration;
        float _pwr, _pwAmp, _pwmValue;                 // PWM-specific

        // Karplus-Strong specific
        RingBuffer* getRingBuffer( BaseSynthEvent* aEvent, float aFrequency );
        void initKarplusStrong( RingBuffer* ringBuffer ); // fill a ring buffer with noise (initial "pluck" of a string sound)

        // E.O. SYNTHESIS VARIABLES -----------

        // additional oscillators

        std::vector<Synthesizer*> _oscillators;
        bool hasParent;
        void createOscillator ( int aOscillatorNum );
        void destroyOscillator( int aOscillatorNum );
        float tuneOscillator  ( int aOscillatorNum, float aFrequency );

        // modules

        void applyModules( SynthInstrument* instrument );
};

#endif
