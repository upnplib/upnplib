# The UPnP Library
**Under developement!**
Not usable at time. README.md only as draft, work in progress.

## 1. Overview
This is a fork of the <a href="https://github.com/pupnp/">Portable SDK for UPnP Devices (pupnp)</a> with the aim of a complete re-engeneering based on <a href="UPnP-arch-DeviceArchitecture-v2.0-20200417.pdf">UPnP™ Device Architecture 2.0</a>. The following general goals are in progress or planned:
- Drop in compatibillity with the old pupnp library
- Continue optimization for embedded devices
- Based on C++ instead of C
- Object oriented programming
- Unit-Test driven developement
- Using CMake for managing the code
- Focus on IPv6 support

## 2. Technical Documentation
Here you can find the [**Technical Documentation**](pages.html).\n
If you want to be compatible with the classic pUPnP library here you find the [Compatible API](\ref compaAPI).\n
To use the new written object oriented part of the library here you find its [UPnPlib API](\ref upnplibAPI).

<!--
## 2. Version numbering
We follow the [Semantic Versioning](https://semver.org/spec/v2.0.0.html#semantic-versioning-200).

The fork is based on [release-1.14.18](https://github.com/pupnp/pupnp/releases/tag/release-1.14.18) from the pupnp library. Because it is intended to be backwards compatible on the API we start with release number 1.14.0 for this repository to reflect it by following
- MAJOR version when you make incompatible API changes,
- MINOR version when you add functionality in a backwards compatible manner, and
- PATCH version when you make backwards compatible bug fixes.

Because we will use CMake to manage the code it can be seen as integral part of it. The version number will also include changes to the CMake system.
-->

## 3. Milestones
- Ongoing: create extensive Unit Tests without modification of the old source code
- Ongoing: define C++ interfaces for the API
- Ongoing: change old C program to C++ objects but preserve drop in compatibility
- Ongoing: support IP6
- Ongoing: support OpenSSL

<!--
## 4. Cmake subprojects
                                      UPnPlib
                                         |
            +---------------+------------+-------------+----------------+
            |               |            |             |                |
       UPNPLIB_CORE    UPNPLIB_IXML    PUPNP    UPNPLIB_GTESTS    UPNPLIB_SAMPLE
                                       /   \
                              PUPNP_UPNP   PUPNP_IXML
                                              \
                                             PUPNP_IXML_TEST

These names are also the names of the CMake subprojects.
-->

## 4. Build Instructions
If nothing others said we are always staying at the root directory of the project (CMAKE_SOURCE_DIR), that is **upnplib_project/** if you don't changed the name.

### 4.1. Linux and MacOS build
First configure then build:

    upnplib_project$ cmake -S . -B build -D UPNPLIB_WITH_SAMPLES=YES
    upnplib_project$ cmake --build build

To clean up a build just delete the build folder:

    upnplib_project$ rm -rf build

### 4.2. Microsoft Windows build
The developement of this UPnP Library has started on Linux. So for historical reasons it uses POSIX threads (pthreads) as specified by [The Open Group](http://get.posixcertified.ieee.org/certification_guide.html). Unfortunately Microsoft Windows does not support it so I have to use a third party library. I use the well known and well supported [pthreads4w library](https://sourceforge.net/p/pthreads4w). It will be downloaded on Microsoft Windows and compiled with building the project and should do out of the box. To build the UPnPlib you need a Developer Command Prompt. How to install it is out of scope of this description. Microsoft has good documentation about it. For example this is the prompt I used:

    **********************************************************************
    ** Visual Studio 2019 Developer Command Prompt v16.9.5
    ** Copyright (c) 2021 Microsoft Corporation
    **********************************************************************
    [vcvarsall.bat] Environment initialized for: 'x64'

    ingo@WIN10-DEVEL C:\Users\ingo> pwsh
    PowerShell 7.4.1
    PS C:\Users\ingo>

First configure then build:

    PS C: upnplib_project> cmake -S . -B build -D UPNPLIB_WITH_SAMPLES=YES
    PS C: upnplib_project> cmake --build build --config Release

After build don't forget to copy the needed `./build/_deps/pthreads4w-build/pthread*.dll` library file to a location where your program can find it. Copying it to your programs directory or to the system directory `Windows\System32` will always do. Within the project developement directory tree (default root upnplib_project/) built programs and libraries find its dll files. There is nothing to do.

To clean up a build just delete the build folder:

    PS C: upnplib_project> rm -rf build

If you need more details about installation of POSIX threads on Microsoft Windows I have made an example at [github pthreadsWinLin](https://github.com/upnplib/pthreadsWinLin.git).

### 4.3 Googletest build
I strongly recommend to use shared gtest libraries for this project because there are situations where static and shared libraries are linked together. Using static linked Googletest libraries may fail then. If you know what you ar doing and you are able to manage possible linker errors you can try to use static built Googletest libraries.

    # strongly recommended shared libs
    upnplib_project$ cmake -S . -B build -D CMAKE_BUILD_TYPE=Debug -D UPNPLIB_WITH_GOOGLETEST=ON
    upnplib_project$ cmake --build build --config Debug

    # or alternative static libs
    upnplib_project$ cmake -S . -B build -D CMAKE_BUILD_TYPE=Debug -D UPNPLIB_WITH_GOOGLETEST=ON -D GTESTS_WITH_SHARED_LIBS=OFF
    upnplib_project$ cmake --build build --config Debug

Using build type "Debug" is not necessary for Googletest but it will enable additional debug messages from the library. if you don't need it you can just use "Release" instead of "Debug" above as option.

<!--
## 5. Configure Options for cmake
Option prefixed with -D | Default | Description
-------|---------|---
UPNP_GOOGLETEST=[ON\|OFF] | OFF | Enables installation of GoogleTest for Unit-Tests. For details look at section *Googletest build*.
BUILD_SHARED_LIBS=[ON\|OFF] | OFF | This option affects only Googletest to build it with shared gtest libraries. UPnPlib is always build shared and static.
CMAKE_BUILD_TYPE=[Debug\| Release\| MinSizeRel\| RelWithDebInfo] | Release | If you set this option to **Debug** you will have additional developement support. The mnemonic program symbols are compiled into the binary programs so you can better examine the code and simply debug it. But I think it is better to write a Unit Test instead of using a debugger. Compiling with symbols increases the program size a big amount. With focus on embedded devices this is a bad idea.
PT4W_BUILD_TESTING=[ON\|OFF] | OFF | Runs the testsuite of pthreads4w (PT4W) with nearly 1000 tests. It will take some time but should be done at least one time.

- -D DEVEL=OFF          This enables some additional information for developement. It preserves installation options that normaly will be deleted after Installation for Optimisation so you can examine them. These are mainly the installation directory from **pthread4w** and its temporary installation files even on a non MS Windows environment.
-->
## 5. Limitations
No limits documented so far.

<pre><sup>
Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, \<Ingo@Hoeft-online.de>
Redistribution only with this Copyright remark. Last modified: 2024-04-21
</sup></pre>
