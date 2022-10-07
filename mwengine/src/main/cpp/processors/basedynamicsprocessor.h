/**
 * The MIT License (MIT)
 * Copyright (c) 2013-2022 Igor Zinken - https://www.igorski.nl
 *
 * dynamics processing based off work by Chunkware
 *
 * (c) 2006, ChunkWare Music Software, OPEN-SOURCE
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#ifndef __MWENGINE_BASE_DYNAMICSPROCESSOR_H__
#define __MWENGINE_BASE_DYNAMICSPROCESSOR_H__

#include <algorithm>
#include <cmath>

namespace MWEngine {
class EnvelopeDetector
{
    public:
        EnvelopeDetector( float ms = 1.0, float sampleRate = 44100.0 );
        ~EnvelopeDetector() {}

        void setTimeConstant( float ms );
        float getTimeConstant( void ) const {
            return _timeConstant;
        }

        void setSampleRate( float sampleRate );
        float getSampleRate( void ) const {
            return _sampleRate;
        }

        inline void run( float in, float &state ) {
            state = in + _coefficient * ( state - in );
        }

    protected:
        float _sampleRate;
        float _timeConstant;
        float _coefficient;

        void cacheCoefficient();
};

class BaseDynamicsProcessor
{
    public:
        BaseDynamicsProcessor( float attackInMs = 10.0, float releaseInMs = 100.0, int sampleRate = 44100 );
        virtual ~BaseDynamicsProcessor() {}

        virtual float getAttack() const {
            return _attackEnvelope.getTimeConstant();
        }
        virtual void setAttack( float ms );

        virtual float getRelease() const {
            return _releaseEnvelope.getTimeConstant();
        }
        virtual void setRelease( float ms );

        virtual float getSampleRate() const {
            return _attackEnvelope.getSampleRate();
        }
        virtual void setSampleRate( int sampleRate );

        inline void run( float in, float &state ) {
            if ( in > state ) {
                _attackEnvelope.run( in, state ); // attack phase
            } else {
                _releaseEnvelope.run( in, state ); // release phase
            }
        }

    private:
        EnvelopeDetector _attackEnvelope;
        EnvelopeDetector _releaseEnvelope;
};

} // E.O. namespace MWEngine

#endif
