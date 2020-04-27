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
#ifndef __MWENGINE__PITCH_H_INCLUDED__
#define __MWENGINE__PITCH_H_INCLUDED__

#include <string>

namespace MWEngine {
/**
 * Pitch is a helper class where musical notes can be calculated
 * to their frequencies in Hz or as MIDI notes
 *
 * initial variables set @ 4th octave where "middle C" resides
 * these are used for calculating requested notes, we differentiate
 * between whole-notes and enharmonic notes by using 'sharp' ( while
 * these might not be "musically correct" in regard to scales and theory,
 * the returned frequencies are the same!
 *
 * note: we calculate from the 4th octave as way of moving from "center"
 * pitches outward ( to lower / higher ranges ) as the changes in Hz feature
 * slight deviations, which would become more apparent by calculating powers of n.
 */
class Pitch {
    public:
        static constexpr double C                = 261.626;
        static constexpr double C_SHARP          = 277.183;
        static constexpr double D                = 293.665;
        static constexpr double D_SHARP          = 311.127;
        static constexpr double E                = 329.628;
        static constexpr double F                = 349.228;
        static constexpr double F_SHARP          = 369.994;
        static constexpr double G                = 391.995;
        static constexpr double G_SHARP          = 415.305;
        static constexpr double A                = 440;
        static constexpr double A_SHARP          = 466.164;
        static constexpr double B                = 493.883;

        /**
         * generates the frequency in Hz corresponding to the given note at the given octave
         *
         * @param aNote   {String} musical note to return ( A, B, C, D, E, F, G with
         *                possible enharmonic notes ( 'b' meaning 'flat', '#' meaning 'sharp' )
         *                NOTE: flats are CASE sensitive ( to prevent seeing the note 'B' instead of 'b' )
         * @param aOctave {int} the octave to return ( accepted range 0 - 9 )
         *
         * @return {double} frequency in Hz for the requested note
         */
        static double note( std::string aNote, int aOctave );

    private:

        static const unsigned int NOTES_IN_OCTAVE = 12;
        static const double OCTAVE[ NOTES_IN_OCTAVE ];

        static const std::string OCTAVE_SCALE[ NOTES_IN_OCTAVE ];
        static const std::string FLAT;
        static const std::string SHARP;

        /**
         * retrieves the index in the octave array for a given note
         * modifier enharmonic returns the previous ( for a 'flat' note )
         * or next ( for a 'sharp' note ) index
         *
         * @param note        {String} ( A, B, C, D, E, F, G )
         * @param enharmonic  {int}    ( 0, -1 for flat, 1 for sharp )
         *
         * @return {double}
         */
        inline static double getOctaveIndex( const std::string& note, int enharmonic )
        {
            for ( size_t i = 0; i < NOTES_IN_OCTAVE; ++i )
            {
                if ( OCTAVE_SCALE[ i ].compare( note ) == 0 ) {
                    int k = ( int ) i + enharmonic;

                    if ( k > NOTES_IN_OCTAVE )
                        return OCTAVE[ 0 ];

                    if ( k < 0 )
                        return OCTAVE[ NOTES_IN_OCTAVE - 1 ];

                    return OCTAVE[ k ];
                }
            }
            return 0;
        }
};
} // E.O. namespace MWEngine

#endif