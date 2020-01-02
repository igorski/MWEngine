/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Igor Zinken - https://www.igorski.nl
 *
 * AAudio library loader implementation adapted from the Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
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
#ifndef __MWENGINE__LIBRARIES_H_INCLUDED__
#define __MWENGINE__LIBRARIES_H_INCLUDED__

#include <stdint.h>
#include <sys/types.h>

/**
 * Libraries describes methods provided by runtime loaded libraries (such as
 * AAudio on builds with an APP_PLATFORM lower than 26).
 * 
 * The methods use the same naming and signature as the library originals, to
 * omit the need for MWEngine to use wrapper functions and to remain agnostic
 * with regards to dependencies.
 *
 * @see <library_loader.h>
 */
namespace MWEngine {

#ifndef INCLUDE_AAUDIO_LIBRARY

    // --- runtime linking of AAudio library methods
    // --- allows us to build the engine at a lower minSdkVersion
    // --- while allowing usage of AAudio on devices with a newer OS

    namespace AAudio {

        // types provided by <aaudio/AAudio.h>
        // we simply re-specify them here (hoping that the order doesn't change in AAudio !)

        const int32_t AAUDIO_UNSPECIFIED = 0;

        enum aaudio_stream_state_t : int32_t {
            AAUDIO_STREAM_STATE_UNINITIALIZED = 0,
            AAUDIO_STREAM_STATE_UNKNOWN = 1,
            AAUDIO_STREAM_STATE_OPEN = 2,
            AAUDIO_STREAM_STATE_STARTING = 3,
            AAUDIO_STREAM_STATE_STARTED = 4,
            AAUDIO_STREAM_STATE_PAUSING = 5,
            AAUDIO_STREAM_STATE_PAUSED = 6,
            AAUDIO_STREAM_STATE_FLUSHING = 7,
            AAUDIO_STREAM_STATE_FLUSHED = 8,
            AAUDIO_STREAM_STATE_STOPPING = 9,
            AAUDIO_STREAM_STATE_STOPPED = 10,
            AAUDIO_STREAM_STATE_CLOSING = 11,
            AAUDIO_STREAM_STATE_CLOSED = 12,
            AAUDIO_STREAM_STATE_DISCONNECTED = 13
        };
        enum aaudio_direction_t : int32_t {
            AAUDIO_DIRECTION_OUTPUT = 0,
            AAUDIO_DIRECTION_INPUT = 1
        };
        enum aaudio_format_t : int32_t {
            AAUDIO_FORMAT_INVALID = -1,
            AAUDIO_FORMAT_UNSPECIFIED = 0,
            AAUDIO_FORMAT_PCM_I16 = 1,
            AAUDIO_FORMAT_PCM_FLOAT = 2,
        };
        enum aaudio_data_callback_result_t : int32_t {
            AAUDIO_CALLBACK_RESULT_CONTINUE = 0,
            AAUDIO_CALLBACK_RESULT_STOP = 1
        };
        enum aaudio_result_t : int32_t {
            AAUDIO_OK = 0, // AAUDIO_OK
        };
        enum aaudio_sharing_mode_t : int32_t {
            AAUDIO_SHARING_MODE_EXCLUSIVE = 0,
            AAUDIO_SHARING_MODE_SHARED = 1
        };
        enum aaudio_performance_mode_t : int32_t {
            AAUDIO_PERFORMANCE_MODE_NONE = 10,
            AAUDIO_PERFORMANCE_MODE_POWER_SAVING = 11,
            AAUDIO_PERFORMANCE_MODE_LOW_LATENCY = 12
        };

        typedef struct AAudioStreamStruct         AAudioStream;
        typedef struct AAudioStreamBuilderStruct  AAudioStreamBuilder;

        typedef aaudio_data_callback_result_t( *AAudioStream_dataCallback )(
            AAudioStream *stream, void* userData, void* audioData, int32_t numFrames
        );

        typedef void( *AAudioStream_errorCallback )(
            AAudioStream *stream, void* userData, aaudio_result_t error
        );

        // method signatures provided by the AAudio API (this is used by library_loader to
        // map the methods in the loaded library to this mediating layer). The abbreviations are:
        // S = Stream, B = Builder, I = int32_t, L = int64_t, T = sTate, K = clocKid_t
        // P = Pointer, C = const prefix, H = cHar

        namespace Signatures {

            typedef int32_t         ( *I_PPB )    ( AAudioStreamBuilder** builder );
            typedef const char*     ( *CPH_I )    ( int32_t );
            typedef int32_t         ( *I_PBPPS )  ( AAudioStreamBuilder*, AAudioStream **stream );
            typedef int32_t         ( *I_PB )     ( AAudioStreamBuilder* );
            typedef void            ( *V_PBI )    ( AAudioStreamBuilder*, int32_t );
            typedef int32_t         ( *I_PS )     ( AAudioStream* );
            typedef int64_t         ( *L_PS )     ( AAudioStream* );
            typedef int32_t         ( *I_PSI )    ( AAudioStream *, int32_t );
            typedef void            ( *V_PBPDPV ) ( AAudioStreamBuilder*, AAudioStream_dataCallback,  void* );
            typedef void            ( *V_PBPEPV ) ( AAudioStreamBuilder*, AAudioStream_errorCallback, void* );
            typedef aaudio_format_t ( *F_PS )     ( AAudioStream *stream );
            typedef int32_t         ( *I_PSPVIL ) ( AAudioStream*, void*, int32_t, int64_t );
            typedef int32_t         ( *I_PSCPVIL )( AAudioStream*, const void*, int32_t, int64_t );
            typedef int32_t         ( *I_PSTPTL ) ( AAudioStream*, aaudio_stream_state_t, aaudio_stream_state_t*, int64_t );
            typedef int32_t         ( *I_PSKPLPL )( AAudioStream*, clockid_t, int64_t *, int64_t* );
            typedef bool            ( *B_PS )     ( AAudioStream* );
    
        }

        // methods provided by the AAudio API
        // Note we use the same names as the in the library so aaudio_io can
        // remain agnostic as to whether or not AAudio was bundled with MWEngine
        // or loaded at runtime using library_loader

        extern Signatures::I_PPB    AAudio_createStreamBuilder;
        extern Signatures::I_PB     AAudioStreamBuilder_delete;
        extern Signatures::I_PBPPS  AAudioStreamBuilder_openStream;
        extern Signatures::V_PBI    AAudioStreamBuilder_setBufferCapacityInFrames;
        extern Signatures::V_PBI    AAudioStreamBuilder_setChannelCount;
        extern Signatures::V_PBI    AAudioStreamBuilder_setDeviceId;
        extern Signatures::V_PBI    AAudioStreamBuilder_setDirection;
        extern Signatures::V_PBI    AAudioStreamBuilder_setFormat;
        extern Signatures::V_PBI    AAudioStreamBuilder_setFramesPerDataCallback;
        extern Signatures::V_PBI    AAudioStreamBuilder_setPerformanceMode;
        extern Signatures::V_PBI    AAudioStreamBuilder_setSampleRate;
        extern Signatures::V_PBI    AAudioStreamBuilder_setSharingMode;
        extern Signatures::V_PBPDPV AAudioStreamBuilder_setDataCallback ;
        extern Signatures::V_PBPEPV AAudioStreamBuilder_setErrorCallback;

        extern Signatures::F_PS      AAudioStream_getFormat;
        extern Signatures::I_PSPVIL  AAudioStream_read;
        extern Signatures::I_PSCPVIL AAudioStream_write;
        extern Signatures::I_PSTPTL  AAudioStream_waitForStateChange;
        extern Signatures::I_PSKPLPL AAudioStream_getTimestamp;
        extern Signatures::B_PS      AAudioStream_isMMapUsed;
        extern Signatures::I_PS      AAudioStream_close;
        extern Signatures::I_PS      AAudioStream_getChannelCount;
        extern Signatures::I_PS      AAudioStream_getDeviceId;
        extern Signatures::I_PS      AAudioStream_getBufferSizeInFrames;
        extern Signatures::I_PS      AAudioStream_getBufferCapacity;
        extern Signatures::I_PS      AAudioStream_getFramesPerBurst;
        extern Signatures::I_PS      AAudioStream_getState;
        extern Signatures::I_PS      AAudioStream_getPerformanceMode;
        extern Signatures::I_PS      AAudioStream_getSampleRate;
        extern Signatures::I_PS      AAudioStream_getSharingMode;
        extern Signatures::I_PS      AAudioStream_getXRunCount;
        extern Signatures::I_PSI     AAudioStream_setBufferSizeInFrames;
        extern Signatures::I_PS      AAudioStream_requestStart;
        extern Signatures::I_PS      AAudioStream_requestPause;
        extern Signatures::I_PS      AAudioStream_requestFlush;
        extern Signatures::I_PS      AAudioStream_requestStop;
        extern Signatures::L_PS      AAudioStream_getFramesRead;
        extern Signatures::L_PS      AAudioStream_getFramesWritten;

        extern Signatures::CPH_I AAudio_convertResultToText;
   }

#endif

} // E.O. namespace MWEngine

#endif