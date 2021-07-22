# Copyright 2021 GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Last modified: 2021-07-21

CMAKE_MINIMUM_REQUIRED (VERSION 3.18)
CMAKE_POLICY (VERSION 3.18)
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
set (UPNP_VERSION_STRING "1.0.0")
set (UPNP_VERSION_MAJOR 1)
project(UPnPlib VERSION ${UPNP_VERSION_STRING}
                DESCRIPTION "UPnP library"
                HOMEPAGE_URL "https://github.com/upnplib")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Set upnplib options
#------------------------------------------------------------------------
# If you add or modify an option you MUST also do it in the corresponding
# "${PROJECT_SOURCE_DIR}/upnp/cmake/upnpconfig.h.cmake".
# Variablename and its value-placeholder must be the same.
# #cmakedefine VAR1 ${VAR} does not work.
# #cmakedefine VAR2 1  : VAR2 must be exist before even you set a constant.

    # TODO: Test this option to link statically
    # UPNP_STATIC_LIB - for creating a statically linkable UPnP-library
    option(UPNP_STATIC_LIB "Create a statically linkable UPnP-library" OFF)

    # TODO: Test this option to use the static pthreads4W
    # PTW32_STATIC_LIB - for using the static pthreads4W library
    option(PTW32_STATIC_LIB "For using the static pthreads4W library" OFF)

    # Check available library functions. We need that for later configuration.
    include(CheckCXXSymbolExists)
    check_cxx_symbol_exists(strnlen "cstring" HAVE_STRNLEN)
    check_cxx_symbol_exists(strndup "cstring" HAVE_STRNDUP)

    # All needed variables defined so we can write upnpconfig.h now
    configure_file(${PROJECT_SOURCE_DIR}/cmake/upnpconfig.h.cmake
                   ${PROJECT_BINARY_DIR}/upnp/inc/upnpconfig.h)
    # TODO: Remove symlink which is only for compatibility reasons
    file(CREATE_LINK ${PROJECT_BINARY_DIR}/upnp/inc/upnpconfig.h
                     ${PROJECT_BINARY_DIR}/autoconfig.h
                     COPY_ON_ERROR SYMBOLIC)


# Configure and compile settings
#-------------------------------
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


include_directories(
#    "${PROJECT_SOURCE_DIR}/../build/_deps/googletest-src/googletest/include"
#    "${PROJECT_SOURCE_DIR}/../build/_deps/googletest-src/googlemock/include"
    "${PROJECT_SOURCE_DIR}/upnp/inc"
    "${PROJECT_SOURCE_DIR}/upnp/src/inc"
    "${PROJECT_SOURCE_DIR}/upnp/src/threadutil"
    "${PROJECT_SOURCE_DIR}/ixml/inc"
    "${PROJECT_SOURCE_DIR}/ixml/src/inc"
    # TODO: Check correct include path for pthreads4w
    "${PROJECT_SOURCE_DIR}/pthreads4w"
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_BINARY_DIR}/upnp/inc"
)

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

target_compile_definitions (upnplib
    #PRIVATE $<$<BOOL:${script_support}>:IXML_HAVE_SCRIPTSUPPORT>
    #PUBLIC $<IF:$<CONFIG:Debug>,DEBUG,NDEBUG>
    PUBLIC $<$<BOOL:${MSVC}>:UPNP_USE_MSVCPP>
    PUBLIC $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>
)


if (WIN32)
# Only on MS Windows we must link to the open source pthreads library.
# Microsoft does not support POSIX threads that we need.

    add_library(pthreads4w STATIC IMPORTED)
    set_target_properties(pthreads4w PROPERTIES IMPORTED_LOCATION
                          ${PROJECT_SOURCE_DIR}/pthreads4w/pthreadVC3)

    # In addition to pthreads We need some more system libraries
    # ws2_32: winsock to support sockets
    # iphlpapi: ip helper interface to get network management tools
    target_link_libraries(upnplib ws2_32 iphlpapi pthreads4w)
endif()
