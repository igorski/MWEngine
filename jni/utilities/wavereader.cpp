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
#include "wavereader.h"
#include "debug.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

waveFile WaveReader::fileToBuffer( std::string inputFile )
{ 
    FILE* fp;
    AudioBuffer* buffer = 0;
    char id[ 5 ];
    unsigned int size;
    short* sound_buffer;
    short tag, amountOfChannels, blockAlign, bps;
    unsigned int format, sampleRate, bytesPerSec, dataSize, i;

    fp = fopen( inputFile.c_str(), "rb" );

    if ( fp )
    {
        // first we'll check the necessary headers identifying the WAV file

        fread( id, sizeof( char ), 4, fp );
        id[ 4 ] = '\0';

        if ( !strcmp( id, "RIFF" ))
        {
            fread( &size, sizeof( unsigned int ),  1, fp );
            fread( id,    sizeof( unsigned char ), 4, fp );
            id[ 4 ] = '\0';

            if ( !strcmp( id, "WAVE" ))
            {
                // read the header data (see http://soundfile.sapp.org/doc/WaveFormat/)

                fread( id,                sizeof( char ), 4, fp );
                fread( &format,           sizeof( unsigned int ), 1, fp );
                fread( &tag,              sizeof( short ), 1, fp );
                fread( &amountOfChannels, sizeof( short ), 1, fp );
                fread( &sampleRate,       sizeof( unsigned int ), 1, fp );
                fread( &bytesPerSec,      sizeof( unsigned int ), 1, fp );
                fread( &blockAlign,       sizeof( short ), 1, fp );
                fread( &bps,              sizeof( short ), 1, fp );
                fread( id,                sizeof( char ), 4, fp );
                fread( &dataSize,         sizeof( unsigned int ), 1, fp );

                sound_buffer = ( short* ) malloc( dataSize );

                fread( sound_buffer, sizeof( short ), dataSize / sizeof( short ), fp );
                unsigned int bufferSize = ( dataSize / sizeof( short )) / amountOfChannels;

                buffer = new AudioBuffer( amountOfChannels, bufferSize );

                // convert short values into SAMPLE_TYPE

                SAMPLE_TYPE MAX_VALUE = ( SAMPLE_TYPE ) 32767; // max size for either side of a signed short

                int cb = 0;

                for ( i = 0; i < bufferSize; ++i, cb += amountOfChannels )
                {
                    for ( int c = 0; c < amountOfChannels; ++c ) {
                        buffer->getBufferForChannel( c )[ i ] = ( SAMPLE_TYPE )( sound_buffer[ cb + c ]) / MAX_VALUE;
                    }
                }
                free( sound_buffer );
            }
            else
                Debug::log( "WaveReader::Error not a valid WAVE file" );
        }
        else
            Debug::log( "WaveReader::Error not a valid WAVE file (no RIFF header)" );

        fclose( fp );
    }
    else {
        Debug::log( "WaveReader::Error could not open file '%s'", inputFile.c_str() );
    }

    waveFile out = { sampleRate, buffer };

    return out;
}

WaveTable* WaveReader::fileToTable( std::string inputFile )
{
    waveFile WAV = fileToBuffer( inputFile );

    if ( WAV.buffer == 0 ) {
        Debug::log( "WaveReader::could not convert file '%s' to WaveTable", inputFile.c_str() );
        return 0;
    }

    WaveTable* out = new WaveTable( WAV.buffer->bufferSize, 440.0f );

    // clone the left/mono channel of the temporary buffer and
    // assign the clone to the WaveTable

    SAMPLE_TYPE* targetBuffer = new SAMPLE_TYPE[ WAV.buffer->bufferSize ];
    memcpy( targetBuffer, WAV.buffer->getBufferForChannel( 0 ), WAV.buffer->bufferSize * sizeof( SAMPLE_TYPE ));
    out->setBuffer( targetBuffer );

    delete WAV.buffer; // free memory used by the temporary buffer

    return out;
}

waveFile WaveReader::byteArrayToBuffer( std::vector<char> byteArray )
{
    // TODO: this mostly mirrors the fileToBuffer-method, clean this up!

    AudioBuffer* buffer = 0;

    // first we'll check the necessary headers identifying the WAV file

    char id[ 5 ];
    short* sound_buffer;
    short amountOfChannels;
    unsigned int format, sampleRate, dataSize, i, l, w;

    sliceString( byteArray, id, 0, 4 );
    id[ 4 ] = '\0';

    if ( !strcmp( id, "RIFF" ))
    {
        sliceString( byteArray, id, 8, 4 );
        id[ 4 ] = '\0';

        if ( !strcmp( id, "WAVE" ))
        {
            // read the header data (see http://soundfile.sapp.org/doc/WaveFormat/)

            format           = sliceLong( byteArray, 20, true );
            amountOfChannels = ( short ) sliceLong( byteArray, 22, true );
            sampleRate       = sliceLong( byteArray, 24, true );
            dataSize         = sliceLong( byteArray, 40, true );
            sound_buffer     = ( short* ) malloc( dataSize );

            Debug::log("WaveReader::reading %d channels w/ %d samples @ %d Hz", amountOfChannels, dataSize, sampleRate );

            // in Android NDK 26 reading of byte array is horribly broken...

            if ( !( dataSize > 1 )) {
                Debug::log( "WaveReader::Could not parse WAVE file" );
                waveFile out = { sampleRate, buffer };
                return out;
            }

            // 2 is sizeof short

            for ( i = 44, l = i + dataSize, w = 0; i < l; i += 2, ++w )
                sound_buffer[ w ] = ( uint8_t ) byteArray[ i ] | (( uint8_t ) byteArray[ i + 1 ] << 8 );

            unsigned int bufferSize = ( dataSize / sizeof( short )) / amountOfChannels;

            buffer = new AudioBuffer( amountOfChannels, bufferSize );

            // convert short values into SAMPLE_TYPE

            SAMPLE_TYPE MAX_VALUE = ( SAMPLE_TYPE ) 32767; // max size for either side of a signed short

            int cb = 0;

            for ( i = 0; i < bufferSize; ++i, cb += amountOfChannels )
            {
                for ( int c = 0; c < amountOfChannels; ++c )
                    buffer->getBufferForChannel( c )[ i ] = ( SAMPLE_TYPE )( sound_buffer[ cb + c ]) / MAX_VALUE;
            }
            free( sound_buffer );
        }
        else
            Debug::log( "WaveReader::Error not a valid WAVE file" );
    }
    else
        Debug::log( "WaveReader::Error not a valid WAVE file (no RIFF header)" );

    waveFile out = { sampleRate, buffer };

    return out;
}
