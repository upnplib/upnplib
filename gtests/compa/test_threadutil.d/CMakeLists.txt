# Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-11-01

cmake_minimum_required(VERSION 3.18)
include(../../../cmake/project-header.cmake)

project(GTESTS_COMPA_THREADUTIL VERSION 0001
                  DESCRIPTION "Tests for the compa threadutil module"
                  HOMEPAGE_URL "https://github.com/upnplib")


# FreeList
#=========
add_executable(test_FreeList_compa-psh
#-------------------------------------
        ./test_FreeList.cpp
)
target_include_directories(test_FreeList_compa-psh
    PRIVATE
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_FreeList_compa-psh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_FreeList_compa-psh
    PRIVATE
        pupnp_shared
        upnplib_shared
)
add_test(NAME ctest_FreeList_compa-psh COMMAND test_FreeList_compa-psh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)
