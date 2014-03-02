#ifndef __LFO_H_INCLUDED__
#define __LFO_H_INCLUDED__

class LFO
{
    public:
        LFO();
        ~LFO();

        static const float MAX_LFO_RATE = 10;   // the maximum rate of oscillation in Hz
        static const float MIN_LFO_RATE = .1;   // the minimum rate of oscillation in Hz

        float getRate();
        void setRate( float value );
        int getWave();
        void setWave( int value );
        void generate( int aLength );
        int getLength();
        void setLength( int value );

        float peek();

    protected:
        float TWO_PI_OVER_SR;
        float TWO_PI;

        float _phase;
        float _phaseIncr;
        float _rate;
        float* _buffer; // cached buffer
        int _wave;
        int _length;
        int _readOffset;

        int calculateBufferLength( float aMinRate );
};

#endif
