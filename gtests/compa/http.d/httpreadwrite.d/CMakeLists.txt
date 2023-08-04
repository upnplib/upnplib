# Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2023-08-04

cmake_minimum_required(VERSION 3.18)
include(../../../../cmake/project-header.cmake)

project(GTESTS_COMPA_HTTP_HTTPREADWRITE VERSION 0001
                  DESCRIPTION "Tests for the compa httpreadwrite module"
                  HOMEPAGE_URL "https://github.com/upnplib")


# httpreadwrite incl. statcodes
#==============================
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts. We must use static libraries.
if(FALSE)
# Direct linking with source files currently not used but having available for
# reference. We link against the internal upnplib libraries (see below).
add_executable(test_httpreadwrite
    ${PUPNP_UPNP_SOURCE_DIR}/src/global.cpp
    ${GTESTS_SOURCE_DIR}/tools/mockObj.cpp
    $<$<BOOL:${WIN32}>:${GTESTS_SOURCE_DIR}/tools/mockObj_win32.cpp>

    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/FreeList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/LinkedList.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/sock.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/uri/uri.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/httpparser.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/statcodes.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/list.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/membuffer.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/strintmap.cpp

    # ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpExtraHeaders.c
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpString.cpp
    $<$<CONFIG:Debug>:${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpdebug.cpp>

    ${UPNPLIB_SOURCE_DIR}/src/api/upnptools.cpp
    ./test_httpreadwrite.cpp
)
endif()

add_executable(test_httpreadwrite-pst
#------------------------------------
    ./test_httpreadwrite.cpp
)
target_include_directories(test_httpreadwrite-pst
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_compile_options(test_httpreadwrite-pst
    PRIVATE
        # To fix these warnigs we have to modify the original pupnp code but
        # that isn't intended. So the warnings are suppressed.
        #
        # disable warning: 'sprintf' (and others) is deprecated: This function is
        # provided for compatibility reasons only.
        $<$<CXX_COMPILER_ID:AppleClang>:-Wno-deprecated-declarations>
        # disable warning C4505: 'upnplib::Check_Connect_And_Wait_Connection':
        # unreferenced local function has been removed
        $<$<CXX_COMPILER_ID:MSVC>:/wd4505>
        # disable warning C4706: assignment within conditional expression
        $<$<CXX_COMPILER_ID:MSVC>:/wd4706>
)
target_link_libraries(test_httpreadwrite-pst
    PRIVATE
        pupnp_static
        upnplib_static
)
add_test(NAME ctest_httpreadwrite-pst COMMAND test_httpreadwrite-pst --gtest_shuffle
)


# httpreadwrite: netconnect
#==========================
# This is also part of httpreadwrite but splitted to have a better overview.
# It's intended to make netconnect a separate modul because it seems to be a
# general base function.
#
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts. We must use static libraries.
if(FALSE)
# Direct linking with source files currently not used but having available for
# reference. We link against the upnplib libraries (see below).
add_executable(test_netconnect
    ${PUPNP_UPNP_SOURCE_DIR}/src/global.cpp
    ${GTESTS_SOURCE_DIR}/tools/mockObj.cpp
    $<$<BOOL:${WIN32}>:${GTESTS_SOURCE_DIR}/tools/mockObj_win32.cpp>

    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/FreeList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/LinkedList.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/sock.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/uri/uri.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/httpparser.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/statcodes.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/list.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/membuffer.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/strintmap.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpString.cpp
    $<$<CONFIG:Debug>:${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpdebug.cpp>

    test_httpreadwrite.d/test_netconnect.cpp
)
endif()

add_executable(test_netconnect-pst
#---------------------------------
    ./test_netconnect.cpp
)
target_include_directories(test_netconnect-pst
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_compile_options(test_netconnect-pst
    PRIVATE
        # To fix these warnigs we have to modify the original pupnp code but
        # that isn't intended. So the warnings are suppressed.
        # disable warning: 'sprintf' (and others) is deprecated: This function is
        # provided for compatibility reasons only.
        $<$<CXX_COMPILER_ID:AppleClang>:-Wno-deprecated-declarations>
        # disable warning C4706: assignment within conditional expression
        $<$<CXX_COMPILER_ID:MSVC>:/wd4706>
)
target_link_libraries(test_netconnect-pst
    PRIVATE pupnp_static
)
add_test(NAME ctest_netconnect-pst COMMAND test_netconnect-pst --gtest_shuffle
)

add_executable(test_netconnect-cst
#---------------------------------
    ./test_netconnect.cpp
)
target_include_directories(test_netconnect-cst
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_link_libraries(test_netconnect-cst
    PRIVATE compa_static
)
add_test(NAME ctest_netconnect-cst COMMAND test_netconnect-cst --gtest_shuffle
)


# httpreadwrite: http_Download
#=============================
# This is also part of httpreadwrite but splitted to have a better overview.
#
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts. We must use static libraries.
add_executable(test_http_Download-pst
#---------------------------------
    ./test_http_Download.cpp
)
target_include_directories(test_http_Download-pst
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_compile_options(test_http_Download-pst
    PRIVATE
        # disable warning: 'sprintf' (and others) is deprecated: This function is
        # provided for compatibility reasons only.
        $<$<CXX_COMPILER_ID:AppleClang>:-Wno-deprecated-declarations>
        # disable warning C4706: assignment within conditional expression
        $<$<CXX_COMPILER_ID:MSVC>:/wd4706>
)
target_link_libraries(test_http_Download-pst
    PRIVATE
        pupnp_static
        upnplib_static
)
add_test(NAME ctest_http_Download-pst COMMAND test_http_Download-pst --gtest_shuffle)
