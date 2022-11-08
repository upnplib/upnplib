# Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-11-08

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
    PRIVATE
        ${GTESTS_SOURCE_DIR}/include
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
        ${UPNPLIB_PROJECT_SOURCE_DIR}
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        $<$<NOT:$<PLATFORM_ID:Windows>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${PUPNP_IXML_SOURCE_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_webserver-pst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
        UPNP_HAVE_WEBSERVER
        UPNP_HAVE_DEVICE
        UPNP_HAVE_SOAP
        UPNP_HAVE_SSDP
)
target_link_libraries(test_webserver-pst
        PRIVATE
            pupnp_static
            upnplib_static
)
add_test(NAME ctest_webserver-pst COMMAND test_webserver-pst
)

add_executable(test_webserver-cst
#--------------------------------
    ./test_webserver.cpp
)
target_include_directories(test_webserver-cst
    PRIVATE
        ${GTESTS_SOURCE_DIR}/include
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
        ${UPNPLIB_PROJECT_SOURCE_DIR}
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        $<$<NOT:$<PLATFORM_ID:Windows>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${PUPNP_IXML_SOURCE_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_webserver-cst
    PRIVATE
        UPNP_HAVE_WEBSERVER
        UPNP_HAVE_DEVICE
        UPNP_HAVE_SOAP
        UPNP_HAVE_SSDP
)
target_link_libraries(test_webserver-cst
        PRIVATE
            compa_static
            upnplib_static
)
add_test(NAME ctest_webserver-cst COMMAND test_webserver-cst
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
        ./test_httpreadwrite.cpp
)
target_include_directories(test_httpreadwrite-pst
    PRIVATE
        ${GTESTS_SOURCE_DIR}/include
        ${UPNPLIB_PROJECT_SOURCE_DIR}
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        $<$<NOT:$<PLATFORM_ID:Windows>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${PUPNP_IXML_SOURCE_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_httpreadwrite-pst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
)
target_link_libraries(test_httpreadwrite-pst
    PRIVATE
        pupnp_static
        upnplib_static
)
add_test(NAME ctest_httpreadwrite-pst COMMAND test_httpreadwrite-pst
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
        ./test_netconnect.cpp
)
target_include_directories(test_netconnect-pst
    PRIVATE
        ${GTESTS_SOURCE_DIR}/include
        ${UPNPLIB_PROJECT_SOURCE_DIR}
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        $<$<NOT:$<PLATFORM_ID:Windows>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${PUPNP_IXML_SOURCE_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_netconnect-pst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
)
target_link_libraries(test_netconnect-pst
    PRIVATE
        pupnp_static
)
add_test(NAME ctest_netconnect-pst COMMAND test_netconnect-pst
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
    ./test_httpparser.cpp
)
target_include_directories(test_httpparser-pst
    PRIVATE
        ${GTESTS_SOURCE_DIR}/include
        ${UPNPLIB_PROJECT_SOURCE_DIR}
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        $<$<NOT:$<PLATFORM_ID:Windows>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${PUPNP_IXML_SOURCE_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_httpparser-pst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
)
target_link_libraries(test_httpparser-pst
        PRIVATE
            pupnp_static
)
add_test(NAME ctest_httpparser-pst COMMAND test_httpparser-pst
)
