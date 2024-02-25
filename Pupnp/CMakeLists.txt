# Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2024-02-26

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(PUPNP VERSION 0003
                  DESCRIPTION "Old code from the pupnp library"
                  HOMEPAGE_URL "https://github.com/pupnp")


add_compile_options(
    # Default warning level
    $<$<CXX_COMPILER_ID:MSVC>:/W1>
    # disable warning: 'sprintf' (and others) is deprecated: This function is
    # provided for compatibility reasons only.
    # We do not fix deprecated functions on old code.
    $<$<CXX_COMPILER_ID:AppleClang>:-Wno-deprecated-declarations>
)

add_subdirectory(${PROJECT_SOURCE_DIR}/ixml)
if(UPNPLIB_WITH_NATIVE_PUPNP)
    add_subdirectory(${PROJECT_SOURCE_DIR}/upnp)
endif()
