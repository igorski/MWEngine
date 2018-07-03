/**
 * Ported from mdaReverb.h
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
#include "reverb.h"
#include <algorithm>
#include <cstring>
#include <utilities/utils.h>

/* constructor / destructor */

Reverb::Reverb( float size, float hfDamp, float mix, float output )
{
    _size   = size;
    _hfDamp = hfDamp;
    _mix    = mix;
    _output = output;

    buf1 = new SAMPLE_TYPE[ 1024 ];
    buf2 = new SAMPLE_TYPE[ 1024 ];
    buf3 = new SAMPLE_TYPE[ 1024 ];
    buf4 = new SAMPLE_TYPE[ 1024 ];

    fil = 0.0f;
    den = pos = 0;

    recalculate();
}

Reverb::~Reverb()
{
    if ( buf1 )
        delete[] buf1;

    if ( buf2 )
        delete[] buf2;

    if ( buf3 )
        delete[] buf3;

    if ( buf4 )
        delete[] buf4;

    buf1 = buf2 = buf3 = buf4 = 0;
}

/* public methods */

float Reverb::getSize()
{
    return _size;
}

void Reverb::setSize( float value )
{
    value = capParam( value );

    if ( _size != value ) {
        _size = value;
        recalculate();
    }
}

float Reverb::getHFDamp()
{
    return _hfDamp;
}

void Reverb::setHFDamp( float value )
{
    value = capParam( value );

    if ( _hfDamp != value ) {
        _hfDamp = value;
        recalculate();
    }
}

float Reverb::getMix()
{
    return _mix;
}

void Reverb::setMix( float value )
{
    value = capParam( value );

    if ( _mix != value ) {
        _mix = value;
        recalculate();
    }
}

float Reverb::getOutput()
{
    return _output;
}

void Reverb::setOutput( float value )
{
    value = capParam( value );

    if ( _output != value ) {
        _output = value;
        recalculate();
    }
}

void Reverb::process( AudioBuffer* audioBuffer, bool isMonosource )
{
    int sampleFrames = audioBuffer->bufferSize;

    SAMPLE_TYPE* in1  = audioBuffer->getBufferForChannel( 0 );
    SAMPLE_TYPE* in2  = audioBuffer->getBufferForChannel( 1 );
    SAMPLE_TYPE* out1 = audioBuffer->getBufferForChannel( 0 );
    SAMPLE_TYPE* out2 = audioBuffer->getBufferForChannel( 1 );

    SAMPLE_TYPE a, b, r;
    SAMPLE_TYPE t, f = fil, fb = fbak, dmp = damp, y = dry, w = wet;
    int  p = pos, d1, d2, d3, d4;

    if ( rdy == 0 )
        clearBuffers();

    d1 = ( p + ( int )( 107 * size )) & 1023;
    d2 = ( p + ( int )( 142 * size )) & 1023;
    d3 = ( p + ( int )( 277 * size )) & 1023;
    d4 = ( p + ( int )( 379 * size )) & 1023;

    --in1;
    --in2;
    --out1;
    --out2;

    while ( --sampleFrames >= 0 )
    {
        a = *++in1;
        b = *++in2;

        f += dmp * (w * (a + b) - f); // HF damping
        r = f;

        t = *(buf1 + p);
        r -= fb * t;
        *(buf1 + d1) = r; // allpass
        r += t;

        t = *(buf2 + p);
        r -= fb * t;
        *(buf2 + d2) = r; // allpass
        r += t;

        t = *(buf3 + p);
        r -= fb * t;
        *(buf3 + d3) = r; // allpass
        r += t;
        a = y * a + r - f; // left output

        t = *(buf4 + p);
        r -= fb * t;
        *(buf4 + d4) = r; // allpass
        r += t;
        b = y * b + r - f; // right output

        ++p  &= 1023;
        ++d1 &= 1023;
        ++d2 &= 1023;
        ++d3 &= 1023;
        ++d4 &= 1023;

        *++out1 = a;
        *++out2 = b;
    }
    pos = p;

    // catch denormals

    if ( fabs( f ) > 1.0e-10 ) {
        fil = f;
        den = 0;
    }
    else {
        fil = 0.0f;
        if ( den == 0 ) {
            den = 1;
            clearBuffers();
        }
    }
}

/* protected methods */

void Reverb::clearBuffers()
{
    memset( buf1, 0, 1024 * sizeof( SAMPLE_TYPE ));
    memset( buf2, 0, 1024 * sizeof( SAMPLE_TYPE ));
    memset( buf3, 0, 1024 * sizeof( SAMPLE_TYPE ));
    memset( buf4, 0, 1024 * sizeof( SAMPLE_TYPE ));

    rdy = 1;
}

void Reverb::recalculate()
{
    SAMPLE_TYPE tmp;

    fbak = 0.8f;
    damp = 0.05f + 0.9f * _hfDamp;
    tmp = ( SAMPLE_TYPE ) powf( 10.0f, 2.0f * _output - 1.0f );
    dry = tmp - _mix * _mix * tmp;
    wet = ( 0.4f + 0.4f ) * _mix * tmp;

    tmp = 0.025f + 2.665f * _size;

    if ( size != tmp )
        rdy = 0; // need to flush buffer

    size = tmp;
}
