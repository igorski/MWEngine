MWEngine is..
=============

...an audio engine for Android written in C++, using OpenSL for low latency performance. The engine has
been written for the MikroWave synth/sequencing-application.

The engine provides an architecture that allows you to work with audio within a musical context (i.e.: tempo
synced sequencing working within time signatures, multi-channel output of either (live) synthesized audio or
sample based-playback). It is easy to extend the base classes for your custom audio generating mayhem.

### Build instructions

The makefile (src/jni/Android.mk) has been written to build the library using the Android NDK (Native Development Kit) in conjunction with SWIG.

Those of a Unix-bent can immediately un the _build.sh_-file in the /src-folder, just make sure ANDROID_NDK_ROOT points to the correct installation
directory on your system!

For the unfortunates on Windows, adjusting the sh to a .BAT-file shouldn't be too troublesome. :-)

### SWIG ?

While you CAN use the library and write the application solely using C++, the Android Java SDK is a convenient development
tool and with SWIG it is possible to construct and communicate with native layer code from Java, while enjoying all the
benefits of running hyperfast code outside of the Dalvik VM. It is however important to note that when a Java object
finalizes (i.e. is out of scope and is garbage collected), the destructors on the native objects are invoked, which can
lead to unpredictable results if you happen to overlook this! As such, audio engine objects such as effects processors
or events that are created on the Java side, must also hold strong references during their lifecycle.

### Documentation

this repository is constantly being updated and as such so is the documentation. You can view the Wiki (which will document the basic
engine architecture) here:

https://github.com/igorski/MWEngine/wiki

 * Wiki pages documenting basic engine architecture

Note you can always view the contents of the header files to get more details about the inner workings of each class.

### Demo

The repository contains an example Activity that is ready to deploy onto an Android device / emulator. The example will
demonstrate how to quickly get a musical sequence going using the library.
