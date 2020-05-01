/**
 * The MIT License (MIT)
 *
 * based on public source code by alex@smartelectronix.com, adapted
 * by Igor Zinken - https://www.igorski.nl
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
#ifndef __MWENGINE__FORMANTFILTER_H_INCLUDED__
#define __MWENGINE__FORMANTFILTER_H_INCLUDED__

#include "baseprocessor.h"

namespace MWEngine {
class FormantFilter : public BaseProcessor
{
    public:
        FormantFilter( double aVowel );
        ~FormantFilter();

        std::string getType() {
            return std::string( "FormantFilter" );
        }

        void setVowel( double aVowel );
        double getVowel();

#ifndef SWIG
        // internal to the engine
        void process( AudioBuffer* audioBuffer, bool isMonoSource );
        bool isCacheable();
#endif

        static const int VOWEL_A = 0;
        static const int VOWEL_E = 1;
        static const int VOWEL_I = 2;
        static const int VOWEL_O = 3;
        static const int VOWEL_U = 4;

    private:
        double  _vowel;
        double _currentCoeffs[ 11 ];
        double _coeffs[ 5 ][ 11 ];
        double _memory[ 10 ];
        void calculateCoeffs();
};
} // E.O namespace MWEngine

#endif
