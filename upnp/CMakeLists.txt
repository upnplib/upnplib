# Copyright 2021 GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Last modified: 2021-07-25

project(upnplib-build DESCRIPTION "Binary build of the UPnP library")

# This contains all settings to compile the library dynamically and static.
#==========================================================================

# Compiler options
#-----------------

if (MSVC)
    # TODO: Fix warnings from the MSVC compiler
    # Reports the compiler warning that is specified by nnnn only once.
    #add_compile_options(/wo4273)
    # suppress all warnings for development to see only errors
    #add_compile_options(/W0)
else()
    # lots of warnings and all warnings as errors
    #add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()


# Source files and compile settings
#----------------------------------

include_directories(
#    "${PROJECT_SOURCE_DIR}/../build/_deps/googletest-src/googletest/include"
#    "${PROJECT_SOURCE_DIR}/../build/_deps/googletest-src/googlemock/include"
    ${PROJECT_SOURCE_DIR}/inc
    ${PROJECT_SOURCE_DIR}/src/inc
    ${PROJECT_SOURCE_DIR}/src/threadutil
    ${CMAKE_SOURCE_DIR}/ixml/inc
    ${CMAKE_SOURCE_DIR}/ixml/src/inc
    ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/upnp/inc
    ${PTHREADS4W_BINARY_DIR}/include
)

add_executable(upnplib
    ${CMAKE_SOURCE_DIR}/ixml/src/attr.c
    ${CMAKE_SOURCE_DIR}/ixml/src/document.c
    ${CMAKE_SOURCE_DIR}/ixml/src/element.c
    ${CMAKE_SOURCE_DIR}/ixml/src/ixml.c
    ${CMAKE_SOURCE_DIR}/ixml/src/ixmlmembuf.c
    ${CMAKE_SOURCE_DIR}/ixml/src/ixmlparser.c
    ${CMAKE_SOURCE_DIR}/ixml/src/namedNodeMap.c
    ${CMAKE_SOURCE_DIR}/ixml/src/node.c
    ${CMAKE_SOURCE_DIR}/ixml/src/nodeList.c

    ${PROJECT_SOURCE_DIR}/src/threadutil/FreeList.cpp
    ${PROJECT_SOURCE_DIR}/src/threadutil/LinkedList.cpp
    ${PROJECT_SOURCE_DIR}/src/threadutil/ThreadPool.cpp
    ${PROJECT_SOURCE_DIR}/src/threadutil/TimerThread.cpp

    ${PROJECT_SOURCE_DIR}/src/genlib/net/sock.c

    ${PROJECT_SOURCE_DIR}/src/genlib/net/uri/uri.c

    ${PROJECT_SOURCE_DIR}/src/genlib/net/http/httpparser.c
    ${PROJECT_SOURCE_DIR}/src/genlib/net/http/httpreadwrite.c
    ${PROJECT_SOURCE_DIR}/src/genlib/net/http/statcodes.c

    ${PROJECT_SOURCE_DIR}/src/genlib/util/list.c
    ${PROJECT_SOURCE_DIR}/src/genlib/util/membuffer.c
    ${PROJECT_SOURCE_DIR}/src/genlib/util/strintmap.c

    ${PROJECT_SOURCE_DIR}/src/api/UpnpExtraHeaders.c
    ${PROJECT_SOURCE_DIR}/src/api/UpnpString.c
    ${PROJECT_SOURCE_DIR}/src/api/upnpapi.cpp
    ${PROJECT_SOURCE_DIR}/src/api/upnpdebug.cpp

    ${PROJECT_SOURCE_DIR}/src/upnplib.cpp
)

target_compile_definitions (upnplib
    #PRIVATE $<$<BOOL:${script_support}>:IXML_HAVE_SCRIPTSUPPORT>
    #PUBLIC $<IF:$<CONFIG:Debug>,DEBUG,NDEBUG>
    PUBLIC $<$<BOOL:${MSVC}>:UPNP_USE_MSVCPP>
    PUBLIC $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>
)
