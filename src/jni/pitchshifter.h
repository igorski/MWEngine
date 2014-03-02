#ifndef __PITCHSHIFTER_H_INCLUDED__
#define __PITCHSHIFTER_H_INCLUDED__

#include "baseprocessor.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

#define M_PI 3.14159265358979323846
#define MAX_FRAME_LENGTH 8192

void smbFft    ( float *fftBuffer, long fftFrameSize, long sign );
float smbAtan2( float x, float y );

class PitchShifter : public BaseProcessor
{
    public:
        PitchShifter( float shiftAmount, long osampAmount );
        ~PitchShifter();
        void process( float* sampleBuffer, int bufferLength );
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
        float magn, phase, tmp, window, real, imag;
        float freqPerBin, expct;
        long qpd, index, inFifoLatency, stepSize, fftFrameSize, fftFrameSize2, osamp;
};

#endif
