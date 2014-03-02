#ifndef __FORMANTFILTER_H_INCLUDED__
#define __FORMANTFILTER_H_INCLUDED__

#include "baseprocessor.h"

class FormantFilter : public BaseProcessor
{
    public:
        FormantFilter( float aVowel );
        ~FormantFilter();
        void setVowel( float aVowel );
        float getVowel();
        void process( float* sampleBuffer, int bufferLength );

        /**
        * adapted from public source code by alex@smartelectronix.com
        * Simple example of implementation of formant filter
        * Vowelnum can be 0,1,2,3,4 <=> A,E,I,O,U
        * Good for spectral rich input like saw or square
        */
        static const int VOWEL_A = 0;
        static const int VOWEL_E = 1;
        static const int VOWEL_I = 2;
        static const int VOWEL_O = 3;
        static const int VOWEL_U = 4;

    private:
        float  _vowel;
        float _currentCoeffs[ 11 ];
        float _coeffs[ 5 ][ 11 ];
        float _memory[ 10 ];
        void calculateCoeffs();
};

#endif
