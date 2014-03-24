/**
 * The MIT License (MIT)
 *
 * based on public source code by alex@smartelectronix.com, adapted
 * by Igor Zinken - http://www.igorski.nl
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
#include "formant_filter.h"
#include "utils.h"
#include <cfloat>
#include <cmath>

/* constructor / destructor */

/**
 * @param aVowel {double} interpolated value within
 *                       the range of the amount specified in the coeffs Array
 */
FormantFilter::FormantFilter( double aVowel )
{
    int i = 0;
    for ( i; i < 11; i++ )
        _currentCoeffs[ i ] = 0.0;

    for ( i = 0; i < 10; i++ )
        _memory[ i ] = 0.0;

    calculateCoeffs();
    setVowel( aVowel );
}

FormantFilter::~FormantFilter()
{
    delete _memory;
    delete _currentCoeffs;
}

/* public methods */

double FormantFilter::getVowel()
{
    return _vowel;
}

void FormantFilter::setVowel( double aVowel )
{
    _vowel = aVowel;

    double interpolationThreshold = .7;

    // Linearly interpolate the values when below threshold
    // TODO : this is weak ;) keep calculated coeffecient values within double range

    if ( aVowel < interpolationThreshold )
    {
        int roundVowel  = ( int )( aVowel * 4.0 );
        double fracpart = aVowel - roundVowel;

        for ( int i = 0; i < 11; i++ )
        {
            _currentCoeffs[ i ] = _coeffs[ roundVowel ][ i ] * ( 1.0 - fracpart ) +
                                ( _coeffs[ roundVowel + ( roundVowel < 4 )][ i ] * fracpart );
        }
    }
    else
    {
        // interpolation beyond this point is quite nasty... keep in exact range

        int min      = ( int ) round( aVowel );
        int max      = min < ( 4 /* _coeffs.length - 1 */ ) ? min + 1 : min;
        double delta = std::abs( min - aVowel );

        int l = 11; /* ARRAY_SIZE( _currentCoeffs )*/

        for ( int i = 0; i < l; ++i )
        {
            double minCoeff = _coeffs[ min ][ i ];
            double maxCoeff = _coeffs[ max ][ i ];

            _currentCoeffs[ i ] = delta < .5 ? minCoeff : maxCoeff;
        }
    }
}

void FormantFilter::process( AudioBuffer* sampleBuffer, bool isMonoSource )
{
    int bufferSize = sampleBuffer->bufferSize;

    for ( int c = 0, ca = sampleBuffer->amountOfChannels; c < ca; ++c )
    {
        SAMPLE_TYPE* channelBuffer = sampleBuffer->getBufferForChannel( c );

        for ( int i = 0; i < bufferSize; ++i )
        {
            double res = ( _currentCoeffs[ 0 ]  * channelBuffer[ i ] +
                           _currentCoeffs[ 1 ]  * _memory[ 0 ] +
                           _currentCoeffs[ 2 ]  * _memory[ 1 ] +
                           _currentCoeffs[ 3 ]  * _memory[ 2 ] +
                           _currentCoeffs[ 4 ]  * _memory[ 3 ] +
                           _currentCoeffs[ 5 ]  * _memory[ 4 ] +
                           _currentCoeffs[ 6 ]  * _memory[ 5 ] +
                           _currentCoeffs[ 7 ]  * _memory[ 6 ] +
                           _currentCoeffs[ 8 ]  * _memory[ 7 ] +
                           _currentCoeffs[ 9 ]  * _memory[ 8 ] +
                           _currentCoeffs[ 10 ] * _memory[ 9 ] );

            _memory[ 9 ] = _memory[ 8 ];
            _memory[ 8 ] = _memory[ 7 ];
            _memory[ 7 ] = _memory[ 6 ];
            _memory[ 6 ] = _memory[ 5 ];
            _memory[ 5 ] = _memory[ 4 ];
            _memory[ 4 ] = _memory[ 3 ];
            _memory[ 3 ] = _memory[ 2 ];
            _memory[ 2 ] = _memory[ 1 ];
            _memory[ 1 ] = _memory[ 0 ];
            _memory[ 0 ] = res;

            // 32-bit float resolution is too low on certain coefficients, below "hack"
            // cheaply prevents extreme self oscillation to exceed the max. capacity

            #if PRECISION == 1
                float output;

                if ( res > FLT_MAX )
                    output = FLT_MAX;
                else if ( res < -FLT_MAX )
                    output = -FLT_MAX;
                else
                    output = ( float ) res;

                channelBuffer[ i ] = output;
            #else
                channelBuffer[ i ] = res;
            #endif
        }

        // omit unnecessary cycles by copying the mono content
        if ( isMonoSource )
        {
            sampleBuffer->applyMonoSource();
            break;
        }
    }
}

bool FormantFilter::isCacheable()
{
    return true;
}

/* private methods */

// store the vowel coefficients

void FormantFilter::calculateCoeffs()
{
    // vowel "A"

    _coeffs[ VOWEL_A ][ 0 ]  = 8.11044e-06;
    _coeffs[ VOWEL_A ][ 1 ]  = 8.943665402;
    _coeffs[ VOWEL_A ][ 2 ]  = -36.83889529;
    _coeffs[ VOWEL_A ][ 3 ]  = 92.01697887;
    _coeffs[ VOWEL_A ][ 4 ]  = -154.337906;
    _coeffs[ VOWEL_A ][ 5 ]  = 181.6233289;
    _coeffs[ VOWEL_A ][ 6 ]  = -151.8651235;
    _coeffs[ VOWEL_A ][ 7 ]  = 89.09614114;
    _coeffs[ VOWEL_A ][ 8 ]  = -35.10298511;
    _coeffs[ VOWEL_A ][ 9 ]  = 8.388101016;
    _coeffs[ VOWEL_A ][ 10 ] = -0.923313471;

    // vowel "E"

    _coeffs[ VOWEL_E ][ 0 ]  = 4.36215e-06;
    _coeffs[ VOWEL_E ][ 1 ]  = 8.90438318;
    _coeffs[ VOWEL_E ][ 2 ]  = -36.55179099;
    _coeffs[ VOWEL_E ][ 3 ]  = 91.05750846;
    _coeffs[ VOWEL_E ][ 4 ]  = -152.422234;
    _coeffs[ VOWEL_E ][ 5 ]  = 179.1170248;
    _coeffs[ VOWEL_E ][ 6 ]  = -149.6496211;
    _coeffs[ VOWEL_E ][ 7 ]  = 87.78352223;
    _coeffs[ VOWEL_E ][ 8 ]  = -34.60687431;
    _coeffs[ VOWEL_E ][ 9 ]  = 8.282228154;
    _coeffs[ VOWEL_E ][ 10 ] = -0.914150747;

    // vowel "I"

    _coeffs[ VOWEL_I ][ 0 ]  = 3.33819e-06;
    _coeffs[ VOWEL_I ][ 1 ]  = 8.893102966;
    _coeffs[ VOWEL_I ][ 2 ]  = -36.49532826;
    _coeffs[ VOWEL_I ][ 3 ]  = 90.96543286;
    _coeffs[ VOWEL_I ][ 4 ]  = -152.4545478;
    _coeffs[ VOWEL_I ][ 5 ]  = 179.4835618;
    _coeffs[ VOWEL_I ][ 6 ]  = -150.315433;
    _coeffs[ VOWEL_I ][ 7 ]  = 88.43409371;
    _coeffs[ VOWEL_I ][ 8 ]  = -34.98612086;
    _coeffs[ VOWEL_I ][ 9 ]  = 8.407803364;
    _coeffs[ VOWEL_I ][ 10 ] = -0.932568035;

    // vowel "O"

    _coeffs[ VOWEL_O ][ 0 ]  = 1.13572e-06;
    _coeffs[ VOWEL_O ][ 1 ]  = 8.994734087;
    _coeffs[ VOWEL_O ][ 2 ]  = -37.2084849;
    _coeffs[ VOWEL_O ][ 3 ]  = 93.22900521;
    _coeffs[ VOWEL_O ][ 4 ]  = -156.6929844;
    _coeffs[ VOWEL_O ][ 5 ]  = 184.596544;
    _coeffs[ VOWEL_O ][ 6 ]  = -154.3755513;
    _coeffs[ VOWEL_O ][ 7 ]  = 90.49663749;
    _coeffs[ VOWEL_O ][ 8 ]  = -35.58964535;
    _coeffs[ VOWEL_O ][ 9 ]  = 8.478996281;
    _coeffs[ VOWEL_O ][ 10 ] = -0.929252233;

    // vowel "U"

    _coeffs[ VOWEL_U ][ 0 ]  = 4.09431e-07;
    _coeffs[ VOWEL_U ][ 1 ]  = 8.997322763;
    _coeffs[ VOWEL_U ][ 2 ]  = -37.20218544;
    _coeffs[ VOWEL_U ][ 3 ]  = 93.11385476;
    _coeffs[ VOWEL_U ][ 4 ]  = -156.2530937;
    _coeffs[ VOWEL_U ][ 5 ]  = 183.7080141;
    _coeffs[ VOWEL_U ][ 6 ]  = -153.2631681;
    _coeffs[ VOWEL_U ][ 7 ]  = 89.59539726;
    _coeffs[ VOWEL_U ][ 8 ]  = -35.12454591;
    _coeffs[ VOWEL_U ][ 9 ]  = 8.338655623;
    _coeffs[ VOWEL_U ][ 10 ] = -0.910251753;
}
