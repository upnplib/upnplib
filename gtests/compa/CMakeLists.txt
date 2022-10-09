# Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-10-12

cmake_minimum_required(VERSION 3.18)
include(../../cmake/project-header.cmake)

project(GTESTS_COMPA VERSION 0008
                  DESCRIPTION "Tests for pupnp native and compatible code"
                  HOMEPAGE_URL "https://github.com/upnplib")


# UpnpString
#===========
add_executable(test_UpnpString-pnsh
#----------------------------------
        ./test_UpnpString.cpp
)
target_include_directories(test_UpnpString-pnsh
    PRIVATE
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
)
target_compile_definitions(test_UpnpString-pnsh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_UpnpString-pnsh
    PRIVATE
        pupnp_native_shared
)
add_test(NAME ctest_UpnpString-pnsh COMMAND test_UpnpString-pnsh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

add_executable(test_UpnpString-ucsh
#----------------------------------
        ./test_UpnpString.cpp
)
target_include_directories(test_UpnpString-ucsh
    PRIVATE
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
        ${UPnPlib_SOURCE_DIR}/include
        ${UPNPLIB_CORE_SOURCE_DIR}/compa/inc
        ${PUPNP_UPNP_SOURCE_DIR}/inc
)
target_compile_definitions(test_UpnpString-ucsh
    PRIVATE
        UPNPLIB_SHARED
)
target_link_libraries(test_UpnpString-ucsh
    PRIVATE
        upnplib_compa_shared
)
add_test(NAME ctest_UpnpString-ucsh COMMAND test_UpnpString-ucsh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# FreeList
#=========
add_executable(test_FreeList_compa-pnsh
#--------------------------------
        ./test_threadutil.d/test_FreeList.cpp
)
target_include_directories(test_FreeList_compa-pnsh
    PRIVATE
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${UPnPlib_SOURCE_DIR}/include
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_FreeList_compa-pnsh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_FreeList_compa-pnsh
    PRIVATE
        pupnp_native_shared
        #upnplib_native_shared
)
add_test(NAME ctest_FreeList_compa-pnsh COMMAND test_FreeList_compa-pnsh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

if(FALSE)
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
        upnplib_native_static
)
add_test(NAME ctest_FreeList-ucsh COMMAND test_FreeList-ucsh
    WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)
endif()
