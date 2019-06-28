/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2019 Igor Zinken - http://www.igorski.nl
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
#include "../global.h"
#include "debug.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

namespace MWEngine {

/**
 * structs used to parse WAV file headers
 * see http://soundfile.sapp.org/doc/WaveFormat/
 */
struct wav_header
{
    char fileType[4];           // "RIFF" = 0x46464952
    UINT32 fileSize;            // in bytes
    char format[4];             // "WAVE" = 0x45564157
    char formatName[4];         // "fmt " = 0x20746D66
    UINT32 formatLength;
    INT16 audioFormat;          // 1 == PCM, 3 == IEEE float, 6 = A-law, 7 = µ-law
    INT16 amountOfChannels;
    UINT32 sampleRate;
    UINT32 bytesPerSecond;
    INT16 blockAlign;
    INT16 bitsPerSample;
};

struct wav_header_data_chunk
{
    char ID[4];         // "data" = 0x61746164
    UINT32 size;
};

/* internal methods */

template <typename T>
inline void convert( T, AudioBuffer* buffer, FILE* fp, int sampleSize, SAMPLE_TYPE maxValue )
{
    T input;
    for ( int i = 0; i < buffer->bufferSize; ++i )
    {
        for ( int c = 0; c < buffer->amountOfChannels; ++c ) {
            fread( &input, ( size_t ) sampleSize, 1, fp );
            buffer->getBufferForChannel( c )[ i ] = (( SAMPLE_TYPE ) input ) / maxValue;
        }
    }
}

/* public methods */

waveFile WaveReader::fileToBuffer( std::string inputFile )
{
    FILE* fp;
    waveFile out = { ( unsigned int ) AudioEngineProps::SAMPLE_RATE, nullptr };

    fp = fopen( inputFile.c_str(), "rb" );

    if ( !fp ) {
        Debug::log( "WaveReader::Error could not open file '%s'", inputFile.c_str() );
        return out;
    }

    // get the WAV file properties

    wav_header header;
    fread( &header, sizeof( header ), 1, fp );

    // validate the WAV file header data

    if ( !strstr( header.fileType,   "RIFF" ) ||
         !strstr( header.format,     "WAVE" ) ||
         !strstr( header.formatName, "fmt " )) // yes, the trailing space belongs in there!
    {
        Debug::log( "WaveReader::Error not a valid WAVE file" );
        return out;
    }

#ifdef DEBUG

    Debug::log( "About to parse data for WAV file '%s'", inputFile.c_str() );
    Debug::log( "File size        : %d", header.fileSize );
    Debug::log( "Audio format     : %d", header.audioFormat );
    Debug::log( "Channel amount   : %d", header.amountOfChannels );
    Debug::log( "Sample rate      : %d", header.sampleRate );
    Debug::log( "Bytes per second : %d", header.bytesPerSecond );
    Debug::log( "Block align      : %d", header.blockAlign );
    Debug::log( "Bits per sample  : %d", header.bitsPerSample );

#endif

    // the data chunk can be offset in case the WAV file contains
    // meta data, seek for the data definition

    wav_header_data_chunk dataChunk;

    while ( true )
    {
        fread( &dataChunk, sizeof( dataChunk ), 1, fp );

        if ( *( unsigned int* ) &dataChunk.ID == 0x61746164 )
            break;

        // skip chunk data bytes

        if ( dataChunk.size > header.fileSize ) {

            // unless this occurs; when the reported data chunk size exceeds the fileSize reported
            // by the header, this is a clear indication that we're dealing with a corrupted file
            // header. "Rewind" fread position and attempt to read file by assuming next chunk
            // is data (in other words: assume no metadata is present in the header)

            Debug::log( "WaveReader::Warning data chunk parsing failure. Assuming header corruption, attempting to read data as-is" );

            fseek( fp, -( sizeof dataChunk ), SEEK_CUR );

            fread( dataChunk.ID,    sizeof( char ), 4, fp );
            fread( &dataChunk.size, sizeof( unsigned int ), 1, fp );

            break;
        }
        else {
            fseek( fp, dataChunk.size, SEEK_CUR );
        }

        if ( feof( fp )) {
            Debug::log( "WaveReader::Error could not find data chunk" );
            return out;
        }
    }

    unsigned int amountOfSamples = dataChunk.size * 8 / header.bitsPerSample;
    unsigned int bufferSize      = amountOfSamples / header.amountOfChannels;

    if ( amountOfSamples <= 0 ) {
        Debug::log( "WaveReader::Error could not find sample data" );
        return out;
    }

    int sampleSize = header.bitsPerSample / 8;
    int i;

    out.sampleRate = ( unsigned int ) header.sampleRate;
    out.buffer     = new AudioBuffer( header.amountOfChannels, bufferSize );

    // Read samples from WAV file and convert data into MWEngine AudioBuffer

    switch ( header.bitsPerSample ) {
        // by default we will treat files as 16-bit
        default:
            Debug::log( "WaveReader::Warning no support for %s-bit file. Treating as 16-bit", header.bitsPerSample );
        // 16-bit
        case 16:
            convert(
                ( short ) 0, out.buffer, fp, sampleSize,
                ( SAMPLE_TYPE ) ( header.audioFormat == 3 ? 1 : 32767 )
            );
            break;

        // 24-bit (via char array conversion as there is no 24-bit data type)
        case 24:
            unsigned char input[ 3 ];
            long output;
            for ( int i = 0; i < bufferSize; ++i )
            {
                for ( int c = 0; c < header.amountOfChannels; ++c ) {
                    fread( &input, sizeof( char ), 3, fp );
                    // note that RIFF files are little endian
                    if ( input[ 2 ] & 0x80 ) {
                        // value is negative
                        output = ( 0xff << 24 ) | ( input[ 2 ] << 16 ) | ( input[ 1 ] << 8 ) | ( input[ 0 ] << 0 );
                    } else {
                        output = ( input[ 2 ] << 16 ) | ( input[ 1 ] << 8 ) | (input[ 0 ] << 0 );
                    }
                    out.buffer->getBufferForChannel( c )[ i ] = (( SAMPLE_TYPE ) output ) / 8388607.;
                }
            }
            break;

        // 32-bit
        case 32:
            convert(
                ( float ) 0, out.buffer, fp, sampleSize,
                ( SAMPLE_TYPE ) ( header.audioFormat == 3 ? 1 : 2147483647 )
            );
            break;

        // 64-bit
        case 64:
            convert(
                ( double ) 0, out.buffer, fp, sampleSize,
                ( SAMPLE_TYPE ) ( header.audioFormat == 3 ? 1 : 9223372036854775807 )
            );
            break;

        // 8-bit (note: 8-bit WAV files are unsigned)
        case 8:
            convert(( unsigned char ) 0, out.buffer, fp, sampleSize, ( SAMPLE_TYPE ) 255 );
            break;
    }

    // free allocated resources

    fclose( fp );

    return out;
}

WaveTable* WaveReader::fileToTable( std::string inputFile )
{
    waveFile WAV = fileToBuffer( inputFile );

    if ( WAV.buffer == nullptr ) {
        Debug::log( "WaveReader::could not convert file '%s' to WaveTable", inputFile.c_str() );
        return nullptr;
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

    AudioBuffer* buffer = nullptr;

    // first we'll check the necessary headers identifying the WAV file

    char id[ 5 ];
    short* temp_buffer;
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
            temp_buffer     = ( short* ) malloc( dataSize );

            Debug::log("WaveReader::reading %d channels w/ %d samples @ %d Hz", amountOfChannels, dataSize, sampleRate );

            // in Android NDK 26 reading of byte array is horribly broken...

            if ( !( dataSize > 1 )) {
                Debug::log( "WaveReader::Could not parse WAVE file" );
                waveFile out = { sampleRate, buffer };
                return out;
            }

            // 2 is sizeof short (16-bits, thus 2 bytes)

            for ( i = 44, l = i + dataSize, w = 0; i < l; i += 2, ++w )
                temp_buffer[ w ] = ( uint8_t ) byteArray[ i ] | (( uint8_t ) byteArray[ i + 1 ] << 8 );

            unsigned int bufferSize = ( dataSize / sizeof( short )) / amountOfChannels;

            buffer = new AudioBuffer( amountOfChannels, bufferSize );

            // convert short values into SAMPLE_TYPE

            SAMPLE_TYPE MAX_VALUE = ( SAMPLE_TYPE ) 32767; // max size for either side of a signed short

            int cb = 0;

            for ( i = 0; i < bufferSize; ++i, cb += amountOfChannels )
            {
                for ( int c = 0; c < amountOfChannels; ++c )
                    buffer->getBufferForChannel( c )[ i ] = ( SAMPLE_TYPE )( temp_buffer[ cb + c ]) / MAX_VALUE;
            }
            free( temp_buffer );
        }
        else
            Debug::log( "WaveReader::Error not a valid WAVE file" );
    }
    else
        Debug::log( "WaveReader::Error not a valid WAVE file (no RIFF header)" );

    waveFile out = { sampleRate, buffer };

    return out;
}

} // E.O namespace MWEngine
