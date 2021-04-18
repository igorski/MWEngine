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
#include "reverbsm.h"

namespace MWEngine {

AllPass::AllPass()
{
    _bufIndex = 0;
}

void AllPass::setBuffer( SAMPLE_TYPE *buf, int size )
{
    _buffer  = buf;
    _bufSize = size;
}

void AllPass::mute()
{
    for ( int i = 0; i < _bufSize; i++ ) {
        _buffer[ i ] = 0;
    }
}


SAMPLE_TYPE AllPass::getFeedback()
{
    return _feedback;
}

void AllPass::setFeedback( SAMPLE_TYPE val )
{
    _feedback = val;
}

Comb::Comb()
{
    _filterStore = 0;
    _bufIndex    = 0;
}

void Comb::setBuffer( SAMPLE_TYPE *buf, int size )
{
    _buffer  = buf;
    _bufSize = size;
}

void Comb::mute()
{
    for ( int i = 0; i < _bufSize; i++ ) {
        _buffer[ i ] = 0;
    }
}

SAMPLE_TYPE Comb::getDamp()
{
    return _damp1;
}

void Comb::setDamp( SAMPLE_TYPE val )
{
    _damp1 = val;
    _damp2 = 1 - val;
}

SAMPLE_TYPE Comb::getFeedback()
{
    return _feedback;
}

void Comb::setFeedback( SAMPLE_TYPE val )
{
    _feedback = val;
}

ReverbSM::ReverbSM()
{
    // Tie the components to their buffers
    combL[ 0 ].setBuffer( bufCombL1, COMB_TUNING_L1 );
    combR[ 0 ].setBuffer( bufCombR1, COMB_TUNING_R1 );
    combL[ 1 ].setBuffer( bufCombL2, COMB_TUNING_L2 );
    combR[ 1 ].setBuffer( bufCombR2, COMB_TUNING_R2 );
    combL[ 2 ].setBuffer( bufCombL3, COMB_TUNING_L3 );
    combR[ 2 ].setBuffer( bufCombR3, COMB_TUNING_R3 );
    combL[ 3 ].setBuffer( bufCombL4, COMB_TUNING_L4 );
    combR[ 3 ].setBuffer( bufCombR4, COMB_TUNING_R4 );
    combL[ 4 ].setBuffer( bufCombL5, COMB_TUNING_L5 );
    combR[ 4 ].setBuffer( bufCombR5, COMB_TUNING_R5 );
    combL[ 5 ].setBuffer( bufCombL6, COMB_TUNING_L6 );
    combR[ 5 ].setBuffer( bufCombR6, COMB_TUNING_R6 );
    combL[ 6 ].setBuffer( bufCombL7, COMB_TUNING_L7 );
    combR[ 6 ].setBuffer( bufCombR7, COMB_TUNING_R7 );
    combL[ 7 ].setBuffer( bufCombL8, COMB_TUNING_L8 );
    combR[ 7 ].setBuffer( bufCombR8, COMB_TUNING_R8 );
    allPassL[ 0 ].setBuffer( bufAllPassL1, ALLPASS_TUNING_L1 );
    allPassR[ 0 ].setBuffer( bufAllPassR1, ALLPASS_TUNING_R1 );
    allPassL[ 1 ].setBuffer( bufAllPassL2, ALLPASS_TUNING_L2 );
    allPassR[ 1 ].setBuffer( bufAllPassR2, ALLPASS_TUNING_R2 );
    allPassL[ 2 ].setBuffer( bufAllPassL3, ALLPASS_TUNING_L3 );
    allPassR[ 2 ].setBuffer( bufAllPassR3, ALLPASS_TUNING_R3 );
    allPassL[ 3 ].setBuffer( bufAllPassL4, ALLPASS_TUNING_L4 );
    allPassR[ 3 ].setBuffer( bufAllPassR4, ALLPASS_TUNING_R4 );
    
    // Set default values
    allPassL[ 0 ].setFeedback( 0.5f );
    allPassR[ 0 ].setFeedback( 0.5f );
    allPassL[ 1 ].setFeedback( 0.5f );
    allPassR[ 1 ].setFeedback( 0.5f );
    allPassL[ 2 ].setFeedback( 0.5f );
    allPassR[ 2 ].setFeedback( 0.5f );
    allPassL[ 3 ].setFeedback( 0.5f );
    allPassR[ 3 ].setFeedback( 0.5f );
    
    setWet     ( INITIAL_WET );
    setRoomSize( INITIAL_ROOM );
    setDry     ( INITIAL_DRY );
    setDamp    ( INITIAL_DAMP );
    setWidth   ( INITIAL_WIDTH );
    setMode    ( INITIAL_MODE );
    
    // this will initialize the buffers with silence
    mute();
}

void ReverbSM::mute()
{
    if ( getMode() >= FREEZE_MODE )
        return;
    
    for ( int i = 0; i < NUM_COMBS; i++ ) {
        combL[ i ].mute();
        combR[ i ].mute();
    }

    for ( int i = 0; i < NUM_ALLPASSES; i++ ) {
        allPassL[ i ].mute();
        allPassR[ i ].mute();
    }
}

void ReverbSM::process( AudioBuffer* audioBuffer, bool isMonoSource )
{
    int numSamples = audioBuffer->bufferSize;
    int skip = 1;

    SAMPLE_TYPE* inputL  = audioBuffer->getBufferForChannel( 0 );
    SAMPLE_TYPE* outputL = audioBuffer->getBufferForChannel( 0 );
    SAMPLE_TYPE* inputR  = !isMonoSource ? audioBuffer->getBufferForChannel( 1 ) : nullptr;
    SAMPLE_TYPE* outputR = !isMonoSource ? audioBuffer->getBufferForChannel( 1 ) : nullptr;

    SAMPLE_TYPE outL, outR, input;

    while ( numSamples-- > 0 )
    {
        outL  = outR = 0;
        input = ( isMonoSource ? ( *inputL ) : ( *inputL + *inputR )) * _gain;

        // Accumulate comb filters in parallel
        for ( int i = 0; i< NUM_COMBS; i++ ) {
            outL += combL[ i ].process( input );
            outR += combR[ i ].process( input );
        }

        // Feed through allPasses in series
        for ( int i = 0; i < NUM_ALLPASSES; i++ ) {
            outL = allPassL[ i ].process( outL );
            outR = allPassR[ i ].process( outR );
        }

        // Calculate output REPLACING anything already there
        *outputL = outL * _wet1 + outR * _wet2 + *inputL * _dry;

        if ( !isMonoSource ) {
            *outputR = outR * _wet1 + outL * _wet2 + *inputR * _dry;
        }

        // Increment sample pointers, allowing for interleave (if any)
        inputL  += skip;
        outputL += skip;

        if ( !isMonoSource ) {
            inputR  += skip;
            outputR += skip;
        }
    }

}

void ReverbSM::update()
{
    // Recalculate internal values after parameter change

    _wet1 = _wet * ( _width / 2 + 0.5f );
    _wet2 = _wet * (( 1 - _width ) / 2 );
    
    if ( _mode >= FREEZE_MODE ){
        _roomSize1 = 1;
        _damp1     = 0;
        _gain      = MUTED;
    }
    else {
        _roomSize1 = _roomSize;
        _damp1     = _damp;
        _gain      = FIXED_GAIN;
    }
    
    for ( int i = 0; i < NUM_COMBS; i++ ) {
        combL[ i ].setFeedback( _roomSize1 );
        combR[ i ].setFeedback( _roomSize1 );
    }
    
    for ( int i = 0; i < NUM_COMBS; i++ ) {
        combL[ i ].setDamp( _damp1 );
        combR[ i ].setDamp( _damp1 );
    }
}

void ReverbSM::setRoomSize( float value )
{
    _roomSize = ( value * SCALE_ROOM ) + OFFSET_ROOM;
    update();
}

float ReverbSM::getRoomSize()
{
    return ( _roomSize - OFFSET_ROOM ) / SCALE_ROOM;
}

void ReverbSM::setDamp( float value )
{
    _damp = value * SCALE_DAMP;
    update();
}

float ReverbSM::getDamp()
{
    return _damp / SCALE_DAMP;
}

void ReverbSM::setWet( float value )
{
    _wet = value * SCALE_WET;
    update();
}

float ReverbSM::getWet()
{
    return _wet / SCALE_WET;
}

void ReverbSM::setDry( float value )
{
    _dry = value * SCALE_DRY;
}

float ReverbSM::getDry()
{
    return _dry / SCALE_DRY;
}

void ReverbSM::setWidth( float value )
{
    _width = value;
    update();
}

float ReverbSM::getWidth()
{
    return _width;
}

void ReverbSM::setMode( float value )
{
    _mode = value;
    update();
}

float ReverbSM::getMode()
{
    return ( _mode >= FREEZE_MODE ) ? 1 : 0;
}
} // E.O namespace MWEngine
