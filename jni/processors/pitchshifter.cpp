/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2017 Igor Zinken - http://www.igorski.nl
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
#include "pitchshifter.h"
#include "../audioengine.h"

/* constructors / destructors */

PitchShifter::PitchShifter( float shiftAmount, long osampAmount )
{
    gRover       = false;
    pitchShift   = shiftAmount; // 0.5 is octave down, 1 == normal, 2 is octave up

    fftFrameSize = MAX_FRAME_LENGTH / 2;
    osamp        = osampAmount; // at least 4 for moderate scaling, 32 for max. quality

    fftFrameSize2 = fftFrameSize / 2;
    stepSize      = fftFrameSize / osamp;
    freqPerBin    = ( SAMPLE_TYPE ) AudioEngineProps::SAMPLE_RATE / ( SAMPLE_TYPE ) fftFrameSize;
    expct         = 2.0f * M_PI * ( SAMPLE_TYPE ) stepSize /( SAMPLE_TYPE ) fftFrameSize;
    inFifoLatency = fftFrameSize - stepSize;

    /* initialize our static arrays */

    memset( gInFIFO,      0, MAX_FRAME_LENGTH * sizeof( float ));
    memset( gOutFIFO,     0, MAX_FRAME_LENGTH * sizeof( float ));
    memset( gFFTworksp,   0, 2 * MAX_FRAME_LENGTH * sizeof( float ));
    memset( gLastPhase,   0, ( MAX_FRAME_LENGTH / 2 + 1 ) * sizeof( float ));
    memset( gSumPhase,    0, ( MAX_FRAME_LENGTH / 2 + 1 ) * sizeof( float ));
    memset( gOutputAccum, 0, 2 * MAX_FRAME_LENGTH * sizeof( float ));
    memset( gAnaFreq,     0, MAX_FRAME_LENGTH * sizeof( float ));
    memset( gAnaMagn,     0, MAX_FRAME_LENGTH * sizeof( float ));
}

PitchShifter::~PitchShifter()
{
    // TODO: clear memsets!
    // ...though in the current implementation this is never destructed
}

/* public methods */

void PitchShifter::process( AudioBuffer* sampleBuffer, bool isMonoSource )
{
    // pitch shifted to "normal" ? omit processing and save CPU cycles

    if ( pitchShift == 1.0f )
        return;

    long i, k;

    if ( gRover == false )
        gRover = inFifoLatency;

    int bufferSize = sampleBuffer->bufferSize;

    for ( int cn = 0, cl = sampleBuffer->amountOfChannels; cn < cl; ++cn )
    {
        SAMPLE_TYPE* channelBuffer = sampleBuffer->getBufferForChannel( cn );

        /* main processing loop */

        for ( i = 0; i < bufferSize; ++i )
        {
            /* As long as we have not yet collected enough data just read in */

            gInFIFO[ gRover ]  = channelBuffer[ i ];
            channelBuffer[ i ] = gOutFIFO[ gRover - inFifoLatency ];
            gRover++;

            /* now we have enough data for processing */

            if ( gRover >= fftFrameSize )
            {
                gRover = inFifoLatency;

                /* do windowing and re,im interleave */

                for ( k = 0; k < fftFrameSize; ++k )
                {
                    window                  = -.5 * cos( 2. * M_PI * ( SAMPLE_TYPE ) k / ( SAMPLE_TYPE ) fftFrameSize ) + .5;
                    gFFTworksp[ 2 * k ]     = gInFIFO[ k ] * window;
                    gFFTworksp[ 2 * k + 1 ] = 0.;
                }

                /* ***************** ANALYSIS ******************* */
                /* do transform */
                smbFft( gFFTworksp, fftFrameSize, -1 );

                /* this is the analysis step */

                for ( k = 0; k <= fftFrameSize2; ++k )
                {
                    /* de-interlace FFT buffer */
                    real = gFFTworksp[ 2 * k ];
                    imag = gFFTworksp[ 2 * k + 1 ];

                    /* compute magnitude and phase */
                    magn  = 2. * sqrt( real * real + imag * imag );
                    phase = atan2( imag, real );

                    /* compute phase difference */
                    tmp             = phase - gLastPhase[ k ];
                    gLastPhase[ k ] = phase;

                    /* subtract expected phase difference */
                    tmp -= ( SAMPLE_TYPE ) k * expct;

                    /* map delta phase into +/- Pi interval */
                    qpd = tmp / M_PI;
                    if ( qpd >= 0 )
                        qpd += qpd&1;
                    else
                        qpd -= qpd&1;

                    tmp -= M_PI * ( SAMPLE_TYPE ) qpd;

                    /* get deviation from bin frequency from the +/- Pi interval */
                    tmp = osamp * tmp / ( 2. * M_PI );

                    /* compute the k-th partials' true frequency */
                    tmp = ( SAMPLE_TYPE ) k * freqPerBin + tmp * freqPerBin;

                    /* store magnitude and true frequency in analysis arrays */
                    gAnaMagn[ k ] = magn;
                    gAnaFreq[ k ] = tmp;
                }

                /* ***************** PROCESSING ******************* */
                /* this does the actual pitch shifting */
                memset( gSynMagn, 0, fftFrameSize * sizeof( float ));
                memset( gSynFreq, 0, fftFrameSize * sizeof( float ));

                for ( k = 0; k <= fftFrameSize2; ++k )
                {
                    index = k * pitchShift;
                    if ( index <= fftFrameSize2 )
                    {
                        gSynMagn[ index ] += gAnaMagn[ k ];
                        gSynFreq[ index ] = gAnaFreq[ k ] * pitchShift;
                    }
                }

                /* ***************** SYNTHESIS ******************* */
                /* this is the synthesis step */
                for ( k = 0; k <= fftFrameSize2; ++k )
                {
                    /* get magnitude and true frequency from synthesis arrays */
                    magn = gSynMagn[ k ];
                    tmp  = gSynFreq[ k ];

                    /* subtract bin mid frequency */
                    tmp -= ( SAMPLE_TYPE ) k * freqPerBin;

                    /* get bin deviation from freq deviation */
                    tmp /= freqPerBin;

                    /* take osamp into account */
                    tmp = 2. * M_PI * tmp / osamp;

                    /* add the overlap phase advance back in */
                    tmp += ( SAMPLE_TYPE ) k * expct;

                    /* accumulate delta phase to get bin phase */
                    gSumPhase[ k ] += tmp;
                    phase           = gSumPhase[ k ];

                    /* get real and imag part and re-interleave */
                    gFFTworksp[ 2 * k ]     = magn * cos( phase );
                    gFFTworksp[ 2 * k + 1 ] = magn * sin( phase );
                }

                /* zero negative frequencies */
                for ( k = fftFrameSize + 2; k < 2 * fftFrameSize; ++k )
                    gFFTworksp[ k ] = 0.;

                /* do inverse transform */
                smbFft( gFFTworksp, fftFrameSize, 1 );

                /* do windowing and add to output accumulator */
                for ( k = 0; k < fftFrameSize; ++k )
                {
                    window             = -.5 * cos( 2. * M_PI * ( SAMPLE_TYPE ) k / ( SAMPLE_TYPE ) fftFrameSize ) + .5;
                    gOutputAccum[ k ] += 2. * window * gFFTworksp[ 2 * k ] / ( fftFrameSize2 * osamp );
                }

                for ( k = 0; k < stepSize; ++k )
                    gOutFIFO[ k ] = gOutputAccum[ k ];

                /* shift accumulator */
                memmove( gOutputAccum, gOutputAccum + stepSize, fftFrameSize * sizeof( float ));

                /* move input FIFO */
                for ( k = 0; k < inFifoLatency; ++k )
                    gInFIFO[ k ] = gInFIFO[ k + stepSize ];
            }
        }
        // save CPU cycles when mono source
        if ( isMonoSource )
        {
            sampleBuffer->applyMonoSource();
            break;
        }
    }
}

bool PitchShifter::isCacheable()
{
    return true;
}
