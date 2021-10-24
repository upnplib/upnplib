# The UPnP Library
**Under developement!**  
Not usable at time. README.md only as draft, work in progress.

## 1. Overview
This is a fork of the [Portable SDK for UPnP Devices (pupnp)](https://github.com/pupnp/) with the aim of a complete re-engeneering. The following general goals are in progress or planned:
- Drop in compatibillity with the old pupnp library
- Continue optimization for embedded devices
- Based on C++ instead of C
- Object oriented programming
- Unit-Test driven developement
- Using CMake for managing the code
- Focus on IPv6 support

## 2. Version numbering
We follow the [Semantic Versioning](https://semver.org/spec/v2.0.0.html#semantic-versioning-200).

The last release before forking is [release-1.14.2](https://github.com/upnplib/upnplib/releases/tag/release-1.14.2) from the pupnp library. Because it is intended to be backwards compatible on the API we start with release number 1.14.0 for this repository to reflect it by following
- MAJOR version when you make incompatible API changes,
- MINOR version when you add functionality in a backwards compatible manner, and
- PATCH version when you make backwards compatible bug fixes.

Because we will use CMake to manage the code it can be seen as integral part of it. The version number will also include changes to the CMake system.

## 3. Build Instructions
If nothing others said we are always staying at the root directory of the project (CMAKE_SOURCE_DIR), that is **upnplib/** if you don't changed the name.

### 3.1. Linux build
t.b.d.

    upnplib$ cmake -S . -B build
    upnplib$ cmake --build build

As noted by cmake to clean up a build you will have to delete all build directories on the main project and all subprojects. I do it with:

    upnplib$ find -type d -name build -exec rm -rf {} \;

### 3.2. Microsoft Windows Build
The developement of this UPnP Library has started on Linux. So for historical reasons it uses POSIX threads (pthreads) as specified by [The Open Group](http://get.posixcertified.ieee.org/certification_guide.html). Unfortunately Microsoft Windows does not support it so we have to use a third party library. We use the well known and well supported [pthreads4w library](https://sourceforge.net/p/pthreads4w). To keep things simple for you I have copied the latest stable version archive to this project. It will be build with the installation of this project and should do out of the box. This archive will always be downloaded but deleted on other systems than MS Windows. To build the UPnPlib just do

    upnplib$ cmake -S . -B build
    upnplib$ cmake --build build --config MinSizeRel

After build don't forget to copy the needed `pthreads4w\build\lib\pthread*.dll` library file to a location where your program can find it. Copying it to your programs directory or to the system directory `Windows\System32` will always do.

As noted by cmake to clean up a build you will have to delete all build directories on the main project and all subprojects.

If you need more details about installation of POSIX threads on Microsoft Windows I have made an example at [github pthreadsWinLin](https://github.com/upnplib/pthreadsWinLin.git).

### 3.3 Googletest build
I recommend to [use shared gtest libraries](https://github.com/upnplib/upnplib/blob/main/gtests/README.md) for this project. But you can still build with default static gtest libraries if you prefer it.

    # recommended shared libs
    upnplib$ cmake -S . -B build -D UPNP_GOOGLETEST=ON -D BUILD_SHARED_LIBS=ON
    # or alternative static libs
    upnplib$ cmake -S . -B build -D UPNP_GOOGLETEST=ON
    upnplib$ cmake --build build

Please note that option BUILD_SHARED_LIBS only effects Googletest. By default program type **MinSizeRel** (Minimal Size Release) is build. The build type is cached so Googletest will build the last selected type. This may be confusing if you rebuild Googletest without deleting all build directories. So on rebuild you should always specify all wanted options, for example enable Googletest with static libraries on a previous build:

    # On Linux
    upnplib$ cmake -S . -B build -D CMAKE_BUILD_TYPE=Debug
    upnplib$ cmake --build build
    upnplib$ cmake -S . -B build -D UPNP_GOOGLETEST=ON -D BUILD_SHARED_LIBS=OFF -D CMAKE_BUILD_TYPE=MinSizeRel
    upnplib$ cmake --build build

    # On MS Windows
    # t.b.d.

## 4. Configure Options for cmake
Option prefixed with -D | Default | Description
-------|---------|---
UPNP_GOOGLETEST=[ON\|OFF] | OFF | Enables installation of GoogleTest for Unit-Tests. For details look at section *Googletest build*.
BUILD_SHARED_LIBS=[ON\|OFF] | OFF | This option affects only Googletest to build it with shared gtest libraries. UPnPlib is always build shared and static.
CMAKE_BUILD_TYPE=[Debug\| Release\| MinSizeRel\| RelWithDebInfo] | MinSizeRel | If you set this option to **Debug** you will have additional developement support. The mnemonic program symbols are compiled into the binary programs so you can better examine the code and simply debug it. But I think it is better to write a Unit Test instead of using a debugger. Compiling with symbols increases the program size a big amount. With focus on embedded devices this is a bad idea.
PT4W_BUILD_TESTING=[ON\|OFF] | OFF | Runs the testsuite of pthreads4w (PT4W) with nearly 1000 tests. It will take some time but should be done at least one time.

<!-- - -D DEVEL=OFF          This enables some additional information for developement. It preserves installation options that normaly will be deleted after Installation for Optimisation so you can examine them. These are mainly the installation directory from **pthread4w** and its temporary installation files even on a non MS Windows environment.
-->
## 5. Limitations
No limits documented so far.

<pre><sup>
// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  &#60;Ingo&#64;Hoeft-online.de&#62;
// Redistribution only with this Copyright remark. Last modified: 2021-09-29
</sup></sup>
