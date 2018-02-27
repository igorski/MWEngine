/**
 * Ported from Reverb by Martin Eastwood
 * https://github.com/martineastwood/Reverb
 *
 * Copyright (c) 2010 Martin Eastwood
 * This code is distributed under the terms of the GNU General Public License

 * Reverb is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * at your option) any later version.
 *
 * Reverb is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this Reverb.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "reverb.h"

/* constructor / destructor */

Reverb::Reverb() {
    DampingFreq = 18000.;
    BandwidthFreq = 18000.;
    SampleRate = 44100.;
    Decay = 0.5;
    Gain = 1.;
    Mix = 1.;
    Size = 1.;
    EarlyMix = 1.;
    PreviousLeftTank = 0.;
    PreviousRightTank = 0.;
    PreDelayTime = 100 * (SampleRate / 1000);
    MixSmooth = EarlyLateSmooth = BandwidthSmooth = DampingSmooth = PredelaySmooth = SizeSmooth = DecaySmooth = DensitySmooth = 0.;
    ControlRate = SampleRate / 1000;
    ControlRateCounter = 0;
    reset();
}

Reverb::~Reverb() {

}

/* public methods */

void Reverb::process( AudioBuffer* sampleBuffer, bool isMonoSource )
//void Reverb::process( float **inputs, float **outputs, int sampleFrames )
{
    float OneOverSampleFrames = 1. / sampleFrames;
    float MixDelta	= (Mix - MixSmooth) * OneOverSampleFrames;
    float EarlyLateDelta = (EarlyMix - EarlyLateSmooth) * OneOverSampleFrames;
    float BandwidthDelta = (((BandwidthFreq * 18400.) + 100.) - BandwidthSmooth) * OneOverSampleFrames;
    float DampingDelta = (((DampingFreq * 18400.) + 100.) - DampingSmooth) * OneOverSampleFrames;
    float PredelayDelta = ((PreDelayTime * 200 * (SampleRate / 1000)) - PredelaySmooth) * OneOverSampleFrames;
    float SizeDelta	= (Size - SizeSmooth) * OneOverSampleFrames;
    float DecayDelta = (((0.7995f * Decay) + 0.005) - DecaySmooth) * OneOverSampleFrames;
    float DensityDelta = (((0.7995f * Density1) + 0.005) - DensitySmooth) * OneOverSampleFrames;
    for(int i=0;i<sampleFrames;++i){
        float left = inputs[0][i];
        float right = inputs[1][i];
        MixSmooth += MixDelta;
        EarlyLateSmooth += EarlyLateDelta;
        BandwidthSmooth += BandwidthDelta;
        DampingSmooth += DampingDelta;
        PredelaySmooth += PredelayDelta;
        SizeSmooth += SizeDelta;
        DecaySmooth += DecayDelta;
        DensitySmooth += DensityDelta;
        if (ControlRateCounter >= ControlRate){
            ControlRateCounter = 0;
            bandwidthFilter[0].Frequency(BandwidthSmooth);
            bandwidthFilter[1].Frequency(BandwidthSmooth);
            damping[0].Frequency(DampingSmooth);
            damping[1].Frequency(DampingSmooth);
        }
        ++ControlRateCounter;
        predelay.SetLength(PredelaySmooth);
        Density2 = DecaySmooth + 0.15;
        if (Density2 > 0.5)
            Density2 = 0.5;
        if (Density2 < 0.25)
            Density2 = 0.25;
        allpassFourTap[1].SetFeedback(Density2);
        allpassFourTap[3].SetFeedback(Density2);
        allpassFourTap[0].SetFeedback(Density1);
        allpassFourTap[2].SetFeedback(Density1);
        float bandwidthLeft = bandwidthFilter[0](left) ;
        float bandwidthRight = bandwidthFilter[1](right) ;
        float earlyReflectionsL = earlyReflectionsDelayLine[0] ( bandwidthLeft * 0.5 + bandwidthRight * 0.3 )
                            + earlyReflectionsDelayLine[0].GetIndex(2) * 0.6
                            + earlyReflectionsDelayLine[0].GetIndex(3) * 0.4
                            + earlyReflectionsDelayLine[0].GetIndex(4) * 0.3
                            + earlyReflectionsDelayLine[0].GetIndex(5) * 0.3
                            + earlyReflectionsDelayLine[0].GetIndex(6) * 0.1
                            + earlyReflectionsDelayLine[0].GetIndex(7) * 0.1
                            + ( bandwidthLeft * 0.4 + bandwidthRight * 0.2 ) * 0.5 ;
        float earlyReflectionsR = earlyReflectionsDelayLine[1] ( bandwidthLeft * 0.3 + bandwidthRight * 0.5 )
                            + earlyReflectionsDelayLine[1].GetIndex(2) * 0.6
                            + earlyReflectionsDelayLine[1].GetIndex(3) * 0.4
                            + earlyReflectionsDelayLine[1].GetIndex(4) * 0.3
                            + earlyReflectionsDelayLine[1].GetIndex(5) * 0.3
                            + earlyReflectionsDelayLine[1].GetIndex(6) * 0.1
                            + earlyReflectionsDelayLine[1].GetIndex(7) * 0.1
                            + ( bandwidthLeft * 0.2 + bandwidthRight * 0.4 ) * 0.5 ;
        float predelayMonoInput = predelay(( bandwidthRight + bandwidthLeft ) * 0.5f);
        float smearedInput = predelayMonoInput;
        for(int j=0;j<4;j++)
            smearedInput = allpass[j] ( smearedInput );
        float leftTank = allpassFourTap[0] ( smearedInput + PreviousRightTank ) ;
        leftTank = staticDelayLine[0] (leftTank);
        leftTank = damping[0](leftTank);
        leftTank = allpassFourTap[1](leftTank);
        leftTank = staticDelayLine[1](leftTank);
        float rightTank = allpassFourTap[2] (smearedInput + PreviousLeftTank) ;
        rightTank = staticDelayLine[2](rightTank);
        rightTank = damping[1] (rightTank);
        rightTank = allpassFourTap[3](rightTank);
        rightTank = staticDelayLine[3](rightTank);
        PreviousLeftTank = leftTank * DecaySmooth;
        PreviousRightTank = rightTank * DecaySmooth;
        float accumulatorL = (0.6*staticDelayLine[2].GetIndex(1))
                        +(0.6*staticDelayLine[2].GetIndex(2))
                        -(0.6*allpassFourTap[3].GetIndex(1))
                        +(0.6*staticDelayLine[3].GetIndex(1))
                        -(0.6*staticDelayLine[0].GetIndex(1))
                        -(0.6*allpassFourTap[1].GetIndex(1))
                        -(0.6*staticDelayLine[1].GetIndex(1));
        float accumulatorR = (0.6*staticDelayLine[0].GetIndex(2))
                        +(0.6*staticDelayLine[0].GetIndex(3))
                        -(0.6*allpassFourTap[1].GetIndex(2))
                        +(0.6*staticDelayLine[1].GetIndex(2))
                        -(0.6*staticDelayLine[2].GetIndex(3))
                        -(0.6*allpassFourTap[3].GetIndex(2))
                        -(0.6*staticDelayLine[3].GetIndex(2));
        accumulatorL = ((accumulatorL * EarlyMix) + ((1 - EarlyMix) * earlyReflectionsL));
        accumulatorR = ((accumulatorR * EarlyMix) + ((1 - EarlyMix) * earlyReflectionsR));
        left = ( left + MixSmooth * ( accumulatorL - left ) ) * Gain;
        right = ( right + MixSmooth * ( accumulatorR - right ) ) * Gain;
        outputs[0][i] = left;
        outputs[1][i] = right;
    }
}

void Reverb::reset()
{
    ControlRateCounter = 0;
    bandwidthFilter[0].SetSampleRate (SampleRate );
    bandwidthFilter[1].SetSampleRate (SampleRate );
    bandwidthFilter[0].Reset();
    bandwidthFilter[1].Reset();
    damping[0].SetSampleRate (SampleRate );
    damping[1].SetSampleRate (SampleRate );
    damping[0].Reset();
    damping[1].Reset();
    predelay.Clear();
    predelay.SetLength(PreDelayTime);
    allpass[0].Clear();
    allpass[1].Clear();
    allpass[2].Clear();
    allpass[3].Clear();
    allpass[0].SetLength (0.0048 * SampleRate);
    allpass[1].SetLength (0.0036 * SampleRate);
    allpass[2].SetLength (0.0127 * SampleRate);
    allpass[3].SetLength (0.0093 * SampleRate);
    allpass[0].SetFeedback (0.75);
    allpass[1].SetFeedback (0.75);
    allpass[2].SetFeedback (0.625);
    allpass[3].SetFeedback (0.625);
    allpassFourTap[0].Clear();
    allpassFourTap[1].Clear();
    allpassFourTap[2].Clear();
    allpassFourTap[3].Clear();
    allpassFourTap[0].SetLength(0.020 * SampleRate * Size);
    allpassFourTap[1].SetLength(0.060 * SampleRate * Size);
    allpassFourTap[2].SetLength(0.030 * SampleRate * Size);
    allpassFourTap[3].SetLength(0.089 * SampleRate * Size);
    allpassFourTap[0].SetFeedback(Density1);
    allpassFourTap[1].SetFeedback(Density2);
    allpassFourTap[2].SetFeedback(Density1);
    allpassFourTap[3].SetFeedback(Density2);
    allpassFourTap[0].SetIndex(0,0,0,0);
    allpassFourTap[1].SetIndex(0,0.006 * SampleRate * Size, 0.041 * SampleRate * Size, 0);
    allpassFourTap[2].SetIndex(0,0,0,0);
    allpassFourTap[3].SetIndex(0,0.031 * SampleRate * Size, 0.011 * SampleRate * Size, 0);
    staticDelayLine[0].Clear();
    staticDelayLine[1].Clear();
    staticDelayLine[2].Clear();
    staticDelayLine[3].Clear();
    staticDelayLine[0].SetLength(0.15 * SampleRate * Size);
    staticDelayLine[1].SetLength(0.12 * SampleRate * Size);
    staticDelayLine[2].SetLength(0.14 * SampleRate * Size);
    staticDelayLine[3].SetLength(0.11 * SampleRate * Size);
    staticDelayLine[0].SetIndex(0, 0.067 * SampleRate * Size, 0.011 * SampleRate * Size , 0.121 * SampleRate * Size);
    staticDelayLine[1].SetIndex(0, 0.036 * SampleRate * Size, 0.089 * SampleRate * Size , 0);
    staticDelayLine[2].SetIndex(0, 0.0089 * SampleRate * Size, 0.099 * SampleRate * Size , 0);
    staticDelayLine[3].SetIndex(0, 0.067 * SampleRate * Size, 0.0041 * SampleRate * Size , 0);
    earlyReflectionsDelayLine[0].Clear();
    earlyReflectionsDelayLine[1].Clear();
    earlyReflectionsDelayLine[0].SetLength(0.089 * SampleRate);
    earlyReflectionsDelayLine[0].SetIndex (0, 0.0199*SampleRate, 0.0219*SampleRate, 0.0354*SampleRate,0.0389*SampleRate, 0.0414*SampleRate, 0.0692*SampleRate, 0);
    earlyReflectionsDelayLine[1].SetLength(0.069 * SampleRate);
    earlyReflectionsDelayLine[1].SetIndex (0, 0.0099*SampleRate, 0.011*SampleRate, 0.0182*SampleRate,0.0189*SampleRate, 0.0213*SampleRate, 0.0431*SampleRate, 0);
}

void Reverb::setParameter( int index, float value )
{
    switch( index )
    {
        case params.DAMPINGFREQ:
            DampingFreq =  1. - value;
            break;

        case params.DENSITY:
            Density1 = value;
            break;

        case params.BANDWIDTHFREQ:
            BandwidthFreq = value;
            break;

        case params.PREDELAY:
            PreDelayTime = value;
            break;

        case params.SIZE:
            Size = (0.95 * value) + 0.05;
            allpassFourTap[0].Clear();
            allpassFourTap[1].Clear();
            allpassFourTap[2].Clear();
            allpassFourTap[3].Clear();
            allpassFourTap[0].SetLength(0.020 * SampleRate * Size);
            allpassFourTap[1].SetLength(0.060 * SampleRate * Size);
            allpassFourTap[2].SetLength(0.030 * SampleRate * Size);
            allpassFourTap[3].SetLength(0.089 * SampleRate * Size);
            allpassFourTap[1].SetIndex(0,0.006 * SampleRate * Size, 0.041 * SampleRate * Size, 0);
            allpassFourTap[3].SetIndex(0,0.031 * SampleRate * Size, 0.011 * SampleRate * Size, 0);
            staticDelayLine[0].Clear();
            staticDelayLine[1].Clear();
            staticDelayLine[2].Clear();
            staticDelayLine[3].Clear();
            staticDelayLine[0].SetLength(0.15 * SampleRate * Size);
            staticDelayLine[1].SetLength(0.12 * SampleRate * Size);
            staticDelayLine[2].SetLength(0.14 * SampleRate * Size);
            staticDelayLine[3].SetLength(0.11 * SampleRate * Size);
            staticDelayLine[0].SetIndex(0, 0.067 * SampleRate * Size, 0.011 * SampleRate * Size , 0.121 * SampleRate * Size);
            staticDelayLine[1].SetIndex(0, 0.036 * SampleRate * Size, 0.089 * SampleRate * Size , 0);
            staticDelayLine[2].SetIndex(0, 0.0089 * SampleRate * Size, 0.099 * SampleRate * Size , 0);
            staticDelayLine[3].SetIndex(0, 0.067 * SampleRate * Size, 0.0041 * SampleRate * Size , 0);
            break;

        case params.DECAY:
            Decay = value;
            break;

        case params.GAIN:
            Gain = value;
            break;

        case params.MIX:
            Mix = value;
            break;

        case params.EARLYMIX:
            EarlyMix = value;
            break;
    }
}

float Reverb::getParameter( int index )
{
    switch( index ) {
        case DAMPINGFREQ:
                return DampingFreq * 100.;
                break;
        case DENSITY:
                return Density1 * 100.f;
                break;
        case BANDWIDTHFREQ:
                return BandwidthFreq * 100.;
                break;
        case PREDELAY:
                return PreDelayTime * 100.;
                break;
        case SIZE:
                return (((0.95 * Size) + 0.05)*100.);
                break;
        case DECAY:
                return Decay * 100.f;
                break;
        case GAIN:
                return Gain * 100.f;
                break;
        case MIX:
                return Mix * 100.f;
                break;
        case EARLYMIX:
                return EarlyMix * 100.f;
                break;
        default: return 0.f;
            break;

    }
}

void Reverb::setSampleRate( int sampleRate ){
    SampleRate = sr;
    ControlRate = SampleRate / 1000;
    reset();
}

/* inner classes */

// StaticAllpassFourTap

StaticAllpassFourTap()
{
    SetLength ( maxLength - 1 );
    Clear();
    Feedback = 0.5;
}

float operator()(float input)
{
    float output;
    float bufout;

    bufout = buffer[index1];
    float temp = input * -Feedback;
    output = bufout + temp;
    buffer[index1] = input + ((bufout+temp)*Feedback);

    if(++index1>=Length)
        index1 = 0;
    if(++index2 >= Length)
        index2 = 0;
    if(++index3 >= Length)
        index3 = 0;
    if(++index4 >= Length)
        index4 = 0;

    return output;

}

void SetIndex (int Index1, int Index2, int Index3, int Index4)
{
    index1 = Index1;
    index2 = Index2;
    index3 = Index3;
    index4 = Index4;
}

float GetIndex (int Index)
{
    switch (Index)
    {
        case 0:
            return buffer[index1];
            break;
        case 1:
            return buffer[index2];
            break;
        case 2:
            return buffer[index3];
            break;
        case 3:
            return buffer[index4];
            break;
        default:
            return buffer[index1];
            break;
    }
}

void SetLength (int Length)
{
   if( Length >= maxLength )
        Length = maxLength;
   if( Length < 0 )
        Length = 0;

    this->Length = Length;
}


void Clear()
{
    memset(buffer, 0, sizeof(buffer));
    index1 = index2  = index3 = index4 = 0;
}

void SetFeedback(float feedback)
{
    Feedback = feedback;
}


int GetLength() const
{
    return Length;
}

// StaticDelayLine

StaticDelayLine::StaticDelayLine()
{
    SetLength ( maxLength - 1 );
    Clear();
}

void StaticDelayLine::SetLength( int Length )
{
   if( Length >= maxLength )
        Length = maxLength;

   if( Length < 0 )
        Length = 0;

    this->Length = Length;
}

void StaticDelayLine::Clear()
{
    memset(buffer, 0, sizeof(buffer));
    index = 0;
}

int StaticDelayLine::GetLength() const
{
    return Length;
}

// StaticDelayLineFourTap

StaticDelayLineFourTap::StaticDelayLineFourTap()
{
    SetLength( maxLength - 1 );
    Clear();
}

void StaticDelayLineFourTap::SetIndex( int Index1, int Index2, int Index3, int Index4 )
{
    index1 = Index1;
    index2 = Index2;
    index3 = Index3;
    index4 = Index4;
}

float StaticDelayLineFourTap::GetIndex( int Index )
{
    switch (Index)
    {
        case 0:
            return buffer[index1];
            break;
        case 1:
            return buffer[index2];
            break;
        case 2:
            return buffer[index3];
            break;
        case 3:
            return buffer[index4];
            break;
        default:
            return buffer[index1];
            break;
    }
}

void StaticDelayLineFourTap::SetLength( int Length )
{
   if( Length >= maxLength )
        Length = maxLength;
   if( Length < 0 )
        Length = 0;

    this->Length = Length;
}


void StaticDelayLineFourTap::Clear()
{
    memset(buffer, 0, sizeof(buffer));
    index1 = index2  = index3 = index4 = 0;
}


int StaticDelayLineFourTap::GetLength() const
{
    return Length;
}

// StaticDelayLineEightTap

StaticDelayLineEightTap::StaticDelayLineEightTap()
{
    SetLength ( maxLength - 1 );
    Clear();
}

void StaticDelayLineEightTap::SetIndex (int Index1, int Index2, int Index3, int Index4, int Index5, int Index6, int Index7, int Index8)
{
    index1 = Index1;
    index2 = Index2;
    index3 = Index3;
    index4 = Index4;
    index5 = Index5;
    index6 = Index6;
    index7 = Index7;
    index8 = Index8;
}


float StaticDelayLineEightTap::GetIndex( int Index )
{
    switch (Index)
    {
        case 0:
            return buffer[index1];
            break;

        case 1:
            return buffer[index2];
            break;

        case 2:
            return buffer[index3];
            break;

        case 3:
            return buffer[index4];
            break;

        case 4:
            return buffer[index5];
            break;

        case 5:
            return buffer[index6];
            break;

        case 6:
            return buffer[index7];
            break;

        case 7:
            return buffer[index8];
            break;

        default:
            return buffer[index1];
            break;
    }
}

void StaticDelayLineEightTap::SetLength( int Length )
{
    if( Length >= maxLength )
        Length = maxLength;
    if( Length < 0 )
        Length = 0;

    this->Length = Length;
}

void StaticDelayLineEightTap::Clear()
{
    memset( buffer, 0, sizeof( buffer ));
    index1 = index2  = index3 = index4 = index5 = index6 = index7 = index8 = 0;
}


int StaticDelayLineEightTap::GetLength() const
{
    return Length;
}

// StateVariable

StateVariable::StateVariable()
{
    SetSampleRate(44100.);
    Frequency(1000.);
    Resonance(0);
    Type(LOWPASS);
    Reset();
}

void StateVariable::Reset()
{
    low = high = band = notch = 0;
}

void StateVariable::SetSampleRate( float sampleRate )
{
    this->sampleRate = sampleRate * OverSampleCount;
    UpdateCoefficient();
}

void StateVariable::Frequency( float frequency )
{
    this->frequency = frequency;
    UpdateCoefficient();
}

void StateVariable::Resonance(float resonance)
{
    this->q = 2 - 2 * resonance;
}

void StateVariable::Type( int type )
{
    switch(type)
    {
        case LOWPASS:
            out = &low;
            break;

        case HIGHPASS:
            out = &high;
            break;

        case BANDPASS:
            out = &band;
            break;

        case NOTCH:
            out = &notch;
            break;

        default:
            out = &low;
            break;
    }
}

void StateVariable::UpdateCoefficient()
{
    f = 2. * sinf( PI * frequency / sampleRate );
}
