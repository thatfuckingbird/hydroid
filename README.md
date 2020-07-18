# hydroid
A Hydrus client.

## Building from source

### Dependencies

You will need the following Qt modules:
* core
* quick
* quickcontrols2 
* svg
* network
* androidextras (only if building for Android)

The only version of Qt that works is Qt 5.15. Do not attempt to compile on older versions, it will not succeed.

### Building for Linux

The simplest way is just issuing the `qmake` then `make` commands in the Hydroid folder.
Alternatively (or if you want to debug), if you have QtCreator set up, you can open the project file with it and build inside the IDE.

### Building for Android or WebAssembly

The easiest way to get these working is to download the official Qt installer and install the precompiled Qt versions for these platforms (and also QtCreator).
For WebAssembly, you will also need to install the Emscripten SDK. Make sure that you use the Emscripten version that is supported with the Qt version you installed (your builds will be subtly broken otherwise).
After you have set up QtCreator to be able to build for these platforms, you can open the Hydroid project file and set up build targets for these platforms. No further configuration specific to Hydroid is necessary.
