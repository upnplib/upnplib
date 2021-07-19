# Copyright 2021 GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Last modified: 2021-07-19

CMAKE_MINIMUM_REQUIRED (VERSION 3.18)
#CMAKE_POLICY (VERSION 3.13)
if("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    message ("In-source builds are not supported.")
    message ("Use a separate folder for building, for example:\n")
    message ("    cmake -S . -B build/")
    message ("    cmake --build build/\n")
    message ("Before that, remove the files already created:\n")
    message ("    rm -rf CMakeCache.txt CMakeFiles\n")
    message (FATAL_ERROR "")
endif()

# set the project name and version
set (UPNP_VERSION_STRING "0001")
set (UPNP_VERSION_MAJOR 1)
project(UPnPlib VERSION ${UPNP_VERSION_STRING}
                DESCRIPTION "UPnP library"
                HOMEPAGE_URL "https://github.com/upnplib")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Set library options
#------------------------------------------------------------------------
# If you add or modify an option you MUST also do it in the corresponding
# "${PROJECT_SOURCE_DIR}/upnp/cmake/upnpconfig.h.cmake".

    # Check available library functions. We need that for later configuration.
    include (CheckFunctionExists)
    check_function_exists (strnlen HAVE_STRNLEN)
    check_function_exists (strndup HAVE_STRNDUP)

    configure_file(${PROJECT_SOURCE_DIR}/cmake/upnpconfig.h.cmake
                   ${PROJECT_BINARY_DIR}/upnp/inc/upnpconfig.h)
    # TODO: Remove symlink which is only for compatibility reasons
    file(CREATE_LINK ${PROJECT_BINARY_DIR}/upnp/inc/upnpconfig.h
                     ${PROJECT_BINARY_DIR}/autoconfig.h
                     SYMBOLIC)


# Configure and compile settings
#-------------------------------
include_directories(
#    "${PROJECT_SOURCE_DIR}/../build/_deps/googletest-src/googletest/include"
#    "${PROJECT_SOURCE_DIR}/../build/_deps/googletest-src/googlemock/include"
    "${PROJECT_SOURCE_DIR}/upnp/inc"
    "${PROJECT_SOURCE_DIR}/upnp/src/inc"
    "${PROJECT_SOURCE_DIR}/upnp/src/threadutil"
    "${PROJECT_SOURCE_DIR}/ixml/inc"
    "${PROJECT_SOURCE_DIR}/ixml/src/inc"
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_BINARY_DIR}/upnp/inc"
)

link_libraries("pthread")

add_executable(upnplib
    ${PROJECT_SOURCE_DIR}/ixml/src/attr.c
    ${PROJECT_SOURCE_DIR}/ixml/src/document.c
    ${PROJECT_SOURCE_DIR}/ixml/src/element.c
    ${PROJECT_SOURCE_DIR}/ixml/src/ixml.c
    ${PROJECT_SOURCE_DIR}/ixml/src/ixmlmembuf.c
    ${PROJECT_SOURCE_DIR}/ixml/src/ixmlparser.c
    ${PROJECT_SOURCE_DIR}/ixml/src/namedNodeMap.c
    ${PROJECT_SOURCE_DIR}/ixml/src/node.c
    ${PROJECT_SOURCE_DIR}/ixml/src/nodeList.c

    ${PROJECT_SOURCE_DIR}/upnp/src/threadutil/FreeList.cpp
    ${PROJECT_SOURCE_DIR}/upnp/src/threadutil/LinkedList.cpp
    ${PROJECT_SOURCE_DIR}/upnp/src/threadutil/ThreadPool.cpp
    ${PROJECT_SOURCE_DIR}/upnp/src/threadutil/TimerThread.cpp

    ${PROJECT_SOURCE_DIR}/upnp/src/genlib/net/sock.c

    ${PROJECT_SOURCE_DIR}/upnp/src/genlib/net/uri/uri.c

    ${PROJECT_SOURCE_DIR}/upnp/src/genlib/net/http/httpparser.c
    ${PROJECT_SOURCE_DIR}/upnp/src/genlib/net/http/httpreadwrite.c
    ${PROJECT_SOURCE_DIR}/upnp/src/genlib/net/http/statcodes.c

    ${PROJECT_SOURCE_DIR}/upnp/src/genlib/util/list.c
    ${PROJECT_SOURCE_DIR}/upnp/src/genlib/util/membuffer.c
    ${PROJECT_SOURCE_DIR}/upnp/src/genlib/util/strintmap.c

    ${PROJECT_SOURCE_DIR}/upnp/src/api/UpnpExtraHeaders.c
    ${PROJECT_SOURCE_DIR}/upnp/src/api/UpnpString.c
    ${PROJECT_SOURCE_DIR}/upnp/src/api/upnpapi.cpp
    ${PROJECT_SOURCE_DIR}/upnp/src/api/upnpdebug.cpp

    ${PROJECT_SOURCE_DIR}/upnp/src/upnplib.cpp
)
