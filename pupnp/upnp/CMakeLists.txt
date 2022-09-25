# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-09-25

cmake_minimum_required(VERSION 3.18)
include(../../cmake/project-header.cmake)

project(PUPNP_UPNP VERSION 0025
        DESCRIPTION "Binary build of the UPnP library core"
        HOMEPAGE_URL "https://github.com/upnplib")

# This contains all settings to compile the library dynamically and static.
#==========================================================================

# Switches for this Subproject
#-----------------------------
# These switches are all handled only in this configuration file.

# Options selectable by user configuration
set(UPNPLIB_DEF_GOOGLETEST ${UPNPLIB_WITH_GOOGLETEST})
set(UPNPLIB_DEF_TOOLS ${UPNPLIB_WITH_TOOLS})         # tools -> UPNP_ENABLE_HELPER_API_TOOLS
set(UPNPLIB_DEF_CLIENT ${UPNPLIB_WITH_CLIENT})       # client -> UPNP_ENABLE_CLIENT_API
set(UPNPLIB_DEF_DEVICE ${UPNPLIB_WITH_DEVICE})
set(UPNPLIB_DEF_WEBSERVER ${UPNPLIB_WITH_WEBSERVER}) # webserver -> UPNP_ENABLE_WEBSERVER
                                                     #   only enabled with UPNPLIB_DEF_DEVICE
set(UPNPLIB_DEF_SSDP ${UPNPLIB_WITH_SSDP})           # ssdp -> UPNP_ENABLE_SSDP
set(UPNPLIB_DEF_OPTSSDP ${UPNPLIB_WITH_OPTSSDP})     # optssdp -> UPNP_ENABLE_OPTIONAL_SSDP_HEADERS
set(UPNPLIB_DEF_SOAP ${UPNPLIB_WITH_SOAP})           # soap -> UPNP_ENABLE_SOAP
                                                     #   only enabled with UPNPLIB_DEF_WEBSERVER
set(UPNPLIB_DEF_GENA ${UPNPLIB_WITH_GENA})           # gena -> UPNP_ENABLE_GENA
                                                     #   compiles only with UPNPLIB_DEF_CLIENT and UPNPLIB_DEF_DEVICE
set(UPNPLIB_DEF_IPV6 ${UPNPLIB_WITH_IPV6})           # ipv6 -> UPNP_ENABLE_IPV6
set(UPNPLIB_DEF_OPENSSL NO)                           # open_ssl -> UPNP_ENABLE_OPEN_SSL
set(UPNPLIB_DEF_BLOCKTCP ${UPNPLIB_WITH_BLOCKTCP})   # blocking_tcp_connections -> UPNP_ENABLE_BLOCKING_TCP_CONNECTIONS
set(UPNPLIB_DEF_IXML ${UPNPLIB_WITH_IXML})           # scriptsupport -> IXML_HAVE_SCRIPTSUPPORT
#set(UPNPLIB_DEF_UNSPECSVR NO)                        # unspecified_server -> UPNP_ENABLE_UNSPECIFIED_SERVER
#set(UPNPLIB_DEF_REUSEADDR NO)                        # reuseaddr -> UPNP_MINISERVER_REUSEADDR
if(UPNPLIB_WITH_SAMPLES)                             # all samples -> UPNP_BUILD_SAMPLES
    set(UPNPLIB_DEF_SAMPLES YES) # To make it separate selectable don't set it direct
endif()

# Dependencies of options
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(UPNPLIB_DEF_DEBUG YES)
endif()

if(UPNPLIB_DEF_GOOGLETEST)
    set(UPNPLIB_DEF_WEBSERVER YES)
    set(UPNPLIB_DEF_CLIENT YES)
    set(UPNPLIB_DEF_OPTSSDP YES)
endif()

if(UPNPLIB_DEF_SAMPLES)
    set(UPNPLIB_DEF_TOOLS YES)
    set(UPNPLIB_DEF_CLIENT YES)
    set(UPNPLIB_DEF_WEBSERVER YES)
    set(UPNPLIB_DEF_SOAP YES)
    set(UPNPLIB_DEF_GENA YES)
endif()

if(UPNPLIB_DEF_DEVICE)
    set(UPNPLIB_DEF_WEBSERVER YES)
endif()

if(UPNPLIB_DEF_WEBSERVER)
    set(UPNPLIB_DEF_DEVICE YES)
    set(UPNPLIB_DEF_SOAP YES)
endif()

if(UPNPLIB_DEF_SOAP)
    set(UPNPLIB_DEF_SSDP YES)
endif()

if(UPNPLIB_DEF_GENA)
    set(UPNPLIB_DEF_SSDP YES)
endif()


# Source files and compile settings
#----------------------------------

set(PUPNP_SOURCE_FILES
    ./src/init.cpp
    ./src/global.cpp

    ./src/threadutil/FreeList.cpp
    ./src/threadutil/LinkedList.cpp
    ./src/threadutil/ThreadPool.cpp
    ./src/threadutil/TimerThread.cpp

    ./src/genlib/net/sock.cpp
    ./src/genlib/net/uri/uri.cpp

    ./src/genlib/net/http/httpparser.cpp
    ./src/genlib/net/http/httpreadwrite.cpp
    ./src/genlib/net/http/statcodes.cpp
    ./src/genlib/net/http/webserver.cpp
    ./src/genlib/net/http/parsetools.cpp

    ./src/genlib/util/list.cpp
    ./src/genlib/util/membuffer.cpp
    ./src/genlib/util/strintmap.cpp
    ./src/genlib/util/upnp_timeout.cpp
    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_SOAP}>>:./src/genlib/util/util.cpp>

    $<$<BOOL:${UPNPLIB_DEF_DEVICE}>:./src/genlib/service-table/service_table.cpp>
    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_GENA}>>:./src/genlib/client_table/GenlibClientSubscription.cpp>
    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_GENA}>>:./src/genlib/client_table/client_table.cpp>
    ./src/genlib/miniserver/miniserver.cpp

    $<$<BOOL:${UPNPLIB_DEF_SSDP}>:./src/ssdp/ssdp_server.cpp>
    $<$<BOOL:${UPNPLIB_DEF_SSDP}>:./src/ssdp/ssdp_device.cpp>
    $<$<BOOL:${UPNPLIB_DEF_SSDP}>:./src/ssdp/ssdp_ctrlpt.cpp>
    $<$<BOOL:${UPNPLIB_DEF_SSDP}>:./src/ssdp/SSDPResultData.cpp>
    $<$<BOOL:${UPNPLIB_DEF_SSDP}>:./src/ssdp/SSDPResultDataCallback.cpp>

    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_SOAP}>>:./src/soap/soap_device.cpp>
    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_SOAP}>>:./src/soap/soap_common.cpp>
    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_SOAP}>>:./src/soap/soap_ctrlpt.cpp>

    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_GENA}>>:./src/gena/gena_device.cpp>
    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_GENA}>>:./src/gena/gena_callback2.cpp>
    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_GENA}>>:./src/gena/gena_ctrlpt.cpp>

    $<$<OR:$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_GENA}>>,$<BOOL:${UPNPLIB_DEF_OPTSSDP}>>:./src/uuid/uuid.cpp>
    $<$<OR:$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_GENA}>>,$<BOOL:${UPNPLIB_DEF_OPTSSDP}>>:./src/uuid/md5.cpp>
    $<$<OR:$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_GENA}>>,$<BOOL:${UPNPLIB_DEF_OPTSSDP}>>:./src/uuid/sysdep.cpp>

    ./src/api/UpnpExtraHeaders.cpp
    ./src/api/UpnpString.cpp
    ./src/api/upnpapi.cpp
    $<$<OR:$<BOOL:${UPNPLIB_DEF_TOOLS}>,$<BOOL:${UPNPLIB_DEF_DEBUG}>>:./src/api/upnptools.cpp>
    $<$<BOOL:${UPNPLIB_DEF_WEBSERVER}>:./src/api/UpnpFileInfo.cpp>
    #$<$<BOOL:${UPNPLIB_DEF_SSDP}>:./src/api/UpnpDiscovery.cpp>
    $<$<OR:$<BOOL:${UPNPLIB_DEF_SSDP}>,$<BOOL:${UPNPLIB_DEF_DEBUG}>>:./src/api/UpnpDiscovery.cpp>

    $<$<OR:$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_SOAP}>>,$<BOOL:${UPNPLIB_DEF_DEBUG}>>:./src/api/UpnpActionRequest.cpp>
    $<$<OR:$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_SOAP}>>,$<BOOL:${UPNPLIB_DEF_DEBUG}>>:./src/api/UpnpActionComplete.cpp>
    $<$<OR:$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_SOAP}>>,$<BOOL:${UPNPLIB_DEF_DEBUG}>>:./src/api/UpnpStateVarRequest.cpp>
    $<$<OR:$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_SOAP}>>,$<BOOL:${UPNPLIB_DEF_DEBUG}>>:./src/api/UpnpStateVarComplete.cpp>
    $<$<OR:$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_SOAP}>>,$<BOOL:${UPNPLIB_DEF_DEBUG}>>:./src/api/UpnpSubscriptionRequest.cpp>
    $<$<OR:$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_SOAP}>>,$<BOOL:${UPNPLIB_DEF_DEBUG}>>:./src/api/UpnpEvent.cpp>
    $<$<OR:$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_SOAP}>>,$<BOOL:${UPNPLIB_DEF_DEBUG}>>:./src/api/UpnpEventSubscribe.cpp>
    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_SOAP}>>:./src/api/UpnpStateVarRequest.cpp>
    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_SOAP}>>:./src/api/UpnpActionComplete.cpp>
    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_SOAP}>>:./src/api/UpnpStateVarComplete.cpp>
    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_GENA}>>:./src/api/UpnpSubscriptionRequest.cpp>
    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_GENA}>>:./src/api/UpnpEventSubscribe.cpp>
    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_GENA}>>:./src/api/UpnpEvent.cpp>
    $<$<CONFIG:Debug>:./src/api/upnpdebug.cpp>

    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_WEBSERVER}>>:./src/urlconfig/urlconfig.cpp>

    ${UPnPlib_SOURCE_DIR}/core/src/mocking/sys_select.cpp
    ${UPnPlib_SOURCE_DIR}/core/src/mocking/netdb.cpp
    ${UPnPlib_SOURCE_DIR}/core/src/mocking/pthread.cpp
    ${UPnPlib_SOURCE_DIR}/core/src/mocking/stdio.cpp
    ${UPnPlib_SOURCE_DIR}/core/src/mocking/stdlib.cpp
    ${UPnPlib_SOURCE_DIR}/core/src/mocking/sys_socket.cpp
    ${UPnPlib_SOURCE_DIR}/core/src/mocking/unistd.cpp
    $<$<NOT:$<PLATFORM_ID:Windows>>:${UPnPlib_SOURCE_DIR}/core/src/mocking/ifaddrs.cpp>
    $<$<NOT:$<PLATFORM_ID:Windows>>:${UPnPlib_SOURCE_DIR}/core/src/mocking/net_if.cpp>
    $<$<BOOL:${WIN32}>:${UPnPlib_SOURCE_DIR}/gtests/tools/mockObj_win32.cpp>
)

add_compile_definitions(
    $<$<BOOL:${UPNPLIB_DEF_CLIENT}>:UPNP_HAVE_CLIENT>
    $<$<BOOL:${UPNPLIB_DEF_DEVICE}>:UPNP_HAVE_DEVICE>
    $<$<BOOL:${UPNPLIB_DEF_WEBSERVER}>:UPNP_HAVE_WEBSERVER>
    $<$<BOOL:${UPNPLIB_DEF_SSDP}>:UPNP_HAVE_SSDP>
    $<$<BOOL:${UPNPLIB_DEF_OPTSSDP}>:UPNP_HAVE_OPTSSDP>
    $<$<BOOL:${UPNPLIB_DEF_SOAP}>:UPNP_HAVE_SOAP>
    $<$<BOOL:${UPNPLIB_DEF_GENA}>:UPNP_HAVE_GENA>
    $<$<BOOL:${UPNPLIB_DEF_IPV6}>:UPNP_ENABLE_IPV6>
    $<$<BOOL:${UPNPLIB_DEF_OPENSSL}>:UPNP_ENABLE_OPEN_SSL>
    $<$<BOOL:${UPNPLIB_DEF_BLOCKTCP}>:UPNP_ENABLE_BLOCKING_TCP_CONNECTIONS>
    $<$<BOOL:${UPNPLIB_DEF_DEBUG}>:UPNP_HAVE_DEBUG>
    $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
)


# Create libraries
#=================
if(NOT WIN32)

    # Pupnp Native SHared library: prefix -pnsh
    #------------------------------------------
    add_library(pupnp_native_shared SHARED
            ${PUPNP_SOURCE_FILES}
    )
    set_target_properties(pupnp_native_shared PROPERTIES
            OUTPUT_NAME pupnp_native  # lib*.so will be added
    )
    target_include_directories(pupnp_native_shared
        PRIVATE
            ${PUPNP_IXML_SOURCE_DIR}/inc
            ${UPnPlib_SOURCE_DIR}/include
            ${UPnPlib_SOURCE_DIR}/gtests/include
            ./inc
            ./src/inc
            ./src/threadutil
            # if we are not on WIN32 the linker finds empty "headers.h" and will
            # not complain an error but will not use <headers.h> that isn't available.
            #$<$<NOT:$<BOOL:${WIN32}>>:./inc/not_win32>
            ./inc/not_win32
    )
    target_compile_definitions(pupnp_native_shared
        PRIVATE
            UPNPLIB_SHARED
            UPNPLIB_EXPORTS
            UPNPLIB_WITH_NATIVE_PUPNP
    )
    target_link_libraries(pupnp_native_shared
        PUBLIC
            ixml_shared
        INTERFACE
            # On Linux we cannot link with the static pthreads library because then
            # we have also link with a static libc that's not available by default.
            ${UPNP_PTHREADS_SHARED_LIBRARY}
    )

    # Pupnp Native STatic library: prefix -pnst
    #------------------------------------------
    add_library(pupnp_native_static STATIC
            ${PUPNP_SOURCE_FILES}
    )
    set_target_properties(pupnp_native_static PROPERTIES
            OUTPUT_NAME pupnp_native  # lib*.a will be added
    )
    target_include_directories(pupnp_native_static
        PRIVATE
            ${PUPNP_IXML_SOURCE_DIR}/inc
            ${UPnPlib_SOURCE_DIR}/include
            ${UPnPlib_SOURCE_DIR}/gtests/include
            ./inc
            ./src/inc
            ./src/threadutil
            # if we are not on WIN32 the linker finds empty "headers.h" and will
            # not complain an error but will not use <headers.h> that isn't available.
            #$<$<NOT:$<BOOL:${WIN32}>>:./inc/not_win32>
            ./inc/not_win32
    )
    target_compile_definitions(pupnp_native_static
        PRIVATE
            UPNPLIB_WITH_NATIVE_PUPNP
    )
    target_link_libraries(pupnp_native_static
        # Static always link needed object files, no matter to use PRIVATE or PUBLIC.
        # For propagation only INTERFACE is needed.
        INTERFACE
            ixml_static
            # On Linux we cannot link with the static pthreads library because then
            # we have also link with a static libc that's not available by default.
            ${UPNP_PTHREADS_SHARED_LIBRARY}
    )

else() # WIN32

    # Link with POSIX Threads for Windows (pthreads4w)
    #=================================================
    # Pthreads are not supported by MS Windows. So we have installed the
    # third party library pthreads4w.

    # WIN32 Pupnp Native SHared library: -pnsh
    #-----------------------------------------
    add_library(pupnp_native_shared SHARED
            ${PUPNP_SOURCE_FILES}
    )
    target_include_directories(pupnp_native_shared
        PRIVATE
            ${PUPNP_IXML_SOURCE_DIR}/inc
            ${UPnPlib_SOURCE_DIR}/include
            ${UPnPlib_SOURCE_DIR}/gtests/include
            ./inc
            ./src/inc
            ./src/threadutil
            # if we are not on WIN32 the linker finds empty "headers.h" and will
            # not complain an error but will not use <headers.h> that isn't available.
            #$<$<NOT:$<BOOL:${WIN32}>>:./inc/not_win32>
            # if pthreads4w isn't installed this path is empty. The linker
            # doesn't find "pthread.h" and falls back to look at <pthread.h>.
            ${pthreads4w_SOURCE_DIR}
    )
    target_compile_definitions(pupnp_native_shared
        PRIVATE
            UPNPLIB_SHARED
            UPNPLIB_EXPORTS
            UPNPLIB_WITH_NATIVE_PUPNP
            $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
            $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    # In addition to pthreads we need some more system libraries here
    target_link_libraries(pupnp_native_shared
        PUBLIC
            ixml_shared
            ws2_32   # winsock to support sockets
            iphlpapi # ip helper interface to get network management tools
            # On MS Windows we link with the static pthreads4w library to
            # avoid error prone managing access to its .dll file.
            ${UPNP_PTHREADS_STATIC_LIBRARY}
    )

    # WIN32 Pupnp Native STatic library: prefix -pnst
    #------------------------------------------------
    add_library(pupnp_native_static STATIC   # prefix -pnst
            ${PUPNP_SOURCE_FILES}
    )
    target_include_directories(pupnp_native_static
        PRIVATE
            ${PUPNP_IXML_SOURCE_DIR}/inc
            ${UPnPlib_SOURCE_DIR}/include
            ${UPnPlib_SOURCE_DIR}/gtests/include
            ./inc
            ./src/inc
            ./src/threadutil
            # if we are not on WIN32 the linker finds empty "headers.h" and will
            # not complain an error but will not use <headers.h> that isn't available.
            #$<$<NOT:$<BOOL:${WIN32}>>:./inc/not_win32>
            # if pthreads4w isn't installed this path is empty. The linker
            # doesn't find "pthread.h" and falls back to look at <pthread.h>.
            ${pthreads4w_SOURCE_DIR}
    )
    target_compile_definitions(pupnp_native_static
        PRIVATE
            UPNPLIB_WITH_NATIVE_PUPNP
            $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
            $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    target_link_libraries(pupnp_native_static
        # Static always link needed object files, no matter to use PRIVATE or PUBLIC.
        # For propagation only INTERFACE is needed.
        INTERFACE
            ixml_static
            ws2_32   # winsock to support sockets
            iphlpapi # ip helper interface to get network management tools
            # On MS Windows we link with the static pthreads4w library to
            # avoid error prone managing access to its .dll file.
            ${UPNP_PTHREADS_STATIC_LIBRARY}
    )

endif() # WIN32

# vim: nowrap
