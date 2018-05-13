/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Igor Zinken - http://www.igorski.nl
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
#ifndef __SAMPLE_UTIL_H_INCLUDED__
#define __SAMPLE_UTIL_H_INCLUDED__

#include "../events/sampleevent.h"

class SampleUtility
{
    public:
        /**
         * convenience method to adjust given sampleEvents playback rate
         * to play at the pitch difference implied by given semitones
         * (can be both positive and negative)
         */
        inline static void pitchShift( SampleEvent* sampleEvent, float semitones )
        {
            float rate;

            if ( semitones > 0.f )
                rate = ( float ) pow( 1.05946f, semitones );  // shift up
            else
                rate = ( float ) pow( 0.94387f, -semitones ); // shift down

            unsigned int sampleRate = sampleEvent->getSampleRate();

            if ( sampleRate == AudioEngineProps::SAMPLE_RATE ) {
                sampleEvent->setPlaybackRate( rate );
            }
            else {

                // SampleEvent can reference an AudioBuffer that has a sample rate
                // different to the engine, adjust the rate to make up for this difference as well

                sampleEvent->setPlaybackRate( rate / AudioEngineProps::SAMPLE_RATE * ( float ) sampleRate );
            }
        }
};

#endif
