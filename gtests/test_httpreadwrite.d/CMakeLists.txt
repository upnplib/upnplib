# Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-09-02

cmake_minimum_required(VERSION 3.18)
include(../../cmake/project-header.cmake)

project(GTESTS_HTTPREADWRITE VERSION 0001
                  DESCRIPTION "Tests for the upnplib httpreadwrite module"
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
    ${UPNPLIB_GTESTS_SOURCE_DIR}/tools/mockObj.cpp
    $<$<BOOL:${WIN32}>:${UPNPLIB_GTESTS_SOURCE_DIR}/tools/mockObj_win32.cpp>

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

    ${UPNPLIB_CORE_SOURCE_DIR}/src/api/upnptools.cpp
    ./test_httpreadwrite.cpp
)
endif()

add_executable(test_httpreadwrite-pnst
        ./test_httpreadwrite.cpp
)
target_include_directories(test_httpreadwrite-pnst
    PRIVATE
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
        ${UPnPlib_SOURCE_DIR}
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        $<$<NOT:$<PLATFORM_ID:Windows>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${PUPNP_IXML_SOURCE_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_httpreadwrite-pnst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_httpreadwrite-pnst
    PRIVATE
        pupnp_native_static
        upnplib_native_static
)
add_test(NAME ctest_httpreadwrite-pnst COMMAND test_httpreadwrite-pnst
)


# httpreadwrite: netconnect
#==========================
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts. We must use static libraries.
if(FALSE)
# Direct linking with source files currently not used but having available for
# reference. We link against the upnplib libraries (see below).
add_executable(test_netconnect
    ${PUPNP_UPNP_SOURCE_DIR}/src/global.cpp
    ${UPNPLIB_GTESTS_SOURCE_DIR}/tools/mockObj.cpp
    $<$<BOOL:${WIN32}>:${UPNPLIB_GTESTS_SOURCE_DIR}/tools/mockObj_win32.cpp>

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

add_executable(test_netconnect-pnst
        ./test_netconnect.cpp
)
target_include_directories(test_netconnect-pnst
    PRIVATE
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
        ${UPnPlib_SOURCE_DIR}
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        $<$<NOT:$<PLATFORM_ID:Windows>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${PUPNP_IXML_SOURCE_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_netconnect-pnst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_netconnect-pnst
    PRIVATE
        pupnp_native_static
)
add_test(NAME ctest_netconnect-pnst COMMAND test_netconnect-pnst
)
