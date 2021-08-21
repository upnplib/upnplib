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
- Using only cmake for installation
- Focus on IPv6 support
- Continous integration with build number

## 2. Build Instructions
### 2.1. Linux
t.b.d.
    cmake -S . -B build
    cmake --build build

### 2.2. Microsoft Windows Build
The developement of this UPnP Library has started on Linux. So for historical reasons it uses POSIX threads (pthreads) as specified by [The Open Group](http://get.posixcertified.ieee.org/certification_guide.html). Unfortunately Microsoft Windows does not support it so we have to use a third party library. We use the well known and well supported [pthreads4w library](https://sourceforge.net/p/pthreads4w). To keep things simple for you I have copied the latest stable version archive to this project. It will be build with the installation of this project and should do out of the box. This archive will always be downloaded but deleted on other systems than MS Windows. To build the UPnPlib just do in the projects source directory

    cmake -S . -B build
    cmake --build build --config Release

After build don't forget to copy the needed `pthreads4w\build\lib\pthread*.dll` library file to a location where your program can find it. Copying it to your programs directory or to the system directory `Windows\System32` will always do.

If you need more details about installation of POSIX threads on Microsoft Windows I have made an example at [github pthreadsWinLin](https://github.com/upnplib/pthreadsWinLin.git).

## 3. Configure Options for cmake
Following the default options are shown.

- -D UPNP_GOOGLETEST=OFF     Enables installation of GoogleTest for Unit-Tests. I recommend to [use shared gtest libraries](https://github.com/upnplib/upnplib/blob/main/gtests/README.md) for this project. But you can still build with default static gtest libraries if you prefer it.

    # recommended shared libs
    cmake -S . -B build -D UPNP_GOOGLETEST=ON -D BUILD_SHARED_LIBS=ON
    # or alternative static libs
    cmake -S . -B build -D UPNP_GOOGLETEST=ON
    cmake --build build

- -D BUILD_SHARED_LIBS=OFF  This option affects only Googletest to build it with shared gtest libraries. UPnPlib is always build shared and static.

<!-- - -D DEVEL=OFF          This enables some additional information for developement. It preserves installation options that normaly will be deleted after Installation for Optimisation so you can examine them. These are mainly the installation directory from **pthread4w** and its temporary installation files even on a non MS Windows environment.

- -D DEBUG=OFF          By default the RELEASE version is build. If you set DEBUG=ON you will have additional developement support: the mnemonic program symbols are compiled into the binary programs so you can better examine the code and simply debug it. But I think you should not use a debugger. Instead write a Unit-Test. Compiling with symbols increases the program size a big amount. With focus on embedded devices this is not good idea.
-->
## Limitations
**TODO:** There is a problem building the program type (Release, Debug, ...) on MS Windows. it cannot be set with a configuration option `cmake -S . -B build -D RELEASE=ON`. It must be set at the build stage, e.g. `cmake --build build --config Release`. But at the build stage this option cannot be given to the Googletest configuration. Until there is found a solution or workaround we are limited to only build the Release type.

<br /><pre>
// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  &#60;Ingo&#64;Hoeft-online.de&#62;
// Redistribution only with this Copyright remark. Last modified: 2021-08-19
</pre>
