# Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-11-04

cmake_minimum_required(VERSION 3.18)
include(../../../cmake/project-header.cmake)

project(GTESTS_COMPA_UTIL VERSION 0001
                  DESCRIPTION "Tests for utilities and helper"
                  HOMEPAGE_URL "https://github.com/upnplib")


# list
#=====
add_executable(test_list-psh
        ./test_list.cpp
)
target_include_directories(test_list-psh
    PRIVATE
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
)
target_compile_definitions(test_list-psh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
)
target_link_libraries(test_list-psh
    PRIVATE
        pupnp_shared
)
add_test(NAME ctest_list-psh COMMAND test_list-psh
    WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# membuffer
#==========
add_executable(test_membuffer
        ./test_membuffer.cpp
)
target_include_directories(test_membuffer
    PRIVATE
        ${UPNPLIB_PROJECT_SOURCE_DIR}
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_IXML_SOURCE_DIR}/inc
)
target_compile_definitions(test_membuffer
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
)
add_test(NAME ctest_membuffer COMMAND test_membuffer
)


# strintmap
#==========
add_executable(test_strintmap-psh
        ./test_strintmap.cpp
)
target_include_directories(test_strintmap-psh
    PRIVATE
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        $<$<NOT:$<PLATFORM_ID:Windows>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${PUPNP_IXML_SOURCE_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_strintmap-psh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
)
target_link_libraries(test_strintmap-psh
    PRIVATE
        pupnp_shared
)
add_test(NAME ctest_strintmap-psh COMMAND test_strintmap-psh
    WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)
