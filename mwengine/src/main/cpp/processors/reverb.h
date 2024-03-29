/**
 * Ported from mdaAmbienceProcessor.h
 * Created by Arne Scheffler on 6/13/08.
 *
 * mda VST Plug-ins
 *
 * Copyright (c) 2008 Paul Kellett
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of
 * the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
 * SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#include <utilities/bufferutility.h>
#include "baseprocessor.h"
#include "../audiobuffer.h"

#ifndef __MWENGINE__REVERB_H_INCLUDED__
#define __MWENGINE__REVERB_H_INCLUDED__

namespace MWEngine {
class Reverb : public BaseProcessor {

    public:

        // all values are in 0-1 range

        Reverb( float size, float hfDamp, float mix, float output );
        ~Reverb();

        std::string getType() const {
            return std::string( "Reverb" );
        }

        int addedDurationInSamples() {
            return BufferUtility::millisecondsToBuffer(( int ) ( getSize() * 4000.f ), AudioEngineProps::SAMPLE_RATE );
        }

        float getSize();
        void setSize( float value );
        float getHFDamp();
        void setHFDamp( float value );
        float getMix();
        void setMix( float value );
        float getOutput();
        void setOutput( float value );

#ifndef SWIG
        // internal to the engine
        void process( AudioBuffer* audioBuffer, bool isMonoSource );
#endif

    protected:
        void recalculate();
        void clearBuffers();

        float _size;
        float _hfDamp;
        float _mix;
        float _output;

        SAMPLE_TYPE *buf1, *buf2, *buf3, *buf4;
        SAMPLE_TYPE fil, fbak, damp, wet, dry, size;
        int pos, den, rdy;
};
} // E.O namespace MWEngine

#endif
