/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Igor Zinken - http://www.igorski.nl
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
#ifndef __MWENGINE__WAVE_UTIL_H_INCLUDED__
#define __MWENGINE__WAVE_UTIL_H_INCLUDED__

namespace MWEngine {
namespace WaveUtil
{
    /**
     * convenience method to turn a buffer holding a bipolar
     * waveform into a unipolar waveform
     */
    inline void toUnipolar( SAMPLE_TYPE* buffer, int bufferSize )
    {
        SAMPLE_TYPE halfPhase = 0.5f;
        for ( int i = 0; i < bufferSize; ++i ) {
            buffer[ i ] = buffer[ i ] * halfPhase + halfPhase;
        }
    }

    /**
     * convenience method to turn a buffer holding a unipolar
     * waveform into a bipolar waveform
     */
    inline void toBipolar( SAMPLE_TYPE* buffer, int bufferSize )
    {
        for ( int i = 0; i < bufferSize; ++i ) {
            buffer[ i ] = buffer[ i ] * 2.0 - 1.0;
        }
    }

    /**
     * whether given buffer has no bipolar contents
     */
    inline bool isUnipolar( SAMPLE_TYPE* buffer, int bufferSize )
    {
        for ( int i = 0; i < bufferSize; ++i ) {
            if ( buffer[ i ] < 0 )
                return false;
        }
        return true;
    }

    /**
     * whether given buffer has bipolar contents
     */
    inline bool isBipolar( SAMPLE_TYPE* buffer, int bufferSize )
    {
        for ( int i = 0; i < bufferSize; ++i ) {
            if ( buffer[ i ] < 0 )
                return true;
        }
        return false;
    }
}
} // E.O namespace MWEngine

#endif
