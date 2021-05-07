# Copyright 2021 GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Last modified: 2021-05-07

cmake_minimum_required(VERSION 3.18)

# set the project name and version
project(Gtests VERSION 002
               DESCRIPTION "Unit Tests using googletest"
               HOMEPAGE_URL "https://github.com/upnplib")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# First check available library functions. We need that for later configuration.
include (CheckFunctionExists)
check_function_exists (strnlen HAVE_STRNLEN)
check_function_exists (strndup HAVE_STRNDUP)

# Initialize options from config files.
# upnpconfig.h and autoconfig.h are synchronized
# autoheader.cmake will create autoconfig.h.cm
include(${PROJECT_SOURCE_DIR}/cmake/autoheader.cmake)
configure_file (${PROJECT_BINARY_DIR}/autoconfig.h.cm
                ${PROJECT_BINARY_DIR}/autoconfig.h)
configure_file (${PROJECT_SOURCE_DIR}/upnp/inc/upnpconfig.h.cm
                ${PROJECT_BINARY_DIR}/upnp/inc/upnpconfig.h)


include_directories(
#    "${PROJECT_SOURCE_DIR}/../build/_deps/googletest-src/googletest/include"
#    "${PROJECT_SOURCE_DIR}/../build/_deps/googletest-src/googlemock/include"
    "${PROJECT_SOURCE_DIR}/../upnp/inc"
    "${PROJECT_SOURCE_DIR}/../upnp/src/inc"
    "${PROJECT_SOURCE_DIR}/../upnp/src/threadutil"
    "${PROJECT_SOURCE_DIR}/../ixml/inc"
    "${PROJECT_SOURCE_DIR}/../ixml/src/inc"
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_BINARY_DIR}/upnp/inc"
)

link_libraries("pthread")

add_executable(upnplib_base.a
    ${PROJECT_SOURCE_DIR}/../ixml/src/ixml.c
    ${PROJECT_SOURCE_DIR}/../ixml/src/ixmlmembuf.c
    ${PROJECT_SOURCE_DIR}/../ixml/src/node.c
    ${PROJECT_SOURCE_DIR}/../ixml/src/nodeList.c
    ${PROJECT_SOURCE_DIR}/../ixml/src/attr.c
    ${PROJECT_SOURCE_DIR}/../ixml/src/element.c
    ${PROJECT_SOURCE_DIR}/../ixml/src/document.c
    ${PROJECT_SOURCE_DIR}/../ixml/src/ixmlparser.c
    ${PROJECT_SOURCE_DIR}/../ixml/src/namedNodeMap.c

    ${PROJECT_SOURCE_DIR}/../upnp/src/threadutil/ThreadPool.cpp
    ${PROJECT_SOURCE_DIR}/../upnp/src/threadutil/LinkedList.cpp
    ${PROJECT_SOURCE_DIR}/../upnp/src/threadutil/FreeList.cpp
    ${PROJECT_SOURCE_DIR}/../upnp/src/threadutil/TimerThread.cpp

    ${PROJECT_SOURCE_DIR}/../upnp/src/genlib/net/sock.c

    ${PROJECT_SOURCE_DIR}/../upnp/src/genlib/net/uri/uri.c

    ${PROJECT_SOURCE_DIR}/../upnp/src/genlib/net/http/httpparser.c
    ${PROJECT_SOURCE_DIR}/../upnp/src/genlib/net/http/statcodes.c
    ${PROJECT_SOURCE_DIR}/../upnp/src/genlib/net/http/httpreadwrite.c

    ${PROJECT_SOURCE_DIR}/../upnp/src/genlib/util/strintmap.c
    ${PROJECT_SOURCE_DIR}/../upnp/src/genlib/util/membuffer.c
    ${PROJECT_SOURCE_DIR}/../upnp/src/genlib/util/list.c

    ${PROJECT_SOURCE_DIR}/../upnp/src/api/UpnpExtraHeaders.c
    ${PROJECT_SOURCE_DIR}/../upnp/src/api/UpnpString.c
    ${PROJECT_SOURCE_DIR}/../upnp/src/api/upnpdebug.cpp
    ${PROJECT_SOURCE_DIR}/../upnp/src/api/upnpapi.cpp

    ${PROJECT_SOURCE_DIR}/../upnp/test/test_init.c
)
