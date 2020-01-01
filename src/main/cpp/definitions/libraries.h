/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Igor Zinken - https://www.igorski.nl
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

#pragma once

namespace MWEngine {

#ifndef INCLUDE_AAUDIO_LIBRARY

    // --- runtime linking of AAudio library methods
    // --- this allows us to build the engine at a lower minSdkVersion
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

        typedef aaudio_data_callback_result_t (*AAudioStream_dataCallback)(
                AAudioStream *stream,
                void *userData,
                void *audioData,
                int32_t numFrames);

        typedef void (*AAudioStream_errorCallback)(
                AAudioStream *stream,
                void *userData,
                aaudio_result_t error);

        // method signatures provided by the AAudio API (this is used by library_loader to
        // map the methods in the loaded library to this mediating layer). The abbreviations are:
        // S = Stream, B = Builder, I = int32_t, L = int64_t, T = sTate, K = clocKid_t
        // P = Pointer, C = const prefix, H = cHar

        namespace Signatures {

            typedef int32_t  (*sig_I_PPB)(AAudioStreamBuilder **builder);
    
            typedef const char *(*sig_CPH_I)(int32_t);
    
            typedef int32_t (*sig_I_PBPPS)(AAudioStreamBuilder *,
                                                 AAudioStream **stream);  // AAudioStreamBuilder_open()
    
            typedef int32_t (*sig_I_PB)(AAudioStreamBuilder *);  // AAudioStreamBuilder_delete()
            // AAudioStreamBuilder_setSampleRate()
            typedef void    (*sig_V_PBI)(AAudioStreamBuilder *, int32_t);
    
            typedef int32_t (*sig_I_PS)(AAudioStream *);  // AAudioStream_getSampleRate()
            typedef int64_t (*sig_L_PS)(AAudioStream *);  // AAudioStream_getFramesRead()
            // AAudioStream_setBufferSizeInFrames()
            typedef int32_t (*sig_I_PSI)(AAudioStream *, int32_t);
    
            typedef void    (*sig_V_PBPDPV)(AAudioStreamBuilder *,
                                                  AAudioStream_dataCallback,
                                                  void *);
    
            typedef void    (*sig_V_PBPEPV)(AAudioStreamBuilder *,
                                                  AAudioStream_errorCallback,
                                                  void *);
    
            typedef aaudio_format_t (*sig_F_PS)(AAudioStream *stream);
    
            typedef int32_t (*sig_I_PSPVIL)(AAudioStream *, void *, int32_t, int64_t);
    
            typedef int32_t (*sig_I_PSCPVIL)(AAudioStream *, const void *, int32_t, int64_t);
    
            typedef int32_t (*sig_I_PSTPTL)(AAudioStream *,
                                            aaudio_stream_state_t,
                                            aaudio_stream_state_t *,
                                            int64_t);
    
            typedef int32_t (*sig_I_PSKPLPL)(AAudioStream *, clockid_t, int64_t *, int64_t *);
    
            typedef bool    (*sig_B_PS)(AAudioStream *);
    
        }    // E.O. TODO name

        // methods provided by the AAudio API
        // Note we use the same names as the in the library so aaudio_io can
        // remain agnostic as to whether or not AAudio was bundled with MWEngine
        // or loaded at runtime using library_loader
    
        extern Signatures::sig_I_PPB AAudio_createStreamBuilder = nullptr;
        extern Signatures::sig_I_PB  AAudioStreamBuilder_delete = nullptr;

        extern Signatures::sig_I_PBPPS AAudioStreamBuilder_openStream = nullptr;

        extern Signatures::sig_V_PBI AAudioStreamBuilder_setBufferCapacityInFrames = nullptr;
        extern Signatures::sig_V_PBI AAudioStreamBuilder_setChannelCount = nullptr;
        extern Signatures::sig_V_PBI AAudioStreamBuilder_setDeviceId = nullptr;
        extern Signatures::sig_V_PBI AAudioStreamBuilder_setDirection = nullptr;
        extern Signatures::sig_V_PBI AAudioStreamBuilder_setFormat = nullptr;
        extern Signatures::sig_V_PBI AAudioStreamBuilder_setFramesPerDataCallback = nullptr;
        extern Signatures::sig_V_PBI AAudioStreamBuilder_setPerformanceMode = nullptr;
        extern Signatures::sig_V_PBI AAudioStreamBuilder_setSampleRate = nullptr;
        extern Signatures::sig_V_PBI AAudioStreamBuilder_setSharingMode = nullptr;

        extern Signatures::sig_V_PBI AAudioStreamBuilder_setUsage = nullptr;
        extern Signatures::sig_V_PBI AAudioStreamBuilder_setContentType = nullptr;
        extern Signatures::sig_V_PBI AAudioStreamBuilder_setInputPreset = nullptr;
        extern Signatures::sig_V_PBI AAudioStreamBuilder_setSessionId = nullptr;

        extern Signatures::sig_V_PBPDPV AAudioStreamBuilder_setDataCallback  = nullptr;
        extern Signatures::sig_V_PBPEPV AAudioStreamBuilder_setErrorCallback = nullptr;


        extern Signatures::sig_F_PS AAudioStream_getFormat = nullptr;

        extern Signatures::sig_I_PSPVIL AAudioStream_read = nullptr;
        extern Signatures::sig_I_PSCPVIL AAudioStream_write = nullptr;

        extern Signatures::sig_I_PSTPTL AAudioStream_waitForStateChange = nullptr;

        extern Signatures::sig_I_PSKPLPL AAudioStream_getTimestamp = nullptr;

        extern Signatures::sig_B_PS AAudioStream_isMMapUsed = nullptr;

        extern Signatures::sig_I_PS AAudioStream_close = nullptr;

        extern Signatures::sig_I_PS AAudioStream_getChannelCount = nullptr;
        extern Signatures::sig_I_PS AAudioStream_getDeviceId = nullptr;

        extern Signatures::sig_I_PS AAudioStream_getBufferSizeInFrames = nullptr;
        extern Signatures::sig_I_PS AAudioStream_getBufferCapacity = nullptr;
        extern Signatures::sig_I_PS AAudioStream_getFramesPerBurst = nullptr;
        extern Signatures::sig_I_PS AAudioStream_getState = nullptr;
        extern Signatures::sig_I_PS AAudioStream_getPerformanceMode = nullptr;
        extern Signatures::sig_I_PS AAudioStream_getSampleRate = nullptr;
        extern Signatures::sig_I_PS AAudioStream_getSharingMode = nullptr;
        extern Signatures::sig_I_PS AAudioStream_getXRunCount = nullptr;

        extern Signatures::sig_I_PSI AAudioStream_setBufferSizeInFrames = nullptr;
        extern Signatures::sig_I_PS AAudioStream_requestStart = nullptr;
        extern Signatures::sig_I_PS AAudioStream_requestPause = nullptr;
        extern Signatures::sig_I_PS AAudioStream_requestFlush = nullptr;
        extern Signatures::sig_I_PS AAudioStream_requestStop = nullptr;

        extern Signatures::sig_L_PS AAudioStream_getFramesRead = nullptr;
        extern Signatures::sig_L_PS AAudioStream_getFramesWritten = nullptr;

        extern Signatures::sig_CPH_I AAudio_convertResultToText = nullptr;

        extern Signatures::sig_I_PS AAudioStream_getUsage       = nullptr;
        extern Signatures::sig_I_PS AAudioStream_getContentType = nullptr;
        extern Signatures::sig_I_PS AAudioStream_getInputPreset = nullptr;
        extern Signatures::sig_I_PS AAudioStream_getSessionId   = nullptr;
    }

    // --- runtime linking of AAudio library methods

    /*
    AAudioStream_getPerformanceMode
            AAudioStream_getFormat
    AAudioStream_getXRunCount
            AAudioStream_requestStart
    AAudioStream_requestStop
            AAudioStream_close
    AAudioStream_read
            AAudioStream_getTimestamp
    AAudioStream_getFramesWritten
            AAudioStream_getState
    AAudioStream_getFramesPerBurst
            AAudioStream_getBufferSizeInFrames
    AAudioStream_setBufferSizeInFrames

    AAudioStreamBuilder_setDeviceId
    AAudioStreamBuilder_setFormat
    AAudioStream_getSampleRate
            AAudioStreamBuilder_setSampleRate
    AAudioStreamBuilder_setDirection
            AAudioStreamBuilder_setDataCallback
    AAudioStreamBuilder_setErrorCallback
            AAudioStreamBuilder_setSharingMode
    AAudio_convertResultToText
    */

#endif

} // E.O. namespace MWEngine

#endif