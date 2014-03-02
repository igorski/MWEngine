#ifndef __NATIVEAUDIOENGINEAPI_H_INCLUDED__
#define __NATIVEAUDIOENGINEAPI_H_INCLUDED__

    /**
     * this is the API exposed to the Java application
     * for interacting with the Native Audio Engine
     */

    extern "C"
    {
        void init();
        void start();
        void stop();
        void reset();
    }

#endif
