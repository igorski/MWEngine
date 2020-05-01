/**
 * The MIT License (MIT)
 *
 * A reverberator using Schroeder-Moorer filtered feedback comb filtering
 *
 * Based on freeverb by Jezar at Dreampoint (June 2000)
 * Ported by Robert Avellar (@robtize) to MWEngine (October 2019)
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
#ifndef __MWENGINE__REVERB_SM_H_INCLUDED__
#define __MWENGINE__REVERB_SM_H_INCLUDED__

#include "baseprocessor.h"
#include "../audiobuffer.h"

namespace MWEngine {

// inner classes for allPass and comb filtering

class AllPass
{
    public:
        AllPass();
        void setBuffer( SAMPLE_TYPE *buf, int size );
        inline SAMPLE_TYPE process( SAMPLE_TYPE input )
        {
            SAMPLE_TYPE output;
            SAMPLE_TYPE bufout = _buffer[ _bufIndex ];
            undenormalise( bufout );
        
            output = -input + bufout;
            _buffer[ _bufIndex ] = input + ( bufout * _feedback );
        
            if ( ++_bufIndex >= _bufSize ) {
                _bufIndex = 0;
            }
            return output;
        }
        void mute();
        SAMPLE_TYPE getFeedback();
        void setFeedback( SAMPLE_TYPE val );

    private:
        SAMPLE_TYPE  _feedback;
        SAMPLE_TYPE* _buffer;
        int _bufSize;
        int _bufIndex;
};

class Comb
{
    public:
        Comb();
        void setBuffer( SAMPLE_TYPE *buf, int size );
        inline SAMPLE_TYPE process( SAMPLE_TYPE input )
        {
            SAMPLE_TYPE output = _buffer[ _bufIndex ];
            undenormalise( output );
        
            _filterStore = ( output * _damp2 ) + ( _filterStore * _damp1 );
            undenormalise( _filterStore );
        
            _buffer[_bufIndex] = input + ( _filterStore * _feedback );
            if ( ++_bufIndex >= _bufSize ) {
                _bufIndex = 0;
            }
            return output;
        }
        void mute();
        SAMPLE_TYPE getDamp();
        void setDamp( SAMPLE_TYPE val );
        SAMPLE_TYPE getFeedback();
        void setFeedback( SAMPLE_TYPE val );

    private:
        SAMPLE_TYPE  _feedback;
        SAMPLE_TYPE  _filterStore;
        SAMPLE_TYPE  _damp1;
        SAMPLE_TYPE  _damp2;
        SAMPLE_TYPE* _buffer;
        int _bufSize;
        int _bufIndex;
};

// Reverb class

class ReverbSM : public BaseProcessor {

    static constexpr int NUM_COMBS             = 8;
    static constexpr int NUM_ALLPASSES         = 4;
    static constexpr SAMPLE_TYPE MUTED         = 0;
    static constexpr SAMPLE_TYPE FIXED_GAIN    = 0.015f;
    static constexpr SAMPLE_TYPE SCALE_WET     = 3;
    static constexpr SAMPLE_TYPE SCALE_DRY     = 2;
    static constexpr SAMPLE_TYPE SCALE_DAMP    = 0.4f;
    static constexpr SAMPLE_TYPE SCALE_ROOM    = 0.28f;
    static constexpr SAMPLE_TYPE OFFSET_ROOM   = 0.7f;
    static constexpr SAMPLE_TYPE INITIAL_ROOM  = 0.5f;
    static constexpr SAMPLE_TYPE INITIAL_DAMP  = 0.5f;
    static constexpr SAMPLE_TYPE INITIAL_WET   = 1 / SCALE_WET;
    static constexpr SAMPLE_TYPE INITIAL_DRY   = 0.5;
    static constexpr SAMPLE_TYPE INITIAL_WIDTH = 1;
    static constexpr SAMPLE_TYPE INITIAL_MODE  = 0;
    static constexpr SAMPLE_TYPE FREEZE_MODE   = 0.5f;
    static constexpr int STEREO_SPREAD         = 23;

    // These values assume 44.1 kHz sample rate and will work fine for 48 kHz samplerates
    // TODO: this will however not scale to future more hifi Android audio devices, consider
    // making these changeable at runtime with appropriate sample rate scaling

    static const int COMB_TUNING_L1    = 1116;
    static const int COMB_TUNING_R1    = COMB_TUNING_L1 + STEREO_SPREAD;
    static const int COMB_TUNING_L2    = 1188;
    static const int COMB_TUNING_R2    = COMB_TUNING_L2 + STEREO_SPREAD;
    static const int COMB_TUNING_L3    = 1277;
    static const int COMB_TUNING_R3    = COMB_TUNING_L3 + STEREO_SPREAD;
    static const int COMB_TUNING_L4    = 1356;
    static const int COMB_TUNING_R4    = COMB_TUNING_L4 + STEREO_SPREAD;
    static const int COMB_TUNING_L5    = 1422;
    static const int COMB_TUNING_R5    = COMB_TUNING_L5 + STEREO_SPREAD;
    static const int COMB_TUNING_L6    = 1491;
    static const int COMB_TUNING_R6    = COMB_TUNING_L6 + STEREO_SPREAD;
    static const int COMB_TUNING_L7    = 1557;
    static const int COMB_TUNING_R7    = COMB_TUNING_L7 + STEREO_SPREAD;
    static const int COMB_TUNING_L8    = 1617;
    static const int COMB_TUNING_R8    = COMB_TUNING_L8 + STEREO_SPREAD;
    static const int ALLPASS_TUNING_L1 = 556;
    static const int ALLPASS_TUNING_R1 = ALLPASS_TUNING_L1 + STEREO_SPREAD;
    static const int ALLPASS_TUNING_L2 = 441;
    static const int ALLPASS_TUNING_R2 = ALLPASS_TUNING_L2 + STEREO_SPREAD;
    static const int ALLPASS_TUNING_L3 = 341;
    static const int ALLPASS_TUNING_R3 = ALLPASS_TUNING_L3 + STEREO_SPREAD;
    static const int ALLPASS_TUNING_L4 = 225;
    static const int ALLPASS_TUNING_R4 = ALLPASS_TUNING_L4 + STEREO_SPREAD;

    public:
        ReverbSM();

        std::string getType() {
            return std::string( "ReverbSM" );
        }

        void mute();
        void setRoomSize( float value );
        float getRoomSize();
        void setDamp( float value );
        float getDamp();
        void setWet( float value );
        float getWet();
        void setDry( float value );
        float getDry();
        void setWidth( float value );
        float getWidth();
        void setMode( float value );
        float getMode();

#ifndef SWIG
        // internal to the engine
        void process( AudioBuffer* audioBuffer, bool isMonoSource );
#endif

    private:
        void update();
        float _gain;
        float _roomSize, _roomSize1;
        float _damp, _damp1;
        float _wet, _wet1, _wet2;
        float _dry;
        float _width;
        float _mode;
    
        // The following are all declared inline 
        // to remove the need for dynamic allocation
        // with its subsequent error-checking messiness
        // TODO: we will have to make these allocate dynamically when
        // scaling the tuning constants at runtime
    
        // Comb filters and their buffers

        Comb combL[ NUM_COMBS ];
        Comb combR[ NUM_COMBS ];

        SAMPLE_TYPE bufCombL1[ COMB_TUNING_L1 ];
        SAMPLE_TYPE bufCombR1[ COMB_TUNING_R1 ];
        SAMPLE_TYPE bufCombL2[ COMB_TUNING_L2 ];
        SAMPLE_TYPE bufCombR2[ COMB_TUNING_R2 ];
        SAMPLE_TYPE bufCombL3[ COMB_TUNING_L3 ];
        SAMPLE_TYPE bufCombR3[ COMB_TUNING_R3 ];
        SAMPLE_TYPE bufCombL4[ COMB_TUNING_L4 ];
        SAMPLE_TYPE bufCombR4[ COMB_TUNING_R4 ];
        SAMPLE_TYPE bufCombL5[ COMB_TUNING_L5 ];
        SAMPLE_TYPE bufCombR5[ COMB_TUNING_R5 ];
        SAMPLE_TYPE bufCombL6[ COMB_TUNING_L6 ];
        SAMPLE_TYPE bufCombR6[ COMB_TUNING_R6 ];
        SAMPLE_TYPE bufCombL7[ COMB_TUNING_L7 ];
        SAMPLE_TYPE bufCombR7[ COMB_TUNING_R7 ];
        SAMPLE_TYPE bufCombL8[ COMB_TUNING_L8 ];
        SAMPLE_TYPE bufCombR8[ COMB_TUNING_R8 ];

        // AllPass filters and their buffers

        AllPass allPassL[ NUM_ALLPASSES ];
        AllPass allPassR[ NUM_ALLPASSES ];
        SAMPLE_TYPE bufAllPassL1[ ALLPASS_TUNING_L1 ];
        SAMPLE_TYPE bufAllPassR1[ ALLPASS_TUNING_R1 ];
        SAMPLE_TYPE bufAllPassL2[ ALLPASS_TUNING_L2 ];
        SAMPLE_TYPE bufAllPassR2[ ALLPASS_TUNING_R2 ];
        SAMPLE_TYPE bufAllPassL3[ ALLPASS_TUNING_L3 ];
        SAMPLE_TYPE bufAllPassR3[ ALLPASS_TUNING_R3 ];
        SAMPLE_TYPE bufAllPassL4[ ALLPASS_TUNING_L4 ];
        SAMPLE_TYPE bufAllPassR4[ ALLPASS_TUNING_R4 ];
};
} // E.O namespace MWEngine

#endif
