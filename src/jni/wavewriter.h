/* make_wav.h
 * Fri Jun 18 17:06:02 PDT 2010 Kevin Karplus
 */

#ifndef __WAVEWRITER_H_INCLUDED__
#define __WAVEWRITER_H_INCLUDED__

#include <string>
#include <fstream>

/* open a file named filename, write signed 16-bit values as a
 * monoaural WAV file at the specified sampling rate
 * and close the file
*/
void write_wav( std::string outputFile, unsigned long num_samples, short int * buffer, int sample_rate, int num_channels );

template <typename T>
void t_streamwrite( std::ofstream& stream, const T& t ) {
    stream.write((const char*)&t, sizeof(T));
}

template <typename SampleType>
void writeWAVData( const char* outFile, SampleType* buf, size_t bufSize,
                   int sampleRate, short channels )
{
    std::ofstream stream(outFile, std::ios::binary);
    stream.write("RIFF", 4);
    t_streamwrite<int>(stream, 36 + bufSize);
    stream.write("WAVE", 4);
    stream.write("fmt ", 4);
    t_streamwrite<int>(stream, 16);
    t_streamwrite<short>(stream, 1);                                        // Format (1 = PCM)
    t_streamwrite<short>(stream, channels);                                 // Channels
    t_streamwrite<int>(stream, sampleRate);                                 // Sample Rate
    t_streamwrite<int>(stream, sampleRate * channels * sizeof(SampleType)); // Byterate
    t_streamwrite<short>(stream, channels * sizeof(SampleType));            // Frame size
    t_streamwrite<short>(stream, 8 * sizeof(SampleType));                   // Bits per sample
    stream.write("data", 4);
    stream.write((const char*)&bufSize, 4);
    stream.write((const char*)buf, bufSize);
}

#endif