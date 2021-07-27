# Copyright 2021 GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Last modified: 2021-07-26

# This is the common header included by all (sub)projects
#--------------------------------------------------------
# Version 3.18 is the version from the Debian 11 (bullseye) repository.
# That must be specified in each project.

# This is a guard that you do not build direct in the source tree.
if("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    message ("In-source builds are not supported.")
    message ("Use a separate folder for building, for example:\n")
    message ("    cmake -S . -B build/")
    message ("    cmake --build build/\n")
    message ("Before that, remove the files already created:\n")
    message ("    rm -rf CMakeCache.txt CMakeFiles\n")
    message (FATAL_ERROR "")
endif()

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
