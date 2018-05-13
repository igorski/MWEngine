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
#include "../audiobuffer.h"
#include "../wavetable.h"
#include <string>

typedef struct
{
   unsigned int sampleRate;
   AudioBuffer* buffer;
} waveFile;

class WaveReader
{
    public:

        // opens the File at the given path, reads its WAV data and
        // returns an AudioBuffer representing this file, NOTE : if
        // file doesn't exist / is not a valid WAV file, a null
        // pointer is returned
        static waveFile fileToBuffer( std::string inputFile );

        // opens the File at the given path, reads its WAV data and
        // returns an WaveTable holding this file's contents
        // NOTE : if file doesn't exist / is not a valid WAV file, a null
        // pointer is returned

        static WaveTable* fileToTable( std::string inputFile );

        static waveFile byteArrayToBuffer( std::vector<char> byteArray );
};
