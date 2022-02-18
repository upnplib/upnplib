# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-02-19

cmake_minimum_required(VERSION 3.18)
include(../../cmake/project-header.cmake)

project(UPNP_CORE VERSION 0009
        DESCRIPTION "Binary build of the UPnP library core"
        HOMEPAGE_URL "https://github.com/upnplib")

# This contains all settings to compile the library dynamically and static.
#==========================================================================

# Switches for this Subproject
#-----------------------------
# These switches are all handled only in this configuration file.

# Options selectable by user configuration
set(UPNPLIB_HAVE_TOOLS  YES)
set(UPNPLIB_HAVE_CLIENT YES)
set(UPNPLIB_HAVE_DEVICE YES)
set(UPNPLIB_HAVE_WEBSERVER YES)
set(UPNPLIB_HAVE_SSDP YES)
set(UPNPLIB_HAVE_OPTSSDP YES)

# Internal switches for program options
set(UPNPLIB_ENABLE_EXTRA_HTTP_HEADERS YES)

# Dependencies of options
if(UPNPLIB_HAVE_WEBSERVER)
    # No webserver without devices
    #set(UPNPLIB_HAVE_DEVICE YES)
endif()

# Source files and compile settings
#----------------------------------
# TODO: Remove references to '${PUPNP_IXML_SOURCE_DIR}' if possible

include_directories(
    ${UPnPlib_BINARY_DIR}
    ${UPnPlib_BINARY_DIR}/upnp/inc
    ${PROJECT_SOURCE_DIR}/inc
    ${PROJECT_SOURCE_DIR}/src/inc
    ${PROJECT_SOURCE_DIR}/src/threadutil
    ${PUPNP_IXML_SOURCE_DIR}/inc
    ${PUPNP_IXML_SOURCE_DIR}/src/inc
    # if pthreads4w isn't installed this path is empty. The linker
    # doesn't find "pthread.h" and falls back to look at <pthread.h>.
    ${pthreads4w_SOURCE_DIR}/
)

set(UPNP_SOURCE_FILES
    ${PUPNP_IXML_SOURCE_DIR}/src/attr.c
    ${PUPNP_IXML_SOURCE_DIR}/src/document.c
    ${PUPNP_IXML_SOURCE_DIR}/src/element.c
    ${PUPNP_IXML_SOURCE_DIR}/src/ixml.c
    ${PUPNP_IXML_SOURCE_DIR}/src/ixmlmembuf.c
    ${PUPNP_IXML_SOURCE_DIR}/src/ixmlparser.c
    ${PUPNP_IXML_SOURCE_DIR}/src/namedNodeMap.c
    ${PUPNP_IXML_SOURCE_DIR}/src/node.c
    ${PUPNP_IXML_SOURCE_DIR}/src/nodeList.c
    $<$<CONFIG:Debug>:${PUPNP_IXML_SOURCE_DIR}/src/ixmldebug.c>

    ${PROJECT_SOURCE_DIR}/src/threadutil/FreeList.cpp
    ${PROJECT_SOURCE_DIR}/src/threadutil/LinkedList.cpp
    ${PROJECT_SOURCE_DIR}/src/threadutil/ThreadPool.cpp
    ${PROJECT_SOURCE_DIR}/src/threadutil/TimerThread.cpp

    ${PROJECT_SOURCE_DIR}/src/genlib/net/sock.cpp
    ${PROJECT_SOURCE_DIR}/src/genlib/net/uri/uri.cpp

    ${PROJECT_SOURCE_DIR}/src/genlib/net/http/httpparser.cpp
    ${PROJECT_SOURCE_DIR}/src/genlib/net/http/httpreadwrite.cpp
    ${PROJECT_SOURCE_DIR}/src/genlib/net/http/statcodes.cpp
    ${PROJECT_SOURCE_DIR}/src/genlib/net/http/webserver.cpp

    ${PROJECT_SOURCE_DIR}/src/genlib/util/list.cpp
    ${PROJECT_SOURCE_DIR}/src/genlib/util/membuffer.cpp
    ${PROJECT_SOURCE_DIR}/src/genlib/util/strintmap.cpp
    $<$<BOOL:${UPNPLIB_HAVE_SSDP}>:${PROJECT_SOURCE_DIR}/src/genlib/util/upnp_timeout.cpp>

    $<$<BOOL:${UPNPLIB_HAVE_DEVICE}>:${PROJECT_SOURCE_DIR}/src/genlib/service-table/service_table.cpp>
    ${PROJECT_SOURCE_DIR}/src/genlib/miniserver/miniserver.cpp

    $<$<BOOL:${UPNPLIB_HAVE_SSDP}>:${PROJECT_SOURCE_DIR}/src/ssdp/ssdp_server.cpp>
    $<$<BOOL:${UPNPLIB_HAVE_SSDP}>:${PROJECT_SOURCE_DIR}/src/ssdp/ssdp_device.cpp>
    $<$<BOOL:${UPNPLIB_HAVE_SSDP}>:${PROJECT_SOURCE_DIR}/src/ssdp/ssdp_ctrlpt.cpp>
    $<$<BOOL:${UPNPLIB_HAVE_SSDP}>:${PROJECT_SOURCE_DIR}/src/ssdp/SSDPResultData.cpp>
    $<$<BOOL:${UPNPLIB_HAVE_SSDP}>:${PROJECT_SOURCE_DIR}/src/ssdp/SSDPResultDataCallback.cpp>

    $<$<BOOL:${UPNPLIB_HAVE_OPTSSDP}>:${PROJECT_SOURCE_DIR}/src/uuid/uuid.cpp>
    $<$<BOOL:${UPNPLIB_HAVE_OPTSSDP}>:${PROJECT_SOURCE_DIR}/src/uuid/md5.cpp>
    $<$<BOOL:${UPNPLIB_HAVE_OPTSSDP}>:${PROJECT_SOURCE_DIR}/src/uuid/sysdep.cpp>

    $<$<BOOL:${UPNPLIB_ENABLE_EXTRA_HTTP_HEADERS}>:${PROJECT_SOURCE_DIR}/src/api/UpnpExtraHeaders.c>
    ${PROJECT_SOURCE_DIR}/src/api/UpnpString.cpp
    ${PROJECT_SOURCE_DIR}/src/api/upnpapi.cpp
    $<$<BOOL:${UPNPLIB_HAVE_TOOLS}>:${PROJECT_SOURCE_DIR}/src/api/upnptools.cpp>
    $<$<BOOL:${UPNPLIB_HAVE_WEBSERVER}>:${PROJECT_SOURCE_DIR}/src/api/UpnpFileInfo.cpp>
    $<$<BOOL:${UPNPLIB_HAVE_SSDP}>:${PROJECT_SOURCE_DIR}/src/api/UpnpDiscovery.cpp>
    $<$<CONFIG:Debug>:${PROJECT_SOURCE_DIR}/src/api/upnpdebug.cpp>

    $<$<AND:$<BOOL:${UPNPLIB_HAVE_DEVICE}>,$<BOOL:${UPNPLIB_HAVE_WEBSERVER}>>:${PROJECT_SOURCE_DIR}/src/urlconfig/urlconfig.cpp>

    ${PROJECT_SOURCE_DIR}/src/global.cpp
    ${PROJECT_SOURCE_DIR}/src/mockObj.cpp
    $<$<BOOL:${WIN32}>:${UPNP_CORE_SOURCE_DIR}/src/mockObj_win32.cpp>
    $<$<NOT:$<BOOL:${WIN32}>>:${UPNP_CORE_SOURCE_DIR}/src/mockObj_unix.cpp>
)

add_compile_definitions(
#    $<$<BOOL:${UPNPLIB_HAVE_DEBUG}>:UPNP_HAVE_DEBUG>
    $<$<BOOL:${UPNPLIB_HAVE_TOOLS}>:UPNP_HAVE_TOOLS>
    $<$<BOOL:${UPNPLIB_HAVE_CLIENT}>:UPNP_HAVE_CLIENT>
    $<$<BOOL:${UPNPLIB_HAVE_DEVICE}>:UPNP_HAVE_DEVICE>
    $<$<BOOL:${UPNPLIB_HAVE_WEBSERVER}>:UPNP_HAVE_WEBSERVER>
    $<$<BOOL:${UPNPLIB_HAVE_SSDP}>:UPNP_HAVE_SSDP>
    $<$<BOOL:${UPNPLIB_HAVE_OPTSSDP}>:UPNP_HAVE_OPTSSDP>
#    $<$<BOOL:${UPNPLIB_HAVE_SOAP}>:UPNP_HAVE_SOAP>
#    $<$<BOOL:${UPNPLIB_HAVE_GENA}>:UPNP_HAVE_GENA>
#    $<$<BOOL:${UPNPLIB_ENABLE_IPV6}>:UPNP_ENABLE_IPV6>
#    $<$<BOOL:${UPNPLIB_ENABLE_OPEN_SSL}>:UPNP_ENABLE_OPEN_SSL>
#    $<$<BOOL:${UPNPLIB_ENABLE_BLOCKING_TCP_CONNECTIONS}>:UPNP_ENABLE_BLOCKING_TCP_CONNECTIONS>
#    $<$<BOOL:${UPNPLIB_HAVE_DEBUG}>:UPNP_HAVE_DEBUG>
#    $<$<BOOL:${IXML_HAVE_SCRIPTSUPPORT}>:IXML_HAVE_SCRIPTSUPPORT>
    $<$<BOOL:${UPNPLIB_ENABLE_EXTRA_HTTP_HEADERS}>:UPNPLIB_ENABLE_EXTRA_HTTP_HEADERS>
)

add_compile_options(
        $ENV{MY_CXXFLAG}
        # suppress warning 'type cast': pointer truncation
        $<$<CXX_COMPILER_ID:MSVC>:/wd4311>
)

# Create libraries and create test executable for information
#============================================================
if(NOT WIN32)

    # Shared library
    #---------------
    add_library(upnplib_shared SHARED ${UPNP_SOURCE_FILES})
    set_target_properties(upnplib_shared PROPERTIES
            OUTPUT_NAME upnplib  # lib*.so will be added
            LIBRARY_OUTPUT_DIRECTORY lib  # used if not WIN32
    #       RUNTIME_OUTPUT_DIRECTORY lib  # used if WIN32
    )

    # Test executable linked with the shared library
    add_executable(upnplib_shared_run ${PROJECT_SOURCE_DIR}/src/upnplib.cpp)
    set_target_properties(upnplib_shared_run PROPERTIES
            OUTPUT_NAME upnplib
    )
    target_link_libraries(upnplib_shared_run
            upnplib_shared
            ${UPNP_PTHREADS_SHARED_LIBRARY}
    )

    # Static library
    #---------------
    add_library(upnplib_static STATIC ${UPNP_SOURCE_FILES})
    set_target_properties(upnplib_static PROPERTIES
            OUTPUT_NAME upnplib  # lib*.a will be added
            ARCHIVE_OUTPUT_DIRECTORY lib
    )

else() # WIN32

    # Link with POSIX Threads for Windows (pthreads4w)
    #=================================================
    # Pthreads are not supported by MS Windows. So we have installed the
    # third party library pthreads4w.

    # General options
    #----------------
    add_compile_definitions(
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
        # There are traditional min/max macros on MS Windows which conflict
        # with the C++ std::min|max functions. Following switch disable the
        # macros.
        $<$<CXX_COMPILER_ID:MSVC>:NOMINMAX>
    )

    # Shared library
    #---------------
    add_library(upnplib_shared_win SHARED ${UPNP_SOURCE_FILES})
    set_target_properties(upnplib_shared_win PROPERTIES
            OUTPUT_NAME upnplib
    )
    target_compile_definitions(upnplib_shared_win
            #PRIVATE $<$<BOOL:${script_support}>:IXML_HAVE_SCRIPTSUPPORT>
            PUBLIC $<$<CXX_COMPILER_ID:MSVC>:LIBUPNP_EXPORTS>
    )
    # In addition to pthreads we need some more system libraries here
    # ws2_32: winsock to support sockets
    # iphlpapi: ip helper interface to get network management tools
    target_link_libraries(upnplib_shared_win ws2_32 iphlpapi
            ${UPNP_PTHREADS_STATIC_LIBRARY}
            # If you link with the shared pthreads4w library then its .dll file
            # must also be found.
            #${UPNP_PTHREADS_SHARED_LIBRARY}
    )

    # Test executable linked with the shared library
    add_executable(upnplib_shared_win_exe ${PROJECT_SOURCE_DIR}/src/upnplib.cpp)
    set_target_properties(upnplib_shared_win_exe PROPERTIES
            OUTPUT_NAME upnplib
    )
    target_link_libraries(upnplib_shared_win_exe
            upnplib_shared_win
    )

    # Static library
    #---------------
    add_library(upnplib_static_win STATIC ${UPNP_SOURCE_FILES})
    set_target_properties(upnplib_static_win PROPERTIES
            OUTPUT_NAME upnplib_Static
    )
    target_compile_definitions(upnplib_static_win
            #PRIVATE $<$<BOOL:${script_support}>:IXML_HAVE_SCRIPTSUPPORT>
            PUBLIC $<$<CXX_COMPILER_ID:MSVC>:LIBUPNP_EXPORTS>
    )
    target_link_libraries(upnplib_static_win
            ${UPNP_PTHREADS_STATIC_LIBRARY}
    )

    # Test executable linked with the static library
    add_executable(upnplib_static_win_exe ${PROJECT_SOURCE_DIR}/src/upnplib.cpp)
    set_target_properties(upnplib_static_win_exe PROPERTIES
            OUTPUT_NAME upnplibStatic
    )
    target_link_libraries(upnplib_static_win_exe
            upnplib_static_win
            ws2_32
    )

endif() # WIN32
