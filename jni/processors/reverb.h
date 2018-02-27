/**
 * Ported from mVerb by Martin Eastwood
 * https://github.com/martineastwood/mverb
 *
 * Copyright (c) 2010 Martin Eastwood
 * This code is distributed under the terms of the GNU General Public License

 * MVerb is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * at your option) any later version.
 *
 * MVerb is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this MVerb.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "baseprocessor.h"

#ifndef EMVERB_H
#define EMVERB_H

//forward declaration
template<typename T, int maxLength> class Allpass;
template<typename T, int maxLength> class StaticAllpassFourTap;
template<typename T, int maxLength> class StaticDelayLine;
template<typename T, int maxLength> class StaticDelayLineFourTap;
template<typename T, int maxLength> class StaticDelayLineEightTap;
template<typename T, int OverSampleCount> class StateVariable;

class Reverb : public BaseProcessor
{
    public:
        enum params
        {
            DAMPINGFREQ = 0,
            DENSITY,
            BANDWIDTHFREQ,
            DECAY,
            PREDELAY,
            SIZE,
            GAIN,
            MIX,
            EARLYMIX,
            NUM_PARAMS
        };

        float getParameter( int index );
        void setParameter( int index, float value );
        void setSampleRate( int sampleRate );

        void process( AudioBuffer* sampleBuffer, bool isMonoSource );
        void reset();

    private:
        Allpass<T, 96000> allpass[4];
        StaticAllpassFourTap<T, 96000> allpassFourTap[4];
        StateVariable<T,4> bandwidthFilter[2];
        StateVariable<T,4> damping[2];
        StaticDelayLine<T, 96000> predelay;
        StaticDelayLineFourTap<T, 96000> staticDelayLine[4];
        StaticDelayLineEightTap<T, 96000> earlyReflectionsDelayLine[2];
        float SampleRate, DampingFreq, Density1, Density2, BandwidthFreq, PreDelayTime, Decay, Gain, Mix, EarlyMix, Size;
        float MixSmooth, EarlyLateSmooth, BandwidthSmooth, DampingSmooth, PredelaySmooth, SizeSmooth, DensitySmooth, DecaySmooth;
        float PreviousLeftTank, PreviousRightTank;
        int ControlRate, ControlRateCounter;
};

template<typename T, int maxLength>
class Allpass
{
    public:
        Allpass()
        {
            SetLength ( maxLength - 1 );
            Clear();
            Feedback = 0.5;
        }

        float operator()(float input)
        {
            float output;
            float bufout;
            bufout = buffer[index];
            float temp = input * -Feedback;
            output = bufout + temp;
            buffer[index] = input + ((bufout+temp)*Feedback);
            if(++index>=Length) index = 0;
            return output;

        }

        void SetLength (int Length)
        {
           if( Length >= maxLength )
                Length = maxLength;
           if( Length < 0 )
                Length = 0;

            this->Length = Length;
        }

        void SetFeedback(float feedback)
        {
            Feedback = feedback;
        }

        void Clear()
        {
            memset(buffer, 0, sizeof(buffer));
            index = 0;
        }

        int GetLength() const
        {
            return Length;
        }

    private:
        float buffer[maxLength];
        int index;
        int Length;
        float Feedback;
};

class StaticAllpassFourTap
{
    private:
        float buffer[maxLength];
        int index1, index2, index3, index4;
        int Length;
        float Feedback;

    public:
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
};

class StaticDelayLine
{
    public:
        StaticDelayLine();

        float operator()( float input )
        {
            float output = buffer[index];
            buffer[index++] = input;
            if(index >= Length)
                index = 0;
            return output;

        }

        void SetLength( int Length );
        void Clear();
        int GetLength() const;

    private:
        float buffer[maxLength];
        int index;
        int Length;
        float Feedback;
};

class StaticDelayLineFourTap
{
    public:
        StaticDelayLineFourTap();

        void SetIndex( int Index1, int Index2, int Index3, int Index4 );
        float GetIndex( int Index );
        void SetLength( int Length );
        void Clear();
        int GetLength() const;

        //get output and iterate
        float operator()( float input )
        {
            float output = buffer[index1];
            buffer[index1++] = input;
            if(index1 >= Length)
                index1 = 0;
            if(++index2 >= Length)
                index2 = 0;
            if(++index3 >= Length)
                index3 = 0;
            if(++index4 >= Length)
                index4 = 0;
            return output;

        }

    private:
        float buffer[maxLength];
        int index1, index2, index3, index4;
        int Length;
        float Feedback;
};

class StaticDelayLineEightTap
{
    public:
        StaticDelayLineEightTap();
        void SetIndex( int Index1, int Index2, int Index3, int Index4, int Index5, int Index6, int Index7, int Index8);
        float GetIndex( int Index );
        void SetLength( int Length );
        void Clear();
        int GetLength() const;

        //get output and iterate
        float operator()( float input )
        {
            float output = buffer[index1];
            buffer[index1++] = input;
            if(index1 >= Length)
                index1 = 0;
            if(++index2 >= Length)
                index2 = 0;
            if(++index3 >= Length)
                index3 = 0;
            if(++index4 >= Length)
                index4 = 0;
            if(++index5 >= Length)
                index5 = 0;
            if(++index6 >= Length)
                index6 = 0;
            if(++index7 >= Length)
                index7 = 0;
            if(++index8 >= Length)
                index8 = 0;
            return output;

        }

    private:
        float buffer[maxLength];
        int index1, index2, index3, index4, index5, index6, index7, index8;
        int Length;
        float Feedback;
};

class StateVariable
{
    public:
        enum FilterType
        {
            LOWPASS,
            HIGHPASS,
            BANDPASS,
            NOTCH,
            FilterTypeCount
        };

        StateVariable();

        float operator()( float input )
        {
            for(unsigned int i = 0; i < OverSampleCount; i++)
            {
                low += f * band + 1e-25;
                high = input - low - q * band;
                band += f * high;
                notch = low + high;
            }
            return *out;
        }

        void Reset();
        void SetSampleRate( float sampleRate );
        void Frequency( float frequency );
        void Resonance( float resonance );
        void Type( int type );

    private:
        float sampleRate;
        float frequency;
        float q;
        float f;

        float low;
        float high;
        float band;
        float notch;

        float *out;
        void UpdateCoefficient();
};
#endif
