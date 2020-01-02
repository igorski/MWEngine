/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Igor Zinken - https://www.igorski.nl
 *
 * Implementation adapted from the Android Open Source Project
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
#include "library_loader.h"

namespace MWEngine {

LibraryLoader::~LibraryLoader() {
#ifndef INCLUDE_AAUDIO_LIBRARY
    if ( _aaudioLib != nullptr ) {
        dlclose( _aaudioLib );
        _aaudioLib = nullptr;
    }
#endif
}

LibraryLoader* LibraryLoader::getInstance() {
    static LibraryLoader instance;
    return &instance;
}

#ifndef INCLUDE_AAUDIO_LIBRARY
bool LibraryLoader::loadAAudioLibrary() {
    if ( _aaudioLib != nullptr ) {
        return true;
    }

    _aaudioLib = dlopen( "libaaudio.so", RTLD_NOW );
    if ( _aaudioLib == nullptr ) {
        return false;
    }

    // map all the functions in the loaded library to the wrapper
    // definitions in libraries.h

    using namespace AAudio;
    using namespace Signatures;

    AAudio_createStreamBuilder = MAP_AND_CAST( _aaudioLib, sig_I_PPB, "AAudio_createStreamBuilder");
    AAudioStreamBuilder_openStream = MAP_AND_CAST( _aaudioLib, sig_I_PBPPS, "AAudioStreamBuilder_openStream");

    AAudioStreamBuilder_setChannelCount = MAP_AND_CAST( _aaudioLib, sig_V_PBI, "AAudioStreamBuilder_setChannelCount");
    if (AAudioStreamBuilder_setChannelCount == nullptr) {
        AAudioStreamBuilder_setChannelCount = MAP_AND_CAST( _aaudioLib, sig_V_PBI, "AAudioStreamBuilder_setSamplesPerFrame");
    }

    AAudioStreamBuilder_setBufferCapacityInFrames = MAP_AND_CAST( _aaudioLib, sig_V_PBI, "AAudioStreamBuilder_setBufferCapacityInFrames");
    AAudioStreamBuilder_setDeviceId        = MAP_AND_CAST( _aaudioLib, sig_V_PBI, "AAudioStreamBuilder_setDeviceId");
    AAudioStreamBuilder_setDirection       = MAP_AND_CAST( _aaudioLib, sig_V_PBI, "AAudioStreamBuilder_setDirection");
    AAudioStreamBuilder_setFormat          = MAP_AND_CAST( _aaudioLib, sig_V_PBI, "AAudioStreamBuilder_setFormat");
    AAudioStreamBuilder_setFramesPerDataCallback = MAP_AND_CAST( _aaudioLib, sig_V_PBI, "AAudioStreamBuilder_setFramesPerDataCallback");
    AAudioStreamBuilder_setSharingMode     = MAP_AND_CAST( _aaudioLib, sig_V_PBI, "AAudioStreamBuilder_setSharingMode");
    AAudioStreamBuilder_setPerformanceMode = MAP_AND_CAST( _aaudioLib, sig_V_PBI, "AAudioStreamBuilder_setPerformanceMode");
    AAudioStreamBuilder_setSampleRate      = MAP_AND_CAST( _aaudioLib, sig_V_PBI, "AAudioStreamBuilder_setSampleRate");

    AAudioStreamBuilder_delete             = MAP_AND_CAST( _aaudioLib, sig_I_PB, "AAudioStreamBuilder_delete");


    AAudioStreamBuilder_setDataCallback    = MAP_AND_CAST( _aaudioLib, sig_V_PBPDPV, "AAudioStreamBuilder_setDataCallback");
    AAudioStreamBuilder_setErrorCallback   = MAP_AND_CAST( _aaudioLib, sig_V_PBPEPV, "AAudioStreamBuilder_setErrorCallback");

    AAudioStream_read                = MAP_AND_CAST( _aaudioLib, sig_I_PSPVIL, "AAudioStream_read");
    AAudioStream_write               = MAP_AND_CAST( _aaudioLib, sig_I_PSCPVIL, "AAudioStream_write");
    AAudioStream_waitForStateChange  = MAP_AND_CAST( _aaudioLib, sig_I_PSTPTL, "AAudioStream_waitForStateChange");
    AAudioStream_getTimestamp        = MAP_AND_CAST( _aaudioLib, sig_I_PSKPLPL, "AAudioStream_getTimestamp");
    AAudioStream_isMMapUsed          = MAP_AND_CAST( _aaudioLib, sig_B_PS, "AAudioStream_isMMapUsed");
    AAudioStream_getChannelCount     = MAP_AND_CAST( _aaudioLib, sig_I_PS, "AAudioStream_getChannelCount");
    if (AAudioStream_getChannelCount == nullptr) {
        // Use old alias if needed.
        AAudioStream_getChannelCount    = MAP_AND_CAST( _aaudioLib, sig_I_PS, "AAudioStream_getSamplesPerFrame");
    }

    AAudioStream_close                 = MAP_AND_CAST( _aaudioLib, sig_I_PS, "AAudioStream_close");
    AAudioStream_getBufferSizeInFrames = MAP_AND_CAST( _aaudioLib, sig_I_PS, "AAudioStream_getBufferSizeInFrames");
    AAudioStream_getDeviceId           = MAP_AND_CAST( _aaudioLib, sig_I_PS, "AAudioStream_getDeviceId");
    AAudioStream_getBufferCapacity     = MAP_AND_CAST( _aaudioLib, sig_I_PS, "AAudioStream_getBufferCapacityInFrames");
    AAudioStream_getFormat             = MAP_AND_CAST( _aaudioLib, sig_F_PS, "AAudioStream_getFormat");
    AAudioStream_getFramesPerBurst     = MAP_AND_CAST( _aaudioLib, sig_I_PS, "AAudioStream_getFramesPerBurst");
    AAudioStream_getFramesRead         = MAP_AND_CAST( _aaudioLib, sig_L_PS, "AAudioStream_getFramesRead");
    AAudioStream_getFramesWritten      = MAP_AND_CAST( _aaudioLib, sig_L_PS, "AAudioStream_getFramesWritten");
    AAudioStream_getPerformanceMode    = MAP_AND_CAST( _aaudioLib, sig_I_PS, "AAudioStream_getPerformanceMode");
    AAudioStream_getSampleRate         = MAP_AND_CAST( _aaudioLib, sig_I_PS, "AAudioStream_getSampleRate");
    AAudioStream_getSharingMode        = MAP_AND_CAST( _aaudioLib, sig_I_PS, "AAudioStream_getSharingMode");
    AAudioStream_getState              = MAP_AND_CAST( _aaudioLib, sig_I_PS, "AAudioStream_getState");
    AAudioStream_getXRunCount          = MAP_AND_CAST( _aaudioLib, sig_I_PS, "AAudioStream_getXRunCount");

    AAudioStream_requestStart          = MAP_AND_CAST( _aaudioLib, sig_I_PS, "AAudioStream_requestStart");
    AAudioStream_requestPause          = MAP_AND_CAST( _aaudioLib, sig_I_PS, "AAudioStream_requestPause");
    AAudioStream_requestFlush          = MAP_AND_CAST( _aaudioLib, sig_I_PS, "AAudioStream_requestFlush");
    AAudioStream_requestStop           = MAP_AND_CAST( _aaudioLib, sig_I_PS, "AAudioStream_requestStop");

    AAudioStream_setBufferSizeInFrames = MAP_AND_CAST( _aaudioLib, sig_I_PSI, "AAudioStream_setBufferSizeInFrames");

    AAudio_convertResultToText         = MAP_AND_CAST( _aaudioLib, sig_CPH_I, "AAudio_convertResultToText");

    return true;
}

/*
    sig_I_PPB LibraryLoader::MAP_AND_CAST( _aaudioLib, sig_I_PPB, const char *functionName) {
        void *proc = dlsym(_aaudioLib, functionName);
        //LibraryLoader_check(proc, functionName);
        return reinterpret_cast<sig_I_PPB>(proc);
    }

    sig_CPH_I LibraryLoader::MAP_AND_CAST( _aaudioLib, sig_CPH_I, const char *functionName) {
        void *proc = dlsym(_aaudioLib, functionName);
        //LibraryLoader_check(proc, functionName);
        return reinterpret_cast<sig_CPH_I>(proc);
    }

    sig_V_PBI LibraryLoader::MAP_AND_CAST( _aaudioLib, sig_V_PBI, const char *functionName) {
        void *proc = dlsym(_aaudioLib, functionName);
        //LibraryLoader_check(proc, functionName);
        return reinterpret_cast<sig_V_PBI>(proc);
    }

    sig_V_PBPDPV LibraryLoader::MAP_AND_CAST( _aaudioLib, sig_V_PBPDPV, const char *functionName) {
        void *proc = dlsym(_aaudioLib, functionName);
        //LibraryLoader_check(proc, functionName);
        return reinterpret_cast<sig_V_PBPDPV>(proc);
    }

    sig_V_PBPEPV LibraryLoader::MAP_AND_CAST( _aaudioLib, sig_V_PBPEPV(, const char *functionName) {
        void *proc = dlsym(_aaudioLib, functionName);
        //LibraryLoader_check(proc, functionName);
        return reinterpret_cast<sig_V_PBPEPV>(proc);
    }

    sig_I_PSI LibraryLoader::MAP_AND_CAST( _aaudioLib, sig_I_PSI(, const char *functionName) {
        void *proc = dlsym(_aaudioLib, functionName);
        //LibraryLoader_check(proc, functionName);
        return reinterpret_cast<sig_I_PSI>(proc);
    }

    sig_I_PS LibraryLoader::MAP_AND_CAST( _aaudioLib, sig_I_PS(, const char *functionName) {
        void *proc = dlsym(_aaudioLib, functionName);
        //LibraryLoader_check(proc, functionName);
        return reinterpret_cast<sig_I_PS>(proc);
    }

    sig_L_PS LibraryLoader::MAP_AND_CAST( _aaudioLib, sig_L_PS(, const char *functionName) {
        void *proc = dlsym(_aaudioLib, functionName);
        //LibraryLoader_check(proc, functionName);
        return reinterpret_cast<sig_L_PS>(proc);
    }

    sig_F_PS LibraryLoader::MAP_AND_CAST( _aaudioLib, sig_F_PS(, const char *functionName) {
        void *proc = dlsym(_aaudioLib, functionName);
        //LibraryLoader_check(proc, functionName);
        return reinterpret_cast<sig_F_PS>(proc);
    }

    sig_B_PS LibraryLoader::MAP_AND_CAST( _aaudioLib, sig_B_PS(, const char *functionName) {
        void *proc = dlsym(_aaudioLib, functionName);
        //LibraryLoader_check(proc, functionName);
        return reinterpret_cast<sig_B_PS>(proc);
    }

    sig_I_PB LibraryLoader::MAP_AND_CAST( _aaudioLib, sig_I_PB, const char *functionName) {
        void *proc = dlsym(_aaudioLib, functionName);
        //LibraryLoader_check(proc, functionName);
        return reinterpret_cast<sig_I_PB>(proc);
    }

    sig_I_PBPPS LibraryLoader::MAP_AND_CAST( _aaudioLib, sig_I_PBPPS, const char *functionName) {
        void *proc = dlsym(_aaudioLib, functionName);
        //LibraryLoader_check(proc, functionName);
        return reinterpret_cast<sig_I_PBPPS>(proc);
    }

    sig_I_PSCPVIL LibraryLoader::MAP_AND_CAST( _aaudioLib, sig_I_PSCPVIL(, const char *functionName) {
        void *proc = dlsym(_aaudioLib, functionName);
        //LibraryLoader_check(proc, functionName);
        return reinterpret_cast<sig_I_PSCPVIL>(proc);
    }

    sig_I_PSPVIL LibraryLoader::MAP_AND_CAST( _aaudioLib, sig_I_PSPVIL(, const char *functionName) {
        void *proc = dlsym(_aaudioLib, functionName);
        //LibraryLoader_check(proc, functionName);
        return reinterpret_cast<sig_I_PSPVIL>(proc);
    }

    sig_I_PSTPTL LibraryLoader::MAP_AND_CAST( _aaudioLib, sig_I_PSTPTL(, const char *functionName) {
        void *proc = dlsym(_aaudioLib, functionName);
        //LibraryLoader_check(proc, functionName);
        return reinterpret_cast<sig_I_PSTPTL>(proc);
    }

    sig_I_PSKPLPL LibraryLoader::MAP_AND_CAST( _aaudioLib, sig_I_PSKPLPL(, const char *functionName) {
        void *proc = dlsym(_aaudioLib, functionName);
        //LibraryLoader_check(proc, functionName);
        return reinterpret_cast<sig_I_PSKPLPL>(proc);
    }
    */
#endif
} // E.O. namespace MWEngine
