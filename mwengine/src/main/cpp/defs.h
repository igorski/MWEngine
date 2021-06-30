#ifndef __MWENGINE__DEFS_H_INCLUDED__
#define __MWENGINE__DEFS_H_INCLUDED__

// PRECISION defines the floating-point precision used to synthesize the audio samples
// valid options are 1 (32-bit float) and 2 (64-bit double)

#define PRECISION 2

#if PRECISION == 1 // float
    #define SAMPLE_TYPE float
    #define undenormalise(sample) ((((*(UINT32 *)&(sample))&0x7f800000)==0)&&((sample)!=0.f))
#endif

#if PRECISION == 2 // double
    #define SAMPLE_TYPE double
    #define undenormalise(sample) ((((((UINT32 *)&(sample))[1])&0x7fe00000)==0)&&((sample)!=0.f))
#endif

#endif