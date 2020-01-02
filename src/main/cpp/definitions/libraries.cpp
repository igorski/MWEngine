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
#include "libraries.h"

namespace MWEngine {

#ifndef INCLUDE_AAUDIO_LIBRARY

    namespace AAudio {

        Signatures::I_PPB    AAudio_createStreamBuilder = nullptr;
        Signatures::I_PB     AAudioStreamBuilder_delete = nullptr;
        Signatures::I_PBPPS  AAudioStreamBuilder_openStream = nullptr;
        Signatures::V_PBI    AAudioStreamBuilder_setBufferCapacityInFrames = nullptr;
        Signatures::V_PBI    AAudioStreamBuilder_setChannelCount = nullptr;
        Signatures::V_PBI    AAudioStreamBuilder_setDeviceId = nullptr;
        Signatures::V_PBI    AAudioStreamBuilder_setDirection = nullptr;
        Signatures::V_PBI    AAudioStreamBuilder_setFormat = nullptr;
        Signatures::V_PBI    AAudioStreamBuilder_setFramesPerDataCallback = nullptr;
        Signatures::V_PBI    AAudioStreamBuilder_setPerformanceMode = nullptr;
        Signatures::V_PBI    AAudioStreamBuilder_setSampleRate = nullptr;
        Signatures::V_PBI    AAudioStreamBuilder_setSharingMode = nullptr;
        Signatures::V_PBPDPV AAudioStreamBuilder_setDataCallback  = nullptr;
        Signatures::V_PBPEPV AAudioStreamBuilder_setErrorCallback = nullptr;

        Signatures::F_PS      AAudioStream_getFormat = nullptr;
        Signatures::I_PSPVIL  AAudioStream_read = nullptr;
        Signatures::I_PSCPVIL AAudioStream_write = nullptr;
        Signatures::I_PSTPTL  AAudioStream_waitForStateChange = nullptr;
        Signatures::I_PSKPLPL AAudioStream_getTimestamp = nullptr;
        Signatures::B_PS      AAudioStream_isMMapUsed = nullptr;
        Signatures::I_PS      AAudioStream_close = nullptr;
        Signatures::I_PS      AAudioStream_getChannelCount = nullptr;
        Signatures::I_PS      AAudioStream_getDeviceId = nullptr;
        Signatures::I_PS      AAudioStream_getBufferSizeInFrames = nullptr;
        Signatures::I_PS      AAudioStream_getBufferCapacity = nullptr;
        Signatures::I_PS      AAudioStream_getFramesPerBurst = nullptr;
        Signatures::I_PS      AAudioStream_getState = nullptr;
        Signatures::I_PS      AAudioStream_getPerformanceMode = nullptr;
        Signatures::I_PS      AAudioStream_getSampleRate = nullptr;
        Signatures::I_PS      AAudioStream_getSharingMode = nullptr;
        Signatures::I_PS      AAudioStream_getXRunCount = nullptr;
        Signatures::I_PSI     AAudioStream_setBufferSizeInFrames = nullptr;
        Signatures::I_PS      AAudioStream_requestStart = nullptr;
        Signatures::I_PS      AAudioStream_requestPause = nullptr;
        Signatures::I_PS      AAudioStream_requestFlush = nullptr;
        Signatures::I_PS      AAudioStream_requestStop = nullptr;
        Signatures::L_PS      AAudioStream_getFramesRead = nullptr;
        Signatures::L_PS      AAudioStream_getFramesWritten = nullptr;

        Signatures::CPH_I AAudio_convertResultToText = nullptr;
    }

#endif

} // E.O. namespace MWEngine