/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2020 Igor Zinken - https://www.igorski.nl
 * Based on:
 *
 * NAME: smbPitchShift.cpp
 * VERSION: 1.2
 * HOME URL: http://blogs.zynaptiq.com/bernsee
 *
 * by S.M. Bernsee (1999-2015)
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
#ifndef __MWENGINE__PITCHSHIFTER_H_INCLUDED__
#define __MWENGINE__PITCHSHIFTER_H_INCLUDED__

#include "baseprocessor.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

#define M_PI 3.14159265358979323846
#define MAX_FRAME_LENGTH 8192

namespace MWEngine {
class PitchShifter : public BaseProcessor
{
    public:

        /**
         * DESCRIPTION: The routine takes a pitchShift factor value which is between 0.5
         * (one octave down) and 2. (one octave up). A value of exactly 1 does not change
         * the pitch. numSampsToProcess tells the routine how many samples in indata[0...
         * numSampsToProcess-1] should be pitch shifted and moved to outdata[0 ...
         * numSampsToProcess-1]. The two buffers can be identical (ie. it can process the
         * data in-place). fftFrameSize defines the FFT frame size used for the
         * processing. Typical values are 1024, 2048 and 4096. It may be any value <=
         * MAX_FRAME_LENGTH but it MUST be a power of 2. osamp is the STFT
         * oversampling factor which also determines the overlap between adjacent STFT
         * frames. It should at least be 4 for moderate scaling ratios. A value of 32 is
         * recommended for best quality. sampleRate takes the sample rate for the signal
         * in unit Hz, ie. 44100 for 44.1 kHz audio. The data passed to the routine in
         * indata[] should be in the range [-1.0, 1.0), which is also the output range
         * for the data, make sure you scale the data accordingly (for 16bit signed integers
         * you would have to divide (and multiply) by 32768)
         */
        PitchShifter( float shiftAmount, long osampAmount );
        ~PitchShifter();

        std::string getType() {
            return std::string( "PitchShifter" );
        }

#ifndef SWIG
        // internal to the engine
        void process( AudioBuffer* sampleBuffer, bool isMonoSource );
        bool isCacheable();
#endif

        float pitchShift;

    private:
        float gInFIFO     [ MAX_FRAME_LENGTH ];
        float gOutFIFO    [ MAX_FRAME_LENGTH ];
        float gFFTworksp  [ 2 * MAX_FRAME_LENGTH ];
        float gLastPhase  [ MAX_FRAME_LENGTH / 2 + 1 ];
        float gSumPhase   [ MAX_FRAME_LENGTH / 2 + 1 ];
        float gOutputAccum[ 2 * MAX_FRAME_LENGTH ];
        float gAnaFreq    [ MAX_FRAME_LENGTH ];
        float gAnaMagn    [ MAX_FRAME_LENGTH ];
        float gSynFreq    [ MAX_FRAME_LENGTH ];
        float gSynMagn    [ MAX_FRAME_LENGTH ];
        long gRover;
        SAMPLE_TYPE magn, phase, tmp, window, real, imag, freqPerBin, expct, invFftFrameSizePI2, invFftFrameSize2, osampPI2;
        long qpd, index, inFifoLatency, stepSize, fftFrameSize, fftFrameSize2, osamp;

        // inlining this FFT routine (by S.M. Bernsee, 1996) provides a 21% performance boost

        inline void smbFft( float *fftBuffer, long fftFrameSize, long sign )
        {
            float wr, wi, arg, *p1, *p2, temp;
            float tr, ti, ur, ui, *p1r, *p1i, *p2r, *p2i;
            long doubleFftFrameSize = 2 * fftFrameSize, i, end, bitm, j, le, le2, k;

            for ( i = 2, end = doubleFftFrameSize - 2; i < end; i += 2 ) {
                for ( bitm = 2, j = 0; bitm < doubleFftFrameSize; bitm <<= 1 ) {
                    if ( i & bitm ) ++j;
                    j <<= 1;
                }
                if ( i < j ) {
                    p1 = fftBuffer+i;
                    p2 = fftBuffer+j;
                    temp = *p1; *(p1++) = *p2;
                    *(p2++) = temp; temp = *p1;
                    *p1 = *p2; *p2 = temp;
                }
            }

            for ( k = 0, le = 2, end = ( long )( log( fftFrameSize ) / log( 2. ) + .5 ); k < end; ++k )
            {
                le <<= 1;
                le2  = le >> 1;
                ur  = 1.0;
                ui  = 0.0;
                arg = M_PI / ( le2 >>1 );
                wr  = cos( arg );
                wi  = sign * sin( arg );
                for ( j = 0; j < le2; j += 2 ) {
                    p1r = fftBuffer + j;
                    p1i = p1r + 1;
                    p2r = p1r + le2;
                    p2i = p2r + 1;

                    for ( i = j; i < doubleFftFrameSize; i += le ) {
                        tr = *p2r * ur - *p2i * ui;
                        ti = *p2r * ui + *p2i * ur;
                        *p2r = *p1r - tr; *p2i = *p1i - ti;
                        *p1r += tr; *p1i += ti;
                        p1r += le; p1i += le;
                        p2r += le; p2i += le;
                    }
                    tr = ur*wr - ui*wi;
                    ui = ur*wi + ui*wr;
                    ur = tr;
                }
            }
        }

};
} // E.O namespace MWEngine

#endif
