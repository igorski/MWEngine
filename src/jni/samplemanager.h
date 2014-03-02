#ifndef __SAMPLEMANAGER_H_INCLUDED__
#define __SAMPLEMANAGER_H_INCLUDED__

#include <string>
#include <map>
#include <utility>
#include <jni.h>

typedef struct
{
   int sampleLength;
   float* sampleBuffer;
} cachedSample;

class SampleManager
{
    public:

        static void setSample( std::string aKey, float* aBuffer, int aBufferLength );
        static void setSample( jstring aKey, jfloatArray aBuffer, jint aBufferLength ); // JNI interface

        static float* getSample( std::string aIdentifier );
        static int getSampleLength( std::string aIdentifier );

        static bool hasSample( std::string aIdentifier );
        static void removeSample( std::string aIdentifier );
        static void flushSamples();
};

namespace SampleManagerSamples
{
    extern std::map<std::string, cachedSample> _sampleMap;
}

#endif
