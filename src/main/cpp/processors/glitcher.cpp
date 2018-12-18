/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2018 Igor Zinken - http://www.igorski.nl
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
#include "glitcher.h"
#include <utilities/bufferutility.h>

/* contructor / destructor */

Glitcher::Glitcher( int amountOfChannels, int fragmentLengthInMilliseconds )
{
    _buffer = new AudioBuffer( amountOfChannels,
                               BufferUtility::millisecondsToBuffer( fragmentLengthInMilliseconds,
                                                                    AudioEngineProps::SAMPLE_RATE ));

    _sample = new SampleEvent();
    _sample->setSample( _buffer );
    _sample->setLoopeable( true, false );

    _recording = false;
    _playback  = false;

    _writeOffset = 0;
    _readOffset  = 0;
}

Glitcher::~Glitcher()
{
    delete _sample;
    delete _buffer;
}

/* public methods */

void Glitcher::setRecording( bool value )
{
    _recording = value;
}

void Glitcher::setPlayback( bool value )
{
    _playback = value;
}

void Glitcher::setPlaybackRange( int bufferStartPos, int bufferEndPos )
{
    int maxBufferPosition = getSampleLength() - 1;

    _sample->setBufferRangeStart( std::min( bufferStartPos, maxBufferPosition ));
    _sample->setBufferRangeEnd  ( std::min( bufferEndPos,   maxBufferPosition ));
}

int Glitcher::getSampleLength()
{
    return _sample->getEventLength();
}

void Glitcher::process( AudioBuffer* sampleBuffer, bool isMonoSource )
{
    int sampleLength = getSampleLength();

    if ( _recording )
    {
        // record unmodified input into the buffer
        int sourceLength   = sampleBuffer->bufferSize;
        int writeLength    = sourceLength;
        int samplesAtStart = 0; // in case writes exceeds buffer size, append remainder at beginning of buffer

        // keep writes within the bounds of the buffer

        if (( _writeOffset + writeLength ) > sampleLength )
        {
            writeLength    = sampleLength - _writeOffset;
            samplesAtStart = sourceLength - writeLength;
        }

        for ( int c = 0, ca = std::min( sampleBuffer->amountOfChannels, _buffer->amountOfChannels ); c < ca; ++c )
        {
            SAMPLE_TYPE* srcBuffer    = sampleBuffer->getBufferForChannel( c );
            SAMPLE_TYPE* targetBuffer = _buffer->getBufferForChannel( c );

            int i, l, r;

            for ( i = _writeOffset, l = _writeOffset + writeLength, r = 0; i < l; ++i, ++r )
            {
                targetBuffer[ i ] = srcBuffer[ r ];
            }

            if ( samplesAtStart > 0 )
            {
                for ( i = 0; i < samplesAtStart; ++i, ++r )
                {
                    targetBuffer[ i ] = srcBuffer[ r ];
                }
            }
        }

        // update write pointer and keep in range
        if (( _writeOffset += sourceLength ) >= sampleLength )
            _writeOffset -= sampleLength;
    }

    if ( _playback )
    {
        sampleBuffer->silenceBuffers(); // mute existing contents as they will be replaced
        _sample->getBufferForRange( sampleBuffer, _readOffset );

        // update read pointer and keep in range
        if (( _readOffset += sampleBuffer->bufferSize ) >= sampleLength )
            _readOffset -= sampleLength;
    }
}
