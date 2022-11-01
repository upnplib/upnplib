# Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-11-01

cmake_minimum_required(VERSION 3.18)
include(../../cmake/project-header.cmake)

project(GTESTS_THREADUTIL VERSION 0004
                  DESCRIPTION "Tests for the upnpapi module"
                  HOMEPAGE_URL "https://github.com/upnplib")


# LinkedList
#===========
add_executable(test_LinkedList-psh
        ./test_LinkedList.cpp
)
target_include_directories(test_LinkedList-psh
    PRIVATE
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_LinkedList-psh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_LinkedList-psh
    PRIVATE
        pupnp_shared
)
add_test(NAME ctest_LinkedList-psh COMMAND test_LinkedList-psh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

# ThreadPool
#===========
add_executable(test_ThreadPool-psh
        ./test_ThreadPool.cpp
)
target_include_directories(test_ThreadPool-psh
    PRIVATE
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_ThreadPool-psh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_ThreadPool-psh
    PRIVATE
        pupnp_shared
)
add_test(NAME ctest_ThreadPool-psh COMMAND test_ThreadPool-psh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

# TimerThread
#============
add_executable(test_TimerThread-psh
        ./test_TimerThread.cpp
)
target_include_directories(test_TimerThread-psh
    PRIVATE
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_TimerThread-psh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_TimerThread-psh
    PRIVATE
        pupnp_shared
)
add_test(NAME ctest_TimerThread-psh COMMAND test_TimerThread-psh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)
