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

#### How does this relate/compare to Google Oboe ?

Though MWEngine was initially created before Oboe, its underlying audio drivers are _the same as Google Oboe uses_, MWEngine
and Oboe are merely different abstraction layers to solve the same problem.

Additionally, MWEngine provides a complete audio sequencing and processing environment with built-in effects
without you _needing to write/know C(++) to use it_.

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

Bug reports, feature requests, questions and discussions are welcome on the GitHub Issue Tracker, please _do not send e-mails through the development website_. Please search before posting to avoid duplicates and limit to one issue per post.
For usage / implementation related questions, first consult [the MWEngine Wiki](https://github.com/igorski/MWEngine/wiki).

When reporting a bug, please describe the expected behaviour and the actual result. When possible, for crashes attach stack traces and recordings for audio related glitches.

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

This repository contains two modules:

 * _/mwengine/_ which is the core MWEngine library (native code, Java API wrappers)
 * _/mwengine_example/_ which contains an example Activity bundling the library into an Android app

Both are referenced in the root level _settings.gradle_ file for a standard multi module setup.

The build configurations themselves are defined in:

 * _build.gradle_ (the usual setup, build and deploy toolchain for Android, per module)
 * _CMakeLists.txt_ (for the native layer code, solely in /mwengine module)

If you are uncomfortable with C development, you can ignore the makefile as all build commands
are executed through Gradle.

#### Building the project

Upon checkout this repository does not include the Java API files (_nl.igorski.mwengine.core_-namespace) as
these are wrappers and generated by the build. In order to generate these files, you should run the _assemble_
task of the _mwengine_-module, e.g.:

```:mwengine:assemble```.

Upon completion, you can run usual debug/deploy targets for the _mwengine_example_-module to start the example application.
Using Android Studio you can easily debug native code of the library inside the example Activity using breakpoints.

#### Building MWEngine as a standalone AAR library

While you could create your own custom app by cloning this repository and refactoring the example Activity to
your needs, it will be far easier to maintain and include future MWEngine updates when you build the core MWEngine
library as an Android Archive (.AAR) and reference the library within your project.

In order to do so, you run the following Gradle command:

```:mwengine:assemble```

Which will generate the library in both debug and release configurations, packaged as AAR files in: _./mwengine/build/outputs/aar/_.

##### Importing the MWEngine AAR library inside your custom project

Within Android Studio you can easily do this by importing the generated .aar file by navigating through:

```File > Project structure > Dependencies```

Add a new `Library Dependency` in the `Declared dependencies` tab, select `Jar Dependency`
In the `Add Jar/Aar Dependency` dialog, enter the path to your built AAR library `/path/to/mwengine-release.aar`

Your projects _build.gradle_ file will now contain the following line:

```implementation files('/path/to/mwengine-release.aar')```

In the _build.gradle_ for your application, be sure to add the following entries under the defaultConfig and dependencies sections:

```
android {
    defaultConfig {
        // project specific custom stuff here...
        ndk {
            // these values must match the ABI's defined in mwengine/build.gradle
            // to prevent UnsatisfiedLinkError when this app tries to serve an unsupported architecture
            abiFilters "armeabi-v7a", "arm64-v8a", "x86_64"
        }
    }
}
dependencies {
    implementation project(":mwengine-release")
    // project specific custom stuff here...
}
```

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
