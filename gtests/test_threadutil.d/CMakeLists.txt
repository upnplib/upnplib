# Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-10-09

cmake_minimum_required(VERSION 3.18)
include(../../cmake/project-header.cmake)

project(GTESTS_THREADUTIL VERSION 0003
                  DESCRIPTION "Tests for the upnpapi module"
                  HOMEPAGE_URL "https://github.com/upnplib")


# LinkedList
#===========
add_executable(test_LinkedList-pnsh
        ./test_LinkedList.cpp
)
target_include_directories(test_LinkedList-pnsh
    PRIVATE
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_LinkedList-pnsh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_LinkedList-pnsh
    PRIVATE
        pupnp_native_shared
        upnplib_native_shared
)
add_test(NAME ctest_LinkedList-pnsh COMMAND test_LinkedList-pnsh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

# FreeList
#=========
add_executable(test_FreeList-pnsh
#--------------------------------
        ./test_FreeList.cpp
)
target_include_directories(test_FreeList-pnsh
    PRIVATE
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${UPnPlib_SOURCE_DIR}/include
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_FreeList-pnsh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_FreeList-pnsh
    PRIVATE
        pupnp_native_shared
        upnplib_native_shared
)
add_test(NAME ctest_FreeList-pnsh COMMAND test_FreeList-pnsh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

add_executable(test_FreeList-ucsh
#--------------------------------
    ./test_FreeList.cpp
)
target_include_directories(test_FreeList-ucsh
    PRIVATE
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${UPnPlib_SOURCE_DIR}/include
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_FreeList-ucsh
    PRIVATE
        UPNPLIB_SHARED
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_FreeList-ucsh
    PRIVATE
        upnplib_compa_shared
        upnplib_native_shared
)
add_test(NAME ctest_FreeList-ucsh COMMAND test_FreeList-ucsh
    WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

# ThreadPool
#===========
add_executable(test_ThreadPool-pnsh
        ./test_ThreadPool.cpp
)
target_include_directories(test_ThreadPool-pnsh
    PRIVATE
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_ThreadPool-pnsh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_ThreadPool-pnsh
    PRIVATE
        pupnp_native_shared
        upnplib_native_shared
)
add_test(NAME ctest_ThreadPool-pnsh COMMAND test_ThreadPool-pnsh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

# TimerThread
#============
add_executable(test_TimerThread-pnsh
        ./test_TimerThread.cpp
)
target_include_directories(test_TimerThread-pnsh
    PRIVATE
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_TimerThread-pnsh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_TimerThread-pnsh
    PRIVATE
        pupnp_native_shared
        upnplib_native_shared
)
add_test(NAME ctest_TimerThread-pnsh COMMAND test_TimerThread-pnsh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)
