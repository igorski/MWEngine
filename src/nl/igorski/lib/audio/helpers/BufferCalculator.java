package nl.igorski.lib.audio.helpers;

/**
 * Created by IntelliJ IDEA.
 * User: igorzinken
 * Date: 4/12/12
 * Time: 3:29 PM
 * To change this template use File | Settings | File Templates.
 */
public final class BufferCalculator
{
    /* BufferCalculator provides several functions for basic time / BPM related calculations */
    /* NOTE: these are set for 4/4 bars ( TODO : extend for flexible time signatures )  */

    /* public */

    /**
     * get the tempo in Beats per Minute from the length
     * in milliseconds of a audio snippet
     *
     * @param length       {double} length in milliseconds
     * @param amountOfBars {int} the amount of bars the snippet lasts
     *
     * @return {double}
     */
    public static double getBPMbyLength( double length, int amountOfBars )
    {
        // length to seconds
        length *= .001;

        return 240 / ( length / amountOfBars );
    }

    /**
     * get the tempo in Beats Per Minute by the length in
     * samples of a audio snippet
     *
     * @param length       {int} length in samples
     * @param amountOfBars {int} the amount of bars the snippet lasts
     * @param sampleRate   {int} sampleRate in Hz
     *
     * @return {double}
     */
    public static double getBPMbySamples( int length, int amountOfBars, int sampleRate )
    {
         return 240 / (( length / amountOfBars ) / sampleRate );
    }

    /**
     * return the contents of a given buffer as its length in milliseconds
     *
     * @param {int} bufferSize
     * @param {int} sampleRate in Hz
     *
     * @return {int}
     */
    public static int bufferToMilliseconds( int bufferSize, int sampleRate )
    {
        return ( int ) ( bufferSize / ( sampleRate / 1000 ));
    }

    /**
     * return the required buffer size for a value in milliseconds
     *
     * @param {int} milliSeconds
     * @param {int} sampleRate in Hz
     *
     * @return {int}
     */
    public static int millisecondsToBuffer( int milliSeconds, int sampleRate )
    {
        return ( int ) ( milliSeconds * ( sampleRate / 1000 ));
    }

    /**
     * calculate the bitRate of a given audiostream
     *
     * @param sampleRate  {int} sampleRate in Hz
     * @param bitDepth    {int} bit depth
     * @param channels    {int} the amount of audio channels
     *
     * @return {double}
     */
    public static double getBitRate( int sampleRate, int bitDepth, int channels )
    {
        return sampleRate * bitDepth * channels;
    }

    /**
     * calculations within a musical context:
     * calculate the sample length required for each
     * beat at a given tempo at a given bar subdivision
     *
     * @param tempo       {double} tempo in BPM
     * @param sampleRate  {int} the sample rate in Hz
     * @param subdivision {int} the desired subdivision of the measure, i.e.
     *                    1 = full measure, 2 = half measure, 4 = quaver,
     *                    8 = 8th note, 16 = sixteenth note, 32 = 32nd note, etc.
     *
     * @return {int} amount of samples necessary for containing the buffer
     *               for the given parameters
     */
    public static int calculateSamplesPerBeatDivision( int sampleRate, double tempo, int subdivision )
    {
        final int bytesPerBar = getSamplesPerBar( sampleRate, tempo, 4, 4 );
        return ( int ) ( bytesPerBar / subdivision );
    }

    /**
     * calculate the amount of samples necessary to hold a buffer
     * representing a single beat at the given sample rate and tempo
     *
     * @param sampleRate {int} the sample rate in Hz
     * @param tempo      {double} tempo in BPM
     *
     * @return {int} amount of samples necessary for containing the buffer
     *               for the given parameters
     */
    public static int getSamplesPerBeat( int sampleRate, double tempo )
    {
        return ( int ) (( sampleRate * 60 ) / tempo );
    }

    /**
     * calculate the amount of samples necessary to hold a buffer
     * representing a full bar/measure at the given sample rate, tempo and time signature
     *
     * @param sampleRate {int} the sample rate in Hz
     * @param tempo      {double} tempo in BPM
     * @param beatAmount {int} upper numeral in time signature (i.e. the "3" in 3/4)
     * @param beatUnit   {int} lower numeral in time signature (i.e. the "4" in 3/4)
     *
     * @return {int} amount of samples necessary for containing the buffer
     *               for the given parameters
     */
    public static int getSamplesPerBar( int sampleRate, double tempo, int beatAmount, int beatUnit )
    {
        final int samplesPerDoubleFourTime = getSamplesPerBeat( sampleRate, tempo ) * 4;
        return samplesPerDoubleFourTime / beatUnit * beatAmount;
    }
}
