MWEngine is..
=============

...an audio engine for Android written in C++, using OpenSL for low latency performance. As used by MikroWave.

### Build instructions

The makefile has been written to build the library using the Android NDK (Native Development Kit) in conjunction with SWIG. You CAN
use the library and write the application solely using C++, or you can rely on the Android SDK and write your application in Java, and
enjoy the SWIG generated wrappers to construct the native layer audio engine objects and use Java as a convenient API.

Those of a Unix-bent can immediately un the _build.sh_-file in the /src-folder. Who will supply a Windows .BAT to run the makefile ? ;-)

### TODO:

this repository is constantly being updated and as such so is the documentation. Planned are:

 * Wiki pages documenting basic engine architecture
 * Example Activity showing how to quickly get sound