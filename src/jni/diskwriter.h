#ifndef __DISKWRITER_H_INCLUDED__
#define __DISKWRITER_H_INCLUDED__

#include <string>

namespace DiskWriter
{
    // output directory to write to
    extern std::string outputDirectory;
    extern unsigned long outputBufferSize;
    extern unsigned long outputWriterIndex;
    extern short int* cachedBuffer;

    extern void prepareOutput( std::string aOutputDir, int aBufferSize );
    extern void generateOutputBuffer();
    extern void flushOutput();
    extern void appendBuffer( float* aBuffer, int aBufferLength );
    extern bool bufferFull();
    extern void writeBufferToFile( int aSampleRate, int aNumChannels, bool broadcastUpdate );
}

#endif
