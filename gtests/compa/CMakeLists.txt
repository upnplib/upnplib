# Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-10-25

cmake_minimum_required(VERSION 3.18)
include(../../cmake/project-header.cmake)

project(GTESTS_COMPA VERSION 0008
                  DESCRIPTION "Tests for pupnp native and compatible code"
                  HOMEPAGE_URL "https://github.com/upnplib")


# UpnpString
#===========
add_executable(test_UpnpString-psh
#---------------------------------
        ./test_UpnpString.cpp
)
target_include_directories(test_UpnpString-psh
    PRIVATE
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
)
target_compile_definitions(test_UpnpString-psh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_UpnpString-psh
    PRIVATE
        pupnp_shared
        upnplib_native_shared
)
add_test(NAME ctest_UpnpString-psh COMMAND test_UpnpString-psh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

add_executable(test_UpnpString-csh
#---------------------------------
        ./test_UpnpString.cpp
)
target_include_directories(test_UpnpString-csh
    PRIVATE
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
        ${UPnPlib_SOURCE_DIR}/include
        ${UPnPlib_SOURCE_DIR}/compa/inc
        ${PUPNP_UPNP_SOURCE_DIR}/inc
)
target_compile_definitions(test_UpnpString-csh
    PRIVATE
        UPNPLIB_SHARED
)
target_link_libraries(test_UpnpString-csh
    PRIVATE
        compa_shared
        upnplib_native_shared
)
add_test(NAME ctest_UpnpString-csh COMMAND test_UpnpString-csh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# FreeList
#=========
add_executable(test_FreeList_compa-psh
#-------------------------------------
        ./test_threadutil.d/test_FreeList.cpp
)
target_include_directories(test_FreeList_compa-psh
    PRIVATE
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${UPnPlib_SOURCE_DIR}/include
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
        upnplib_native_shared
)
add_test(NAME ctest_FreeList_compa-psh COMMAND test_FreeList_compa-psh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

if(FALSE) # TODO: There is already another test for FreeList
add_executable(test_FreeList_compa-csh
#-------------------------------------
    ./test_threadutil.d/test_FreeList.cpp
)
target_include_directories(test_FreeList_compa-csh
    PRIVATE
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${UPnPlib_SOURCE_DIR}/include
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_FreeList_compa-csh
    PRIVATE
        UPNPLIB_SHARED
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_FreeList_compa-csh
    PRIVATE
        compa_shared
        upnplib_native_static
)
add_test(NAME ctest_FreeList_compa-csh COMMAND test_FreeList_compa-csh
    WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)
endif()
