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

The fork is based on [release-1.14.18](https://github.com/pupnp/pupnp/releases/tag/release-1.14.18) from the pupnp library. Because it is intended to be backwards compatible on the API we start with release number 1.14.0 for this repository to reflect it by following
- MAJOR version when you make incompatible API changes,
- MINOR version when you add functionality in a backwards compatible manner, and
- PATCH version when you make backwards compatible bug fixes.

Because we will use CMake to manage the code it can be seen as integral part of it. The version number will also include changes to the CMake system.

## 3. Milestones
- Ongoing: create extensive Unit Tests for IP4 without modification of the old source code
- Ongoing: define C++ interfaces for the API
- ToDo:    change old C program to C++ objects but preserve drop in compatibility
- ToDo:    support IP6

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

## 5. Build Instructions
If nothing others said we are always staying at the root directory of the project (CMAKE_SOURCE_DIR), that is **upnplib/** if you don't changed the name.

### 5.1. Linux build
t.b.d.

    upnplib$ cmake -S . -B build
    upnplib$ cmake --build build

As noted by cmake to clean up a build you will have to delete all build directories on the main project and all subprojects. I do it with:

    upnplib$ find -type d -name build -exec rm -rf {} \;

### 5.2. Microsoft Windows Build
The developement of this UPnP Library has started on Linux. So for historical reasons it uses POSIX threads (pthreads) as specified by [The Open Group](http://get.posixcertified.ieee.org/certification_guide.html). Unfortunately Microsoft Windows does not support it so we have to use a third party library. We use the well known and well supported [pthreads4w library](https://sourceforge.net/p/pthreads4w). To keep things simple for you I have copied the latest stable version archive to this project. It will be build with the installation of this project and should do out of the box. This archive will always be downloaded but deleted on other systems than MS Windows. To build the UPnPlib just do

    upnplib$ cmake -S . -B build
    upnplib$ cmake --build build --config Release

After build don't forget to copy the needed `./build/_deps/pthreads4w-build/pthread*.dll` library file to a location where your program can find it. Copying it to your programs directory or to the system directory `Windows\System32` will always do. Within the project developement directory tree (default root ./upnplib/) built programs and libraries find its dll files. There is nothing to do.

As noted by cmake to clean up a build you will have to delete the ./build/ directory on the main project.

If you need more details about installation of POSIX threads on Microsoft Windows I have made an example at [github pthreadsWinLin](https://github.com/upnplib/pthreadsWinLin.git).

### 5.3 Googletest build
I recommend to [use shared gtest libraries](https://github.com/upnplib/upnplib/blob/main/gtests/README.md) for this project. But you can still build with default static gtest libraries if you prefer it.

    # recommended shared libs
    upnplib$ cmake -S . -B build -D UPNP_GOOGLETEST=ON -D BUILD_SHARED_LIBS=ON
    # or alternative static libs
    upnplib$ cmake -S . -B build -D UPNP_GOOGLETEST=ON
    upnplib$ cmake --build build

Please note that option BUILD_SHARED_LIBS only effects Googletest. By default program type **Release** is build. The build type is cached so Googletest will build the last selected type. This may be confusing if you rebuild Googletest without deleting all build directories. So on rebuild you should always specify all wanted options, for example enable Googletest with static libraries on a previous build:

    # On Linux
    upnplib$ cmake -S . -B build -D CMAKE_BUILD_TYPE=Debug
    upnplib$ cmake --build build
    upnplib$ cmake -S . -B build -D UPNP_GOOGLETEST=ON -D BUILD_SHARED_LIBS=OFF -D CMAKE_BUILD_TYPE=Release
    upnplib$ cmake --build build

    # On MS Windows
    # t.b.d.
    # To find the dll files for the test executables add its directory to the path. For example on PowerShell I use:
    PS> $env:path_orig = $env:path
    PS> $env:Path += ';C:\Users\ingo\devel\upnplib\build\bin\Release'
    PS> .\build\gtests\Release\test_template.exe

## 5. Security
The following security issues are taken into account:
### Reuse address
For security and stability resons there is a delay when a server closes a network conection. By default it accepts a new connection on its same local interface address only after a TIME_WAIT. But many server want to be available immediately again on the network with the risk of network communication problems. They use a socket option that can be set to reuse the address, [Bind: Address Already in Use](https://hea-www.harvard.edu/~fine/Tech/addrinuse.html). This library prefers security and do not use this option. A TIME_WAIT is not necessary if the remote peer closes the connection. Upnplib uses the protocol to signal the remote peer to shutdown the connection if it support it.
### Socket network security on Microsoft Windows
In the past Windows had a very bad network security on low level socket handling as documented at [Using SO_REUSEADDR and SO_EXCLUSIVEADDRUSE](https://learn.microsoft.com/en-us/windows/win32/winsock/using-so-reuseaddr-and-so-exclusiveaddruse). Microsoft has fixed this since Windows Server 2003. The network stack now behaves like it does by default on other platforms. Strangely, they call it advanced security, which is standard on other major operating systems. But this isn't done by defauft. The developer has to take this into account by using the socket option SO_EXCLUSIVEADDRUSE. This has be done.

(Will be continued)

## 6. Configure Options for cmake
Option prefixed with -D | Default | Description
-------|---------|---
UPNP_GOOGLETEST=[ON\|OFF] | OFF | Enables installation of GoogleTest for Unit-Tests. For details look at section *Googletest build*.
BUILD_SHARED_LIBS=[ON\|OFF] | OFF | This option affects only Googletest to build it with shared gtest libraries. UPnPlib is always build shared and static.
CMAKE_BUILD_TYPE=[Debug\| Release\| MinSizeRel\| RelWithDebInfo] | Release | If you set this option to **Debug** you will have additional developement support. The mnemonic program symbols are compiled into the binary programs so you can better examine the code and simply debug it. But I think it is better to write a Unit Test instead of using a debugger. Compiling with symbols increases the program size a big amount. With focus on embedded devices this is a bad idea.
PT4W_BUILD_TESTING=[ON\|OFF] | OFF | Runs the testsuite of pthreads4w (PT4W) with nearly 1000 tests. It will take some time but should be done at least one time.

<!-- - -D DEVEL=OFF          This enables some additional information for developement. It preserves installation options that normaly will be deleted after Installation for Optimisation so you can examine them. These are mainly the installation directory from **pthread4w** and its temporary installation files even on a non MS Windows environment.
-->
## 7. Limitations
No limits documented so far.

<pre><sup>
// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  &#60;Ingo&#64;Hoeft-online.de&#62;
// Redistribution only with this Copyright remark. Last modified: 2023-08-25
</sup></sup>
