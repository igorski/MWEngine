#ifndef __NATIVEAUDIOENGINE_H_INCLUDED__
#define __NATIVEAUDIOENGINE_H_INCLUDED__

/**
 * this is the root of AudioEngine which will
 * handle all output and rendering duties
 */
#include <jni.h>
#include "utils.h"
#include "global.h"

// internal handlers triggering JNI callbacks
void handleTempoUpdate            ( float aQueuedTempo, bool broadcastUpdate );
void handleSequencerPositionUpdate( float streamTimeStamp );
void handleBounceComplete         ( int aIdentifier );
void broadcastStepPosition();
void handleHardwareUnavailable();

#endif
