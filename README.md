# The Portable SDK for UPnP\* Devices (libupnp) <!-- omit in toc -->

- [The GitHub project page](https://github.com/pupnp/pupnp) is where all the real action happens.
- [The old Source Forge project page is linked here.](https://sourceforge.net/projects/pupnp)

| branch        | status                                                                                              |
| ------------- | --------------------------------------------------------------------------------------------------- |
| master        | ![master](https://github.com/pupnp/pupnp/workflows/Build/badge.svg)                                 |
| branch-1.14.x | ![1.14.x](https://github.com/pupnp/pupnp/workflows/Build/badge.svg?branch=branch-1.14.x)            |
| branch-1.12.x | ![1.12.x](https://github.com/pupnp/pupnp/workflows/C%2FC%2B%2B%20CI/badge.svg?branch=branch-1.12.x) |
| branch-1.10.x | ![1.10.x](https://github.com/pupnp/pupnp/workflows/C%2FC%2B%2B%20CI/badge.svg?branch=branch-1.10.x) |
| branch-1.8.x  | ![1.8.x](https://github.com/pupnp/pupnp/workflows/C%2FC%2B%2B%20CI/badge.svg?branch=branch-1.8.x)   |
| branch-1.6.x  | ![1.6.x](https://github.com/pupnp/pupnp/workflows/C%2FC%2B%2B%20CI/badge.svg?branch=branch-1.6.x)   |
| branch-1.4.x  | ![1.4.x](https://github.com/pupnp/pupnp/workflows/C%2FC%2B%2B%20CI/badge.svg?branch=branch-1.4.x)   |

Copyright (c) 2000-2003 Intel Corporation - All Rights Reserved.

Copyright (c) 2005-2006 Rémi Turboult <r3mi@users.sourceforge.net>

Copyright (c) 2006 Michel Pfeiffer and others <virtual_worlds@gmx.de>

See [LICENSE](site/LICENSE) for details.

## Table of Contents <!-- omit in toc -->

- [10. Build Instructions](#10-build-instructions)
  - [10.6. Windows Build](#106-windows-build)

## 1. Overview

We pay special attention to embedded devices.

## 10. Build Instructions

### 10.6. Microsoft Windows Build

The developement of this UPnP Library has started on Linux. So for historical reasons it uses POSIX threads (pthreads) as specified by [The Open Group](http://get.posixcertified.ieee.org/certification_guide.html). Unfortunately Microsoft Windows does not support it so we have to use a third party library. We use the well known and well supported [pthreads4w library](https://sourceforge.net/p/pthreads4w). To keep things simple for you I have copied the latest stable version archive to this project. I will be build with the installation of this project and should do out of the box. This archive will always be downloaded but deleted on other systems than MS Windows.

Please note that by default the integration of **pthreads4w** will run its extended test suite. This takes some time so please be patient. You can disable these tests with an option, for example:

    cmake -S . -B build -D BUILDTESTS=OFF

Of course the tests should run as least one time to be sure that **pthreads4w** is working properly on your system.

## 11. Configure Options for cmake

Following the default options are shown.

- -D BUILDTESTS=ON      On configuring there are running tests that take some longer time. This may be anoying if you configure several times. So you can disable (=OFF) the tests but they should run at least one time.

- -D DEBUG=OFF          By default the RELEASE version is build. If you set DEBUG=ON you will have additional developement support: the mnemonic program symbols are compiled into the binary programs so you can better examine the code and simply debug it. Also the storage consuming build directory of **pthreads4w** isn't deleted after installation so you can look at it. Compiling with symbols increases the program size a big amount. With focus on embedded devices this is not e good idea.
