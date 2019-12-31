/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2019 Igor Zinken - http://www.igorski.nl
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
package nl.igorski.mwengine.definitions;

public final class Pitch
{
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

    public static final double C                = 261.626;
    public static final double C_SHARP          = 277.183;
    public static final double D                = 293.665;
    public static final double D_SHARP          = 311.127;
    public static final double E                = 329.628;
    public static final double F                = 349.228;
    public static final double F_SHARP          = 369.994;
    public static final double G                = 391.995;
    public static final double G_SHARP          = 415.305;
    public static final double A                = 440;
    public static final double A_SHARP          = 466.164;
    public static final double B                = 493.883;

    public static final double[] OCTAVE         = { C, C_SHARP, D, D_SHARP, E, F, F_SHARP, G, G_SHARP, A, A_SHARP, B };
    public static final String[] OCTAVE_SCALE   = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

    public static final String FLAT             = "b";
    public static final String SHARP            = "#";

    /* public */

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
    public static double note( String aNote, int aOctave )
    {
        int i;
        double freq;
        int enharmonic = 0;
        String note = aNote.substring(0, 1).toUpperCase() + aNote.substring(1);

        // detect flat enharmonic
        i = note.indexOf( FLAT );
        if ( i > -1 ) {
            note = note.substring( i - 1, 1 );
            enharmonic = -1;
        }
        // detect sharp enharmonic
        i = note.indexOf( SHARP );
        if ( i > -1 ) {
            note = note.substring( i - 1, 1 );
            enharmonic = 1;
        }
        freq = getOctaveIndex( note, enharmonic );

        if ( aOctave == 4 ) {
            return freq;
        }
        else {
            // translate the pitches to the requested octave
            int d = aOctave - 4;
            int j = Math.abs( d );

            for ( i = 0; i < j; ++i )
            {
                if ( d > 0 )
                    freq *= 2;
                else
                    freq *= .5;
            }
            return freq;
        }
    }

    /* private */

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
    private static double getOctaveIndex( String note, int enharmonic )
    {
        int j = OCTAVE.length;

        for ( int i = 0; i < j; ++i ) {
            if ( OCTAVE_SCALE[ i ].equals( note )) {
                int k = i + enharmonic;

                if ( k > j )
                    return OCTAVE[ 0 ];

                if ( k < 0 )
                    return OCTAVE[ OCTAVE.length - 1 ];

                return OCTAVE[ k ];
            }
        }
        return 0;
    }
}
