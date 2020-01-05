MWEngine is..
=============

...an audio engine for Android, compatible with Android 4.1 and up, using either OpenSL or AAudio
(when available) as the drivers for low latency audio performance.

MWEngine provides an architecture that allows you to work with audio within a _musical context_. It is easy to
build upon the base classes and create your own noise generating mayhem. A few keywords describing the
out-of-the-box possibilities are:

 * tempo-based sequencing with support for alternative time signatures
 * on-the-fly audio synthesis
 * multi-channel audio output
 * effect chains operating on individual input/output channels
 * sample playback with real time pitch shifting
 * live recording and processing from your device's inputs (e.g. microphone)
 * bouncing output to WAV files, either live (during a performance) or "offline"

Also note that MWEngine's underlying audio drivers are _the same as Google Oboe uses_, MWEngine and
Oboe are merely abstraction layers to solve the same problem, only in different ways.
Additionally, MWEngine provides a complete audio processing environment with built-in effects, heck, you _don't even need to know C(++) to use it_.

#### What apps are using MWEngine ?

The engine has been written for both [MikroWave](https://play.google.com/store/apps/details?id=nl.igorski.mikrowave.free&hl=en) and
[Kosm](https://play.google.com/store/apps/details?id=nl.igorski.kosm&hl=en) to provide fast live audio synthesis.

While development on aforementioned apps has (practically) been discontinued, the engine itself has over the years been continuously updated
to be of use to third party app developers, such as [TIZE - Beat Maker, Music Maker](https://play.google.com/store/apps/details?id=com.tizemusic.tize)
and [Another Flamenco Compás App](https://play.google.com/store/apps/details?id=com.harthorst.compas).

### C++ ??? What about Java / Kotlin ?

Though the library is written in C++ (and can be used solely within this context), the library can be built using JNI
(Java Native Interface) which makes its API expose itself to Java / Kotlin, while still executing in a native layer outside of
the JVM. In other words : high performance of the engine is ensured by the native layer operations, while
ease of development is ensured by delegating application logic / UI to the realm of the Android Java SDK.

Whether you intend to use MWEngine for its sample based playback or to leverage its built-in synthesizer and
audio processing, you are not required to write any additional C++ code. If you however intend to create your own
DSP or synthesis routines (which is fun to do!) you must [write them in C++](https://github.com/igorski/MWEngine/wiki/Adding-new-components),
but can rely on SWIG for making them usable in Java.

##### A note on garbage collection and SWIG

It is important to note that when a Java object finalizes (i.e. all its references are broken and is garbage collected), the
destructors of the native objects are invoked, which can lead to unpredictable results if you happen to overlook this!
As such, audio engine objects such as effects processors or events that are created on the Java side, must hold
strong references during their lifecycle.

### The [Issue Tracker](https://github.com/igorski/MWEngine/issues?q=is%3Aissue+is%3Aopen+sort%3Aupdated-desc) is your point of contact

Bug reports, feature requests, questions and discussions are welcome on the GitHub Issue Tracker, please do not send e-mails through the development website. However, please search before posting to avoid duplicates, and limit to one issue per post.

Vote on existing feature requests by using the Thumbs Up/Down reaction on the first post.

### Development setup

You will need the following development kits:

 * [Android SDK](https://developer.android.com/studio/index.html)
 * [Android NDK](https://developer.android.com/ndk/downloads/index.html) to build the native layer code

And the following toolchains:

 * [Gradle](https://gradle.org) to run all build commands
 * [CMake](https://cmake.org) to build the native layer code
 * [SWIG](http://www.swig.org) to wrap the native layer code in Java classes

If you use [Android Studio](https://developer.android.com/studio/) you can simply open the project
folder and sync the project with the _build.gradle_ file, after which you will be prompted in case
you need to install any of the above (as Android Studio can resolve and install the
dependencies for you).

Though depending on your platform, you might need to install SWIG
manually (as well as adding it to your path variables). SWIG is readily available from
most package managers such as _brew_ on macOS or _apt-get_ on Linux).

#### Build configurations

The build configurations are defined in:

 * _build.gradle_ (the usual setup, build and deploy toolchain for Android)
 * _CMakeLists.txt_ (for the native layer code)

If you are uncomfortable with C development, you can ignore the make file as all build commands
are executed through Gradle.
 
#### Building using Android Studio

If you are using Android Studio, creating a project from the supplied _build.gradle_ file should
suffice to get you to build both the native and Java code as well as enabling building, debugging and
packaging an Android application directly from the IDE.

Upon opening the project file, the native layer library should be built automatically (if not,
run _Build > Make Project_), which will create the wrapper Java classes under the
_nl.igorski.mwengine.core_-namespace.

You can build and deploy using debug and release configurations as usual.

**NOTE:** on first run there is a known issue where Android Studio is not aware Java classes have been
generated after completing the native library build (e.g. the _/nl/igorski/mwengine/core_-folder
containing files but the IDE not recognizing the files - though oddly enough compiling just fine -).
Closing and reopening the project window should suffice.

#### Building using CLI

The usual Gradle commands to build and sign a release APK file can be reused. In
case you require more fine grained control (as in: you're building MWEngine
solely as a library and not within a standalone application), you can
execute the following Gradle commands for your CI integration:

##### Cleaning all generated output

```
gradle clean
```

This task will delete the build output as well as previously built native code and generated Java wrappers.

```
gradle externalNativeBuildRelease
```

This task will create a release build of the native layer code and generate the Java wrappers. If you are
not packaging the MWEngine library and wrapped Java code directly into your application and intend to
move the build output as a dependency of another project, you would like to copy these files
in an additional build step:

 * _build/intermediates/cmake/release/obj/*_ 
 * _/src/main/java/nl/igorski/mwengine/*_
 
Where the first contains the native library and its wrapper divided into
subdirectories (one for each supported CPU architecture) and the latter
contains the Java code and interface layer. These can be copied to your
projects source folder, note that native code should go to the _/libs_
folder (if you haven't specified a custom _jniLibs_ location).

Example structure:

```bash
├── libs
│   ├── armeabi-v7a
│   │   ├── libmwengine.so
│   │   └── libmwengine_wrapped.so
│   ├── arm64-v8a
│   │   ├── libmwengine.so
│   │   └── libmwengine_wrapped.so
│   └── x86_64
│       ├── libmwengine.so
│       └── libmwengine_wrapped.so
└── src
    └── main
        └── java 
            └── nl
                └── igorski
                    └── mwengine
                        ├── core
                        │   └── ...java
                        ├── definitions
                        │   └── ...java
                        ├── helpers
                        │   └── ...java
                        └── MWEngine.java
```

Preferably you would make MWEngine's gradle file a submodule of your
custom application Gradle file.

### FAQ / Troubleshooting

The contents of this repository should result in a stable library and example
application. If you experience issues with the setup, consult the
[Troubleshooting Wiki page](https://github.com/igorski/MWEngine/wiki/Troubleshooting-MWEngine).

### Documentation

You can view the Wiki (which documents all of the engine's actors as well as a variety of real world
use cases) here:

[https://github.com/igorski/MWEngine/wiki](https://github.com/igorski/MWEngine/wiki)

Note that you can also view the contents of the header files to get more details about the inner
workings of each class.

### Unit tests

The library comes with unit tests (_/src/main/cpp/tests/_), written using the Googletest C++ testing framework.

To enable unit testing upon each build / during development:

 * update _local.properties_ to include the line: _enable_tests=true_
 
Note: _adb_ must be available in your global path settings and the attached device / emulator
must have the x86_64 CPU architecture (see _CMakeLists.txt_).

### Demo

The repository contains an example Activity that is ready to deploy onto any Android device/emulator supporting ARM-, ARMv7-,
x86- architecture and running Android 4.1 or higher. The example will demonstrate how to quickly get a musical
sequence going using the library.

To install the demo: first build the library as described above, and then run the build script to deploy the .APK onto an
attached device/emulator (note that older emulated devices can only operate at a sample rate of 8 kHz!).

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
