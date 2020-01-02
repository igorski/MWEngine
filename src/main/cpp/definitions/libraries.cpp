#include "libraries.h"

namespace MWEngine {

#ifndef INCLUDE_AAUDIO_LIBRARY

namespace AAudio {

    Signatures::sig_I_PPB AAudio_createStreamBuilder = nullptr;
    Signatures::sig_I_PB  AAudioStreamBuilder_delete = nullptr;

    Signatures::sig_I_PBPPS AAudioStreamBuilder_openStream = nullptr;

    Signatures::sig_V_PBI AAudioStreamBuilder_setBufferCapacityInFrames = nullptr;
    Signatures::sig_V_PBI AAudioStreamBuilder_setChannelCount = nullptr;
    Signatures::sig_V_PBI AAudioStreamBuilder_setDeviceId = nullptr;
    Signatures::sig_V_PBI AAudioStreamBuilder_setDirection = nullptr;
    Signatures::sig_V_PBI AAudioStreamBuilder_setFormat = nullptr;
    Signatures::sig_V_PBI AAudioStreamBuilder_setFramesPerDataCallback = nullptr;
    Signatures::sig_V_PBI AAudioStreamBuilder_setPerformanceMode = nullptr;
    Signatures::sig_V_PBI AAudioStreamBuilder_setSampleRate = nullptr;
    Signatures::sig_V_PBI AAudioStreamBuilder_setSharingMode = nullptr;

    Signatures::sig_V_PBI AAudioStreamBuilder_setUsage = nullptr;
    Signatures::sig_V_PBI AAudioStreamBuilder_setContentType = nullptr;
    Signatures::sig_V_PBI AAudioStreamBuilder_setInputPreset = nullptr;
    Signatures::sig_V_PBI AAudioStreamBuilder_setSessionId = nullptr;

    Signatures::sig_V_PBPDPV AAudioStreamBuilder_setDataCallback  = nullptr;
    Signatures::sig_V_PBPEPV AAudioStreamBuilder_setErrorCallback = nullptr;

    Signatures::sig_F_PS AAudioStream_getFormat = nullptr;

    Signatures::sig_I_PSPVIL AAudioStream_read = nullptr;
    Signatures::sig_I_PSCPVIL AAudioStream_write = nullptr;

    Signatures::sig_I_PSTPTL AAudioStream_waitForStateChange = nullptr;

    Signatures::sig_I_PSKPLPL AAudioStream_getTimestamp = nullptr;

    Signatures::sig_B_PS AAudioStream_isMMapUsed = nullptr;

    Signatures::sig_I_PS AAudioStream_close = nullptr;

    Signatures::sig_I_PS AAudioStream_getChannelCount = nullptr;
    Signatures::sig_I_PS AAudioStream_getDeviceId = nullptr;

    Signatures::sig_I_PS AAudioStream_getBufferSizeInFrames = nullptr;
    Signatures::sig_I_PS AAudioStream_getBufferCapacity = nullptr;
    Signatures::sig_I_PS AAudioStream_getFramesPerBurst = nullptr;
    Signatures::sig_I_PS AAudioStream_getState = nullptr;
    Signatures::sig_I_PS AAudioStream_getPerformanceMode = nullptr;
    Signatures::sig_I_PS AAudioStream_getSampleRate = nullptr;
    Signatures::sig_I_PS AAudioStream_getSharingMode = nullptr;
    Signatures::sig_I_PS AAudioStream_getXRunCount = nullptr;

    Signatures::sig_I_PSI AAudioStream_setBufferSizeInFrames = nullptr;
    Signatures::sig_I_PS AAudioStream_requestStart = nullptr;
    Signatures::sig_I_PS AAudioStream_requestPause = nullptr;
    Signatures::sig_I_PS AAudioStream_requestFlush = nullptr;
    Signatures::sig_I_PS AAudioStream_requestStop = nullptr;

    Signatures::sig_L_PS AAudioStream_getFramesRead = nullptr;
    Signatures::sig_L_PS AAudioStream_getFramesWritten = nullptr;

    Signatures::sig_CPH_I AAudio_convertResultToText = nullptr;

    Signatures::sig_I_PS AAudioStream_getUsage       = nullptr;
    Signatures::sig_I_PS AAudioStream_getContentType = nullptr;
    Signatures::sig_I_PS AAudioStream_getInputPreset = nullptr;
    Signatures::sig_I_PS AAudioStream_getSessionId   = nullptr;

} // E.O. namespace AAudio
#endif

} // E.O. namespace MWEngine