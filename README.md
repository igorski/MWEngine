MWEngine is..
=============

...an audio engine for Android written in C++, using OpenSL for low latency performance. The engine has
been written for the MikroWave synthesis/sequencing-application.

The engine provides an architecture that allows you to work with audio within a musical context. It is easy to
build upon the base classes and create your own noise generating mayhem. A few keywords describing the
out-of-the-box possibilities are:

 * tempo-based sequencing
 * support for alternate time signatures
 * multi-channel audio output
 * effect chains operating on individual channels
 * (live) synthesized audio
 * sample-based playback (e.g. drum machines)
 * bouncing output to WAV files, either live or "offline"

### SWIG / What about Java ?

Though the library is written in C++ (and can be used solely within this context), the library is built using JNI
(Java Native Interface) allowing its methods to be called from Java while still operating in a native layer outside of
the Dalvik VM. In other words : high performance of the engine is ensured by the native layer, but ease of development
is ensured by keeping application logic / UI within the realm of the Android Java SDK.

It is however important to note that when a Java object finalizes (i.e. goes out of scope and is garbage collected), the
destructors of the native objects are invoked, which can lead to unpredictable results if you happen to overlook this!
As such, audio engine objects such as effects processors or events that are created on the Java side, must also hold
strong references during their lifecycle.

### Build instructions

The makefile (/jni/Android.mk) has been written to build the library using the Android NDK (Native Development Kit) in conjunction with SWIG.

Those of a Unix-bent can immediately run the _build.sh_-file in the /src-folder, just make sure ANDROID_NDK_ROOT points to the correct installation
directory on your system!

For the unfortunates on Windows, adjusting the shell script to a .BAT-file shouldn't be too troublesome. :-)

### Documentation

This repository is constantly being updated and as such so is the documentation. You can view the Wiki (which will document the basic
engine architecture) here:

https://github.com/igorski/MWEngine/wiki

Note you can always view the contents of the header files to get more details about the inner workings of each class.

### Demo

The repository contains an example Activity that is ready to deploy onto any Android device/emulator supporting ARMv7 architecture
and running Android 2.3 or higher. The example will demonstrate how to quickly get a musical sequence going using the library.

For installing the demo first build the library as described above, and then run the ant build script to deploy the .APK unto an
attached device/emulator.. This requires both the Android NDK and the Android SDK. Be sure to point towards the installation locations
of these in both the build.sh-file and the local.properties-file.
