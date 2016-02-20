MWEngine is..
=============

...an audio engine for Android written in C++, using OpenSL for low latency performance. The engine has
been written for the MikroWave synthesis/sequencing-application and works from API level 9 (Android 2.3/Gingerbread)
and up.

The engine provides an architecture that allows you to work with audio within a musical context. It is easy to
build upon the base classes and create your own noise generating mayhem. A few keywords describing the
out-of-the-box possibilities are:

 * tempo-based sequencing
 * support for alternate time signatures
 * multi-channel audio output
 * effect chains operating on individual channels
 * on-the-fly audio synthesis
 * sample-based playback (e.g. drum machines)
 * bouncing output to WAV files, either live or "offline"

### SWIG / What about Java ?

Though the library is written in C++ (and can be used solely within this context), the library is built using JNI
(Java Native Interface) allowing its methods to be exposed to Java while still executing in a native layer outside of
the Dalvik/ART VM. In other words : high performance of the engine is ensured by the native layer operations, while
ease of development is ensured by keeping application logic / UI within the realm of the Android Java SDK.

If you intend to use the MWEngine for sample based playback / use the built-in synthesizer and processors you will not need to write any additional C++ code. If you however intend to create your own processors or synthesis routines (which is fun!), you must write them in C++, but can rely on SWIG for making them usable in Java.

#### A note on garbage collection and SWIG

It is important to note that when a Java object finalizes (i.e. all its references are broken and is garbage collected), the
destructors of the native objects are invoked, which can lead to unpredictable results if you happen to overlook this!
As such, audio engine objects such as effects processors or events that are created on the Java side, must also hold
strong references during their lifecycle.

### Build instructions

The makefile (_/jni/Android.mk_) will by default compile the library with all available modules. The SWIG interface file
(_/jni/mwengine.i_) includes all the engine's actors that will be exposed to Java.

Those of a Unix-bent can run the _build.sh_-file in the root folder of the repository whereas Windows users can run the
_build.bat_-file that resides in the same directory, just make sure "_ndk-build_" and "_swig_" are globally available
through the PATH settings of your system (or adjust the shell scripts accordingly).

After compiling the C++ code, the SWIG wrappers will generate the _nl.igorski.lib.audio.nativeaudio_-namespace, making the code available to Java.

You can create the .APK package and deploy it instantly onto an attached device / emulator by using Gradle e.g. :

    gradle installDebug

The usual Gradle suspects such as "clean", "build", etc. are also present.

To create a signed release build, add the following into your Gradle's properties file (_~/.gradle/gradle.properties_) and
replace the values accordingly:

    RELEASE_STORE_FILE={path_to_.keystore_file}
    RELEASE_STORE_PASSWORD={password_for_.keystore}
    RELEASE_KEY_ALIAS={alias_for_.keystore}
    RELEASE_KEY_PASSWORD={password_for_.keystore}

You can now build and sign a releasable APK by running:

    gradle build
    
### Unit tests

The library comes with unit tests (_/jni/tests/_), written using the Googletest C++ testing framework (distributed with NDK 10).
To run the tests, simply execute the _test.sh_ (sorry Unix-only shell at the moment)-script with a device attached.
This will also build the library prior to running the tests by calling the build script described above.
Note: _adb_ must be specified in your global path settings.

To repeat the tests you can provide an optional numerical argument to the shell script, for instance :

    ./sh test.sh 500

will repeat all tests 500 times or break on the first failed test. This can be a convenient method to hunt down a
unit test that fails at random.

*Note on unit testing:* To build the application for unit testing observe that there are separate makefiles for the
unit test mode (see suffix _test_ for the .mk files). In short these files set the compiler preprocesser MOCK_ENGINE which
replaces the OpenSL driver with a mocked driver so the engine can be unit tested "offline".

### Documentation

This repository is constantly being updated and as such so is the documentation. You can view the Wiki (which will document the basic
engine architecture) here:

https://github.com/igorski/MWEngine/wiki

Note you can always view the contents of the header files to get more details about the inner workings of each class.

### Demo

The repository contains an example Activity that is ready to deploy onto any Android device/emulator supporting ARM-, ARMv7-,
x86- or MIPS-architecture and running Android 2.3 or higher. The example will demonstrate how to quickly get a musical
sequence going using the library.

To install the demo : first build the library as described above, and then run the Ant build script to deploy the .APK unto an
attached device/emulator (note that emulated devices can only operate at a sample rate of 8 kHz!). This requires both the Android NDK and the Android SDK.

Be sure to point towards the installation locations of these in both the _build.sh_-file and the _local.properties_-file.

### Contributors

MWEngine has received welcome contributions (either suggestions on improving the API or proposal of new features,
solving of bugs, etc.) from the following developers :

 * Andrey Stavrinov (@hypeastrum)
 * Aran Arunakiri
