/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Igor Zinken - http://www.igorski.nl
 *
 * adapted from source by Matt @ hackmeopen.com
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
#include "wavegenerator.h"
#include "../global.h"
#include <math.h>
#include <cmath>

namespace WaveGenerator
{
    void generate( WaveTable* waveTable, int waveformType )
    {
        SAMPLE_TYPE* outputBuffer = waveTable->getBuffer();
        int numberOfSamples       = waveTable->tableLength;

        int tempValue, partials;
        SAMPLE_TYPE frequency, gibbs, sample, tmp;
        SAMPLE_TYPE power, baseFrequency = 440, nyquist = ( SAMPLE_TYPE ) AudioEngineProps::SAMPLE_RATE / 2.0;

        // every other MIDI note (127 values)
        for ( int i = -69; i <= 58; i += 2 )
        {
            power       = i / 12.0;
            tempValue   = baseFrequency * pow( 2, power );
            frequency   = round( tempValue );
            partials    = nyquist / frequency;
            
            for ( int t = 0; t < numberOfSamples; t++ )
            {
                sample = 0, tmp = 0;
                
                // summing of the Fourier series for harmonics up to half the sample rate (Nyquist frequency)
                for ( int s = 1; s <= partials; ++s )
                {
                    // smoothing of sharp transitions
                    gibbs  = cos(( SAMPLE_TYPE )( s - 1.0 ) * PI / ( 2.0 * ( SAMPLE_TYPE ) partials ));
                    gibbs *= gibbs;
                    
                    // generation of the waveform
                    switch ( waveformType )
                    {
                        case WaveForms::SINE:
                            sample += gibbs * sin(( SAMPLE_TYPE ) s * TWO_PI * ( SAMPLE_TYPE ) t / numberOfSamples );
                            tmp     = sample;
                            break;

                        case WaveForms::TRIANGLE:
                            sample += sin(( SAMPLE_TYPE ) s * TWO_PI * ( SAMPLE_TYPE ) t / numberOfSamples );
                            tmp     = MAX_PHASE - ( SAMPLE_TYPE ) ( std::abs( sample - .5 )) * 4.0;
                            break;

                        case WaveForms::SAWTOOTH:
                            sample += gibbs * ( 1.0 / ( SAMPLE_TYPE ) s ) * sin(( SAMPLE_TYPE ) s * TWO_PI * ( SAMPLE_TYPE ) t / numberOfSamples );
                            tmp     = sample;
                            break;

                        case WaveForms::SQUARE:
                            sample += sin(( SAMPLE_TYPE ) s * TWO_PI * ( SAMPLE_TYPE ) t / numberOfSamples );
                            tmp     = ( sample >= 0.0 ) ? MAX_PHASE : -MAX_PHASE;
                            break;
                    }
                    outputBuffer[ t ] = sample;
                }
            }
        }
    }
}
