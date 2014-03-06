MWEngine is..
=============

...an audio engine for Android written in C++, using OpenSL for low latency performance. As used by MikroWave.

### Build instructions

The makefile (src/jni/Android.mk) has been written to build the library using the Android NDK (Native Development Kit) in conjunction with SWIG.

Those of a Unix-bent can immediately un the _build.sh_-file in the /src-folder, just make sure ANDROID_NDK_ROOT points to the correct installation
directory on your system!

What kind soul will supply a Windows .BAT to run the makefile ? ;-)

### SWIG ?

While you CAN use the library and write the application solely using C++, the Android Java SDK is a convenient development
tool and with SWIG it is possible to construct and communicate with native layer code from Java, while enjoying all the
benefits of running hyperfast code outside of the Dalvik VM. It is however important to note that when a Java object
finalizes (i.e. is out of scope and is garbage collected), the destructors on the native objects are invoked, which can
lead to unpredictable results if you happen to overlook this! As such, audio engine objects such as effects processors
or events that are created on the Java side, must also hold strong references during their lifecycle.

### Documentation

this repository is constantly being updated and as such so is the documentation. Planned are:

 * Wiki pages documenting basic engine architecture
 * Example Activity showing how to quickly get sound

 you can always view the contents of the header files to get more details about the inner workings of each class.