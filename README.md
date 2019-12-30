MWEngine is..
=============

...an audio engine for Android, using either OpenSL (compatible with Android 4.1 and up) or AAudio
(Android 8.0 and up) as the drivers for low latency audio performance.

MWEngine provides an architecture that allows you to work with audio within a _musical context_. It is easy to
build upon the base classes and create your own noise generating mayhem. A few keywords describing the
out-of-the-box possibilities are:

 * tempo-based sequencing with support for alternative time signatures
 * on-the-fly audio synthesis
 * multi-channel audio output
 * effect chains operating on individual input/output channels
 * sample playback with real time pitch shifting
 * bouncing output to WAV files, either live (during a performance) or "offline"

Also note that MWEngine's underlying audio drivers are _the same as Google Oboe uses_, MWEngine and
Oboe are merely abstraction layers to solve the same problem, only in different ways. Additionally, MWEngine provides a complete audio processing environment.

#### Who uses this ?

The engine has been written for both [MikroWave](https://play.google.com/store/apps/details?id=nl.igorski.mikrowave.free&hl=en) and
[Kosm](https://play.google.com/store/apps/details?id=nl.igorski.kosm&hl=en) to provide fast live audio synthesis.

While developments on those apps are scarce, the engine itself has been continuously improved and is now also
used by third party app developers, such as [TIZE - Beat Maker, Music Maker](https://play.google.com/store/apps/details?id=com.tizemusic.tize).

### The [Issue Tracker](https://github.com/igorski/MWEngine/issues?q=is%3Aissue+is%3Aopen+sort%3Aupdated-desc) is your point of contact

Bug reports, feature requests, questions and discussions are welcome on the GitHub Issue Tracker, please do not send e-mails through the development website. However, please search before posting to avoid duplicates, and limit to one issue per post.

Please vote on feature requests by using the Thumbs Up/Down reaction on the first post.

### C++ ??? What about Java / Kotlin ?

Though the library is written in C++ (and can be used solely within this context), the library can be built using JNI
(Java Native Interface) which makes its API expose itself to Java, while still executing in a native layer outside of
the JVM. In other words : high performance of the engine is ensured by the native layer operations, while
ease of development is ensured by delegating application logic / UI to the realm of the Android Java SDK.

Whether you intend to use MWEngine for its sample based playback or to leverage its built-in synthesizer and
audio processing, you are not required to write any additional C++ code. If you however intend to create your own
DSP or synthesis routines (which is fun to do!) you must write them in C++, but can rely on SWIG for making them usable in Java.

#### A note on garbage collection and SWIG

It is important to note that when a Java object finalizes (i.e. all its references are broken and is garbage collected), the
destructors of the native objects are invoked, which can lead to unpredictable results if you happen to overlook this!
As such, audio engine objects such as effects processors or events that are created on the Java side, must also hold
strong references during their lifecycle. Basically, follow the same principles you'd use in Java, but be
aware that ignoring these will have a particularly violent result with a very unfriendly stack trace.

### Environment setup

If you intend to use [Android Studio](https://developer.android.com/studio/) you can open the
_build.gradle_ file to setup the project and its dependencies accordingly by following the on-screen
steps.

For CLI builds:

You will need both the [Android SDK](https://developer.android.com/studio/index.html) and the [Android NDK](https://developer.android.com/ndk/downloads/index.html).
Additionally, you will need [SWIG](http://www.swig.org) (available on most package managers like _Brew_ for OS X or _apt-get_ on Linux)

You will need [CMake](https://cmake.org) and [Gradle](https://gradle.org) to run the build scripts. All aforementioned utilities are available on all major Operating Systems.

### Build instructions

The main configuration files are:

 * _build.gradle_ (to bundle native layer code with the example Android application)
 * _CMakeLists.txt_ (for the native layer code)
 
These define the appropriate tasks for both Gradle and CMake.
 
#### Using Android Studio

If you are using Android Studio, creating a project from the supplied _build.gradle_ file should
suffice to get you to build both the native and Java code as well as enabling building, debugging and
packaging directly both from its IDE.

Upon opening the repository the native layer library should be built automatically, the same when
running a build configuration after having made changes to the .cpp files. You can build and deploy
using debug and release configurations as usual.

Remember that when making changes to the make file, _Build > Refresh Linked C++ Projects_ must be invoked.

NOTE: there is a known issue where Android Studio is not aware Java classes have been generated
after completing the native library build (e.g. _/nl/igorski/mwengine/core_ containing files but
the IDE not showing auto complete - and oddly enough compiling just fine -). Restarting AS should suffice.

#### Using CLI

After making sure you have all the correct tools (see _Environment setup_):

##### Cleaning all generated output

```
gradle clean
```

Will delete the build output as well as the built native code and generated Java wrappers.

```
gradle externalNativeBuildRelease
```

Will create a release build of the native layer code and generate the Java wrappers. If you are
not packaging the MWEngine library directly into your application but will move these as dependencies
of another project, you would like to copy these files:

 * _build/intermediates/cmake/release/obj/*_ as it contains the native library for all defined architectures.
  These should go to the _/libs_ (or custom jniLibs)-folder of your Android project.
 * _/src/main/java/nl/igorski/mwengine/*_ as it contains the interface layer with the library. These
  should move to the appropriate namespace in your projects Java source folder.

### FAQ / Troubleshooting

The contents of this repository should result in a stable application. If you experience issues with
the setup, consult the [Troubleshooting Wiki page](https://github.com/igorski/MWEngine/wiki/Troubleshooting-MWEngine).

### Documentation

You can view the Wiki (which documents all of the engine's actors as well as a variety of real world
use cases) here:

[https://github.com/igorski/MWEngine/wiki](https://github.com/igorski/MWEngine/wiki)

Note that you can also view the contents of the header files to get more details about the inner
workings of each class.

### Unit tests

The library comes with unit tests (_/src/main/cpp/tests/_), written using the Googletest C++ testing framework.

To run the tests, we temporarily need a [https://github.com/igorski/MWEngine/issues/106](workaround) :

 * update _local.properties_ to include the line: _enable_tests=true_
 * run _gradle externalNativeBuildDebug_ with a device / emulator attached to your machine.
 
This will build a special version of the library including the test suite and will execute it directly onto the
attach device. Note: _adb_ must be available in your global path settings.

NOTE: to create a release build of your app (or continuing non-test related development) you must unset
the added line in _local.properties_. Once issue #106 is addressed it will no longer be necessary to
update local configuration files and it will be possible to run unit tests next to regular development.

### Demo

The repository contains an example Activity that is ready to deploy onto any Android device/emulator supporting ARM-, ARMv7-,
x86- architecture and running Android 4.1 or higher. The example will demonstrate how to quickly get a musical
sequence going using the library.

To install the demo: first build the library as described above, and then run the build script to deploy the .APK onto an
attached device/emulator (note that older emulated devices can only operate at a sample rate of 8 kHz!).

### Note on OpenSL / AAudio drivers

Currently it is not possible to switch between audio drivers on the fly, rather you must precompile
the library for use with a specific driver. By default, the library will compile for OpenSL for a
wider range of supported devices. If you want to use AAudio instead (and thus are targeting solely
devices running Android 8 and up) :

 * change the desired driver in _global.h_ from type 0 (OpenSL) to 1 (AAudio)

A [https://github.com/igorski/MWEngine/issues/106](future iteration) of the engine will allow runtime selection of audio drivers.

### Contributors

MWEngine has received welcome contributions (either suggestions on improving the API or proposal of new features,
solving of bugs, etc.) from the following developers :

 * Andrey Stavrinov (@hypeastrum)
 * Toufik Zitouni & Robert Avellar (Tize)
 * Koert Gaaikema (@koertgaaikema)
 * Matt Logan (@mattlogan)
 * Thomas Flasche (@harthorst)
 * Rickard Östergård (@rckrdstrgrd)
 * Aran Arunakiri
