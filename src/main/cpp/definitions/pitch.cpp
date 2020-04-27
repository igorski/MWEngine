/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2020 Igor Zinken - https://www.igorski.nl
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
#include "pitch.h"
#include <utilities/stringutility.h>

namespace MWEngine {

const double Pitch::OCTAVE[ NOTES_IN_OCTAVE ] = { C, C_SHARP, D, D_SHARP, E, F, F_SHARP, G, G_SHARP, A, A_SHARP, B };
const std::string Pitch::OCTAVE_SCALE[ NOTES_IN_OCTAVE ] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
const std::string Pitch::FLAT  = "b";
const std::string Pitch::SHARP = "#";

double Pitch::note( std::string aNote, int aOctave )
{
    size_t i;
    double freq;
    int enharmonic = 0;
    // ensure valid input String, note we only make the first character uppercase (as the declaration
    // of a flat (b) should remain lower case)
    std::string n = aNote.substr( 0, 1 ); // should be note
    std::string h = aNote.substr( 1 );    // should be optional enharmonic (b or #)
    StringUtility::toUpperCase( n );
    std::string note = n + h;

    // detect flat enharmonic
    i = note.find( FLAT );
    if ( i != std::string::npos ) {
        note = note.substr( i - 1, 1 );
        enharmonic = -1;
    }
    // detect sharp enharmonic
    i = note.find( SHARP );
    if ( i != std::string::npos ) {
        note = note.substr( i - 1, 1 );
        enharmonic = 1;
    }
    freq = getOctaveIndex( note, enharmonic );

    if ( aOctave == 4 ) {
        return freq;
    }
    else {
        // translate the pitches to the requested octave
        int d = aOctave - 4;
        int j = abs( d );

        for ( i = 0; i < j; ++i ) {
            if ( d > 0 )
                freq *= 2;
            else
                freq *= .5;
        }
        return freq;
    }
}

} // E.O. namespace MWEngine