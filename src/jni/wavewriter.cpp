/* Creates a WAV file from an array of ints.
 * Output is monophonic, signed 16-bit samples
 * copyright
 * Fri Jun 18 16:36:23 PDT 2010 Kevin Karplus
 * Creative Commons license Attribution-NonCommercial
 *  http://creativecommons.org/licenses/by-nc/3.0/
 */
#include "wavewriter.h"
#include "global.h"
#include "utils.h"
#include <fstream>

 void write_wav( std::string outputFile, unsigned long num_samples, short int * buffer, int sample_rate, int num_channels )
{
    unsigned int bytes_per_sample = sizeof( short int );
    const char* filename          = outputFile.c_str();

    writeWAVData( filename, buffer, bytes_per_sample * num_samples * num_channels, sample_rate, ( short ) num_channels );
}
