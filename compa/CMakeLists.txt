# Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2023-07-06

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(COMPA VERSION 0004
              DESCRIPTION "UPnPlib compatible library subproject"
              HOMEPAGE_URL "https://github.com/upnplib")


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
set(UPNPLIB_DEF_OPENSSL ${UPNPLIB_WITH_OPENSSL})     # open_ssl -> UPNP_ENABLE_OPEN_SSL
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
    set(UPNPLIB_DEF_CLIENT YES)
    set(UPNPLIB_DEF_DEVICE YES)
    set(UPNPLIB_DEF_SSDP YES)
endif()


# Source files and compile settings
#----------------------------------

set(PUPNP_SOURCE_FILES
    ${PUPNP_UPNP_SOURCE_DIR}/src/init.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/FreeList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/LinkedList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/ThreadPool.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/TimerThread.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/sock.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/uri/uri.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/httpparser.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/httpreadwrite.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/statcodes.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/webserver.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/parsetools.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/list.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/membuffer.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/strintmap.cpp
    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_SOAP}>>:${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/util.cpp>

    $<$<BOOL:${UPNPLIB_DEF_DEVICE}>:${PUPNP_UPNP_SOURCE_DIR}/src/genlib/service-table/service_table.cpp>
    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_GENA}>>:${PUPNP_UPNP_SOURCE_DIR}/src/genlib/client_table/GenlibClientSubscription.cpp>
    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_GENA}>>:${PUPNP_UPNP_SOURCE_DIR}/src/genlib/client_table/client_table.cpp>
    #${PUPNP_UPNP_SOURCE_DIR}/src/genlib/miniserver/miniserver.cpp

    $<$<BOOL:${UPNPLIB_DEF_SSDP}>:${PUPNP_UPNP_SOURCE_DIR}/src/ssdp/ssdp_server.cpp>
    $<$<BOOL:${UPNPLIB_DEF_SSDP}>:${PUPNP_UPNP_SOURCE_DIR}/src/ssdp/ssdp_device.cpp>
    $<$<BOOL:${UPNPLIB_DEF_SSDP}>:${PUPNP_UPNP_SOURCE_DIR}/src/ssdp/ssdp_ctrlpt.cpp>
    $<$<BOOL:${UPNPLIB_DEF_SSDP}>:${PUPNP_UPNP_SOURCE_DIR}/src/ssdp/SSDPResultData.cpp>
    $<$<BOOL:${UPNPLIB_DEF_SSDP}>:${PUPNP_UPNP_SOURCE_DIR}/src/ssdp/SSDPResultDataCallback.cpp>

    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_SOAP}>>:${PUPNP_UPNP_SOURCE_DIR}/src/soap/soap_device.cpp>
    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_SOAP}>>:${PUPNP_UPNP_SOURCE_DIR}/src/soap/soap_common.cpp>
    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_SOAP}>>:${PUPNP_UPNP_SOURCE_DIR}/src/soap/soap_ctrlpt.cpp>

    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_GENA}>>:${PUPNP_UPNP_SOURCE_DIR}/src/gena/gena_device.cpp>
    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_GENA}>>:${PUPNP_UPNP_SOURCE_DIR}/src/gena/gena_callback2.cpp>
    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_GENA}>>:${PUPNP_UPNP_SOURCE_DIR}/src/gena/gena_ctrlpt.cpp>

    $<$<OR:$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_GENA}>>,$<BOOL:${UPNPLIB_DEF_OPTSSDP}>>:${PUPNP_UPNP_SOURCE_DIR}/src/uuid/uuid.cpp>
    $<$<OR:$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_GENA}>>,$<BOOL:${UPNPLIB_DEF_OPTSSDP}>>:${PUPNP_UPNP_SOURCE_DIR}/src/uuid/md5.cpp>
    $<$<OR:$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_GENA}>>,$<BOOL:${UPNPLIB_DEF_OPTSSDP}>>:${PUPNP_UPNP_SOURCE_DIR}/src/uuid/sysdep.cpp>

    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpExtraHeaders.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpString.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpapi.cpp
    $<$<OR:$<BOOL:${UPNPLIB_DEF_TOOLS}>,$<BOOL:${UPNPLIB_DEF_DEBUG}>>:${PUPNP_UPNP_SOURCE_DIR}/src/api/upnptools.cpp>
    $<$<BOOL:${UPNPLIB_DEF_WEBSERVER}>:${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpFileInfo.cpp>
    #$<$<BOOL:${UPNPLIB_DEF_SSDP}>:${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpDiscovery.cpp>
    $<$<OR:$<BOOL:${UPNPLIB_DEF_SSDP}>,$<BOOL:${UPNPLIB_DEF_DEBUG}>>:${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpDiscovery.cpp>

    $<$<OR:$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_SOAP}>>,$<BOOL:${UPNPLIB_DEF_DEBUG}>>:${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpActionRequest.cpp>
    $<$<OR:$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_SOAP}>>,$<BOOL:${UPNPLIB_DEF_DEBUG}>>:${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpActionComplete.cpp>
    $<$<OR:$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_SOAP}>>,$<BOOL:${UPNPLIB_DEF_DEBUG}>>:${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpStateVarRequest.cpp>
    $<$<OR:$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_SOAP}>>,$<BOOL:${UPNPLIB_DEF_DEBUG}>>:${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpStateVarComplete.cpp>
    $<$<OR:$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_SOAP}>>,$<BOOL:${UPNPLIB_DEF_DEBUG}>>:${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpSubscriptionRequest.cpp>
    $<$<OR:$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_SOAP}>>,$<BOOL:${UPNPLIB_DEF_DEBUG}>>:${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpEvent.cpp>
    $<$<OR:$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_SOAP}>>,$<BOOL:${UPNPLIB_DEF_DEBUG}>>:${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpEventSubscribe.cpp>
    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_SOAP}>>:${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpStateVarRequest.cpp>
    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_SOAP}>>:${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpActionComplete.cpp>
    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_SOAP}>>:${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpStateVarComplete.cpp>
    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_GENA}>>:${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpSubscriptionRequest.cpp>
    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_GENA}>>:${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpEventSubscribe.cpp>
    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_GENA}>>:${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpEvent.cpp>
    $<$<CONFIG:Debug>:${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpdebug.cpp>

    $<$<AND:$<BOOL:${UPNPLIB_DEF_DEVICE}>,$<BOOL:${UPNPLIB_DEF_WEBSERVER}>>:${PUPNP_UPNP_SOURCE_DIR}/src/urlconfig/urlconfig.cpp>

    ${UMOCK_SOURCE_DIR}/src/pupnp_sock.cpp
    ${PUPNP_SOURCE_DIR}/src/interface/pupnp-sock.cpp
)

set(COMPA_SOURCE_FILES
    ./src/init.cpp
    ./src/api/UpnpString.cpp
    ./src/api/UpnpFileInfo.cpp
    ./src/api/upnpdebug.cpp
    ./src/genlib/miniserver/miniserver.cpp
    ./src/genlib/net/sock.cpp
    #./src/ssdp/ssdp_server.cpp   # work in progress
    ${UPNPLIB_SOURCE_DIR}/src/port.cpp
)

set(COMPATIBLE_SOURCE_FILES
    ${PUPNP_SOURCE_FILES}
    ${COMPA_SOURCE_FILES}
)

set(COMPATIBLE_INCLUDE_DIRECTORIES
    PUBLIC ./include
    # if pthreads4w isn't installed this path is empty. The linker
    # doesn't find "pthread.h" and falls back to look at <pthread.h>.
    PUBLIC ${pthreads4w_SOURCE_DIR}
    PUBLIC ${PUPNP_SOURCE_DIR}/include
    PUBLIC ${PUPNP_IXML_SOURCE_DIR}/inc
    PUBLIC ${IXML_SOURCE_DIR}/include
    PRIVATE ${UPNPLIB_SOURCE_DIR}/include
)

add_compile_definitions(
    $<$<BOOL:${UPNPLIB_WITH_TRACE}>:UPNPLIB_WITH_TRACE>
    $<$<BOOL:${UPNPLIB_DEF_CLIENT}>:UPNP_HAVE_CLIENT>
    $<$<BOOL:${UPNPLIB_DEF_DEVICE}>:UPNP_HAVE_DEVICE>
    $<$<BOOL:${UPNPLIB_DEF_WEBSERVER}>:UPNP_HAVE_WEBSERVER>
    $<$<BOOL:${UPNPLIB_DEF_SSDP}>:UPNP_HAVE_SSDP>
    $<$<BOOL:${UPNPLIB_DEF_OPTSSDP}>:UPNP_HAVE_OPTSSDP>
    $<$<BOOL:${UPNPLIB_DEF_SOAP}>:UPNP_HAVE_SOAP>
    $<$<BOOL:${UPNPLIB_DEF_GENA}>:UPNP_HAVE_GENA>
    $<$<BOOL:${UPNPLIB_DEF_IPV6}>:UPNP_ENABLE_IPV6>
    $<$<BOOL:${UPNPLIB_DEF_BLOCKTCP}>:UPNP_ENABLE_BLOCKING_TCP_CONNECTIONS>
    $<$<BOOL:${UPNPLIB_DEF_DEBUG}>:UPNP_HAVE_DEBUG>
    $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
)

# -Wno-deprecated-declarations: disable warning 'sprintf' (and others) is
# deprecated: This function is provided for compatibility reasons only.
# We do not fix deprecated functions on old code.
set_source_files_properties(
        ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/httpreadwrite.cpp
        ${PUPNP_UPNP_SOURCE_DIR}/src/gena/gena_device.cpp
        ${PUPNP_UPNP_SOURCE_DIR}/src/uuid/uuid.cpp
    PROPERTIES COMPILE_FLAGS
        $<$<CXX_COMPILER_ID:AppleClang>:"-Wno-deprecated-declarations">
)

set_source_files_properties(
        ${PUPNP_SOURCE_FILES}
        ${PUPNP_UPNP_SOURCE_DIR}/src/gena/gena_ctrlpt.cpp
        ${PUPNP_UPNP_SOURCE_DIR}/src/ssdp/ssdp_ctrlpt.cpp
        ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/service-table/service_table.cpp
        ${PUPNP_UPNP_SOURCE_DIR}/src/soap/soap_device.cpp
        ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/service-table/service_table.cpp
        ${PUPNP_UPNP_SOURCE_DIR}/src/ssdp/ssdp_device.cpp
        ${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpdebug.cpp
        ${PUPNP_UPNP_SOURCE_DIR}/src/gena/gena_device.cpp
        ${PUPNP_UPNP_SOURCE_DIR}/src/uuid/sysdep.cpp
    # Default low warning level on pUPnP source files.
    PROPERTIES COMPILE_OPTIONS $<$<CXX_COMPILER_ID:MSVC>:/W1>
)


if(NOT WIN32)

    # pupnp compatible libraries
    #===========================

    # pupnp Compatible SHared library: suffix -csh
    #---------------------------------------------
    add_library(compa_shared SHARED
        ${COMPATIBLE_SOURCE_FILES}
    )
    target_include_directories(compa_shared
        ${COMPATIBLE_INCLUDE_DIRECTORIES}
        PUBLIC
            # if we are not on WIN32 the linker finds empty "headers.h" and will
            # not complain an error but will not use <headers.h> that isn't available.
            ${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32
    )
    target_compile_definitions(compa_shared
        PRIVATE
            UPNPLIB_SHARED
            UPNPLIB_EXPORTS
            $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    target_link_libraries(compa_shared
        PUBLIC
            umock_shared
            $<$<BOOL:${UPNPLIB_DEF_OPENSSL}>:upnplib_ssl>
        INTERFACE
            ixml_shared
        PRIVATE
            upnplib_shared
    )

    # pupnp Compatible STatic library: suffix -cst
    #---------------------------------------------
    add_library(compa_static STATIC
            ${COMPATIBLE_SOURCE_FILES}
    )
    target_include_directories(compa_static
        ${COMPATIBLE_INCLUDE_DIRECTORIES}
        PUBLIC
            # if we are not on WIN32 the linker finds empty "headers.h" and will
            # not complain an error but will not use <headers.h> that isn't available.
            ${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32
    )
    target_compile_definitions(compa_static
        PRIVATE
            $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    target_link_libraries(compa_static
        PUBLIC
            umock_static
            $<$<BOOL:${UPNPLIB_DEF_OPENSSL}>:upnplib_ssl>
        INTERFACE
            ixml_static
        PRIVATE
            upnplib_static
    )

else() # WIN32

    # WIN32 pupnp compatible libraries
    #=================================

    # WIN32 pupnp Compatible SHared library: suffix -csh
    #---------------------------------------------------
    add_library(compa_shared SHARED
        ${COMPATIBLE_SOURCE_FILES}
    )
    target_include_directories(compa_shared
        ${COMPATIBLE_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions(compa_shared
        PRIVATE
            UPNPLIB_SHARED
            UPNPLIB_EXPORTS
            $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    target_link_libraries(compa_shared
        PUBLIC
            umock_shared
            ixml_shared
            $<$<BOOL:${UPNPLIB_DEF_OPENSSL}>:upnplib_ssl>
        PRIVATE
            upnplib_shared
            # disable warning LNK4217: symbol 'symbol' defined in
            # 'filename_1.obj' is imported by 'filename_2.obj' in function
            # 'function'
            -ignore:4217
    )

    # WIN32 pupnp Compatible STatic library: suffix -cst
    #---------------------------------------------------
    add_library(compa_static STATIC
        ${COMPATIBLE_SOURCE_FILES}
    )
    target_include_directories(compa_static
        ${COMPATIBLE_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions(compa_static
        PRIVATE
            $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    target_link_libraries(compa_static
        PUBLIC
            umock_static
            $<$<BOOL:${UPNPLIB_DEF_OPENSSL}>:upnplib_ssl>
        INTERFACE
            ixml_static
        PRIVATE
            upnplib_static
    )

endif() # WIN32
