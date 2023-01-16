# Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2023-01-18

cmake_minimum_required(VERSION 3.18)
include(../../../cmake/project-header.cmake)

project(GTESTS_COMPA_HTTP VERSION 0002
                  DESCRIPTION "Tests for the compa http module"
                  HOMEPAGE_URL "https://github.com/upnplib")


# webserver
#==========
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts. We must use static libraries.

add_executable(test_webserver-pst
#--------------------------------
    ./test_webserver.cpp
)
target_include_directories(test_webserver-pst
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
    PRIVATE ${UPNPLIB_PROJECT_BINARY_DIR}/include
)
target_compile_definitions(test_webserver-pst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
        UPNP_HAVE_WEBSERVER
        #UPNPLIB_WITH_TRACE
)
target_compile_options(test_webserver-pst
    PRIVATE
        # To fix these warnigs we have to modify the original pupnp code but
        # that isn't intended. So the warnings are suppressed.
        # disable warning C4701: potentially uninitialized local variable 'subtype' used
        $<$<CXX_COMPILER_ID:MSVC>:/wd4701>
        # disable warning C4703: potentially uninitialized local pointer variable 'subtype' used
        $<$<CXX_COMPILER_ID:MSVC>:/wd4703>
)
target_link_libraries(test_webserver-pst
        PRIVATE
            pupnp_static
            upnplib_static
)
add_test(NAME ctest_webserver-pst COMMAND test_webserver-pst --gtest_shuffle
)

add_executable(test_webserver-cst
#--------------------------------
    ./test_webserver.cpp
)
target_include_directories(test_webserver-cst
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
    PRIVATE ${UPNPLIB_PROJECT_BINARY_DIR}/include
)
target_compile_definitions(test_webserver-cst
    PRIVATE
        UPNP_HAVE_WEBSERVER
        #UPNPLIB_WITH_TRACE
)
target_compile_options(test_webserver-cst
    PRIVATE
        # To fix these warnigs we have to modify the original pupnp code but
        # that isn't intended. So the warnings are suppressed.
        # disable warning C4701: potentially uninitialized local variable 'subtype' used
        $<$<CXX_COMPILER_ID:MSVC>:/wd4701>
        # disable warning C4703: potentially uninitialized local pointer variable 'subtype' used
        $<$<CXX_COMPILER_ID:MSVC>:/wd4703>
)
target_link_libraries(test_webserver-cst
        PRIVATE
            compa_static
            upnplib_static
)
add_test(NAME ctest_webserver-cst COMMAND test_webserver-cst --gtest_shuffle
)


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
    PRIVATE ${UPNPLIB_SOURCE_DIR}/include
)
target_compile_definitions(test_httpreadwrite-pst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
)
target_compile_options(test_httpreadwrite-pst
    PRIVATE
        # To fix these warnigs we have to modify the original pupnp code but
        # that isn't intended. So the warnings are suppressed.
        # disable warning C4505: 'upnplib::Check_Connect_And_Wait_Connection': unreferenced local function has been removed
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
    PRIVATE ${UPNPLIB_SOURCE_DIR}/include
)
target_compile_definitions(test_netconnect-pst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
)
target_compile_options(test_netconnect-pst
    PRIVATE
        # To fix these warnigs we have to modify the original pupnp code but
        # that isn't intended. So the warnings are suppressed.
        # disable warning C4706: assignment within conditional expression
        $<$<CXX_COMPILER_ID:MSVC>:/wd4706>
)
target_link_libraries(test_netconnect-pst
    PRIVATE
        pupnp_static
)
add_test(NAME ctest_netconnect-pst COMMAND test_netconnect-pst --gtest_shuffle
)


# httpparser
#===========
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts. We must use static libraries.
if(FALSE)
# Direct linking with source files currently not used but having available for
# reference. We link against the internal upnplib libraries (see below).
add_executable(test_httpparser-pst
    ${PUPNP_UPNP_SOURCE_DIR}/src/global.cpp
    ./tools/mockObj.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/FreeList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/LinkedList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/uri/uri.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/list.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/membuffer.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/strintmap.cpp
    $<$<CONFIG:Debug>:${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpdebug.cpp>

    ./test_httpparser.cpp
)
endif()

add_executable(test_httpparser-pst
#---------------------------------
    ./test_httpparser.cpp
)
target_include_directories(test_httpparser-pst
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_compile_definitions(test_httpparser-pst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)
target_compile_options(test_httpparser-pst
    PRIVATE
        # To fix these warnigs we have to modify the original pupnp code but
        # that isn't intended. So the warnings are suppressed.
        # disable warning C4706: assignment within conditional expression
        $<$<CXX_COMPILER_ID:MSVC>:/wd4706>
        # disable warning C4701: potentially uninitialized local variable 'status' used
        $<$<CXX_COMPILER_ID:MSVC>:/wd4701>
)
target_link_libraries(test_httpparser-pst
        PRIVATE
            pupnp_static
)
add_test(NAME ctest_httpparser-pst COMMAND test_httpparser-pst --gtest_shuffle
)

add_executable(test_httpparser-cst
#---------------------------------
    ./test_httpparser.cpp
)
target_include_directories(test_httpparser-cst
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
    PRIVATE ${UPNPLIB_SOURCE_DIR}/include
)
target_compile_definitions(test_httpparser-cst
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)
target_compile_options(test_httpparser-cst
    PRIVATE
        # To fix these warnigs we have to modify the original pupnp code but
        # that isn't intended. So the warnings are suppressed.
        # disable warning C4706: assignment within conditional expression
        $<$<CXX_COMPILER_ID:MSVC>:/wd4706>
        # disable warning C4701: potentially uninitialized local variable 'status' used
        $<$<CXX_COMPILER_ID:MSVC>:/wd4701>
)
target_link_libraries(test_httpparser-cst
        PRIVATE
            compa_static
)
add_test(NAME ctest_httpparser-cst COMMAND test_httpparser-cst --gtest_shuffle
)
