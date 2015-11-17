/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Igor Zinken - http://www.igorski.nl
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
#include <stdio.h>

AudioBuffer* WaveReader::fileToBuffer( std::string inputFile )
{ 
   FILE* fp;
   AudioBuffer* out = 0;

   fp = fopen( inputFile.c_str(), "rb" );

   if ( fp )
   {
       char id[ 5 ];
       unsigned long size;
       short* sound_buffer;
       short tag, amountOfChannels, blcokAlign, bps;
       unsigned int format, sampleRate, bytesPerSec, dataSize, i;

       // first we'll check the necessary headers identifying the WAV file

       fread( id, sizeof( char ), 4, fp );
       id[ 4 ] = '\0';

      if ( !strcmp( id, "RIFF" ))
      {
            fread( &size, sizeof( unsigned long ),  1, fp );
            fread( id,    sizeof( unsigned char ), 4, fp );
            id[ 4 ] = '\0';

            if ( !strcmp( id, "WAVE" ))
            {
                fread( id,                sizeof( char ), 4, fp );
                fread( &format,           sizeof( unsigned long ), 1, fp );
                fread( &tag,              sizeof( short ), 1, fp );
                fread( &amountOfChannels, sizeof( short ), 1, fp );
                fread( &sampleRate,       sizeof( unsigned long ), 1, fp );
                fread( &bytesPerSec,      sizeof( unsigned long ), 1, fp );
                fread( &blcokAlign,       sizeof( short ), 1, fp );
                fread( &bps,              sizeof( short ), 1, fp );
                fread( id,                sizeof( char ), 4, fp );
                fread( &dataSize,         sizeof( unsigned long ), 1, fp );

                sound_buffer = ( short* ) malloc( dataSize );

                fread( sound_buffer, sizeof( short ), dataSize / sizeof( short ), fp );
                unsigned long bufferSize = ( dataSize / sizeof( short )) / amountOfChannels;

                out = new AudioBuffer( amountOfChannels, bufferSize );

                // convert short values into SAMPLE_TYPE

                SAMPLE_TYPE MAX_VALUE = ( SAMPLE_TYPE ) 32767; // max size for either side of a signed short

                int cb = 0;

                for ( i = 0; i < bufferSize; ++i, cb += amountOfChannels )
                {
                    for ( int c = 0; c < amountOfChannels; ++c ) {
                        out->getBufferForChannel( c )[ i ] = ( SAMPLE_TYPE )( sound_buffer[ cb + c ]) / MAX_VALUE;
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
    return out;
 }
