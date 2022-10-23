# Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-10-21

cmake_minimum_required(VERSION 3.18)
include(../../cmake/project-header.cmake)

project(GTESTS_UPNPAPI VERSION 0001
                  DESCRIPTION "Tests for the upnpapi module"
                  HOMEPAGE_URL "https://github.com/upnplib")

# upnpapi
#========
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts. We must use static libraries.
if(FALSE)
# Direct linking with source files currently not used but having available for
# reference. We link against the internal upnplib libraries (see below).
set(UPNP_UPNPAPI_SOURCE_FILES
    ${PUPNP_IXML_SOURCE_DIR}/src/attr.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/document.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/element.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/ixml.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/ixmlmembuf.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/ixmlparser.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/namedNodeMap.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/node.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/nodeList.cpp
    $<$<CONFIG:Debug>:${PUPNP_IXML_SOURCE_DIR}/src/ixmldebug.cpp>

    ${PUPNP_UPNP_SOURCE_DIR}/src/global.cpp
    ./tools/mockObj.cpp
    $<$<BOOL:${WIN32}>:./tools/mockObj_win32.cpp>

    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/FreeList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/LinkedList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/ThreadPool.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/TimerThread.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/sock.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/uri/uri.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/httpparser.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/httpreadwrite.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/statcodes.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/list.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/membuffer.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/strintmap.cpp

    # ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpExtraHeaders.c
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpString.cpp
    $<$<CONFIG:Debug>:${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpdebug.cpp>

    ${UPNPLIB_CORE_SOURCE_DIR}/src/api/upnptools.cpp
)
endif()

add_executable(test_upnpapi-pnst
#-------------------------------
        ./test_upnpapi.cpp
)
target_include_directories(test_upnpapi-pnst
    PRIVATE
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
        ${UPnPlib_SOURCE_DIR}
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_IXML_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        $<$<NOT:$<PLATFORM_ID:Windows>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_upnpapi-pnst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
        UPNP_HAVE_WEBSERVER
        UPNP_HAVE_CLIENT
        UPNP_HAVE_OPTSSDP
)
target_link_libraries(test_upnpapi-pnst
    PRIVATE
        pupnp_native_static
        umock_static
)
add_test(NAME ctest_upnpapi-pnst COMMAND test_upnpapi-pnst
)

add_executable(test_upnpapi_platform-pnsh
#----------------------------------------
        $<$<PLATFORM_ID:Windows>:./test_upnpapi_win32.cpp>
        $<$<NOT:$<PLATFORM_ID:Windows>>:./test_upnpapi_unix.cpp>
)
target_include_directories(test_upnpapi_platform-pnsh
    PRIVATE
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
        ${UPnPlib_SOURCE_DIR}
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${PUPNP_IXML_SOURCE_DIR}/inc
        $<$<NOT:$<PLATFORM_ID:Windows>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_upnpapi_platform-pnsh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_upnpapi_platform-pnsh
    PRIVATE
        upnplib_gtest_tools_shared
        pupnp_native_shared
        upnplib_native_shared
        umock_shared
)
add_test(NAME ctest_upnpapi_platform-pnsh COMMAND test_upnpapi_platform-pnsh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# UpnpFileInfo
#=============
add_executable(test_UpnpFileInfo-pnsh
#----------------------------------------
    ./test_UpnpFileInfo.cpp
)
target_include_directories(test_UpnpFileInfo-pnsh
    PRIVATE
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_IXML_SOURCE_DIR}/inc
)
target_compile_definitions(test_UpnpFileInfo-pnsh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
)
target_link_libraries(test_UpnpFileInfo-pnsh
    PRIVATE
        pupnp_native_shared
        upnplib_native_shared
)
add_test(NAME ctest_UpnpFileInfo-pnsh COMMAND test_UpnpFileInfo-pnsh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)
