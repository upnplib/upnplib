# Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2023-10-08

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(COMPA VERSION 0005
              DESCRIPTION "UPnPlib compatible library subproject"
              HOMEPAGE_URL "https://github.com/upnplib")

# Switches for this Subproject
#-----------------------------
# These switches are all handled only in this configuration file.

# Options selectable by user configuration
set(COMPA_DEF_GOOGLETEST ${UPNPLIB_WITH_GOOGLETEST})
set(COMPA_DEF_TOOLS ${UPNPLIB_WITH_TOOLS})         # tools -> UPNP_ENABLE_HELPER_API_TOOLS
set(COMPA_DEF_CLIENT ${UPNPLIB_WITH_CLIENT})       # client -> UPNP_ENABLE_CLIENT_API
set(COMPA_DEF_DEVICE ${UPNPLIB_WITH_DEVICE})
set(COMPA_DEF_WEBSERVER ${UPNPLIB_WITH_WEBSERVER}) # webserver -> UPNP_ENABLE_WEBSERVER
                                                     #   only enabled with COMPA_DEF_DEVICE
set(COMPA_DEF_SSDP ${UPNPLIB_WITH_SSDP})           # ssdp -> UPNP_ENABLE_SSDP
set(COMPA_DEF_OPTSSDP ${UPNPLIB_WITH_OPTSSDP})     # optssdp -> UPNP_ENABLE_OPTIONAL_SSDP_HEADERS
set(COMPA_DEF_SOAP ${UPNPLIB_WITH_SOAP})           # soap -> UPNP_ENABLE_SOAP
                                                     #   only enabled with COMPA_DEF_WEBSERVER
set(COMPA_DEF_GENA ${UPNPLIB_WITH_GENA})           # gena -> UPNP_ENABLE_GENA
                                                     #   compiles only with COMPA_DEF_CLIENT and COMPA_DEF_DEVICE
set(COMPA_DEF_OPENSSL ${UPNPLIB_WITH_OPENSSL})     # open_ssl -> UPNP_ENABLE_OPEN_SSL
set(COMPA_DEF_BLOCKTCP ${UPNPLIB_WITH_BLOCKTCP})   # blocking_tcp_connections -> UPNP_ENABLE_BLOCKING_TCP_CONNECTIONS
set(COMPA_DEF_IXML ${UPNPLIB_WITH_IXML})           # scriptsupport -> IXML_HAVE_SCRIPTSUPPORT
#set(COMPA_DEF_UNSPECSVR ${UPNPLIB_WITH_UNSPECIFIED_SERVER}) # unspecified_server -> UPNP_ENABLE_UNSPECIFIED_SERVER
#set(COMPA_DEF_REUSEADDR NO)                        # reuseaddr -> UPNP_MINISERVER_REUSEADDR
if(UPNPLIB_WITH_SAMPLES)                             # all samples -> UPNP_BUILD_SAMPLES
    set(COMPA_DEF_SAMPLES YES) # To make it separate selectable don't set it direct
endif()

# Dependencies of options
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(COMPA_DEF_DEBUG YES)
endif()

if(COMPA_DEF_GOOGLETEST)
    set(COMPA_DEF_CLIENT YES)
    set(COMPA_DEF_OPTSSDP YES)
    set(COMPA_DEF_GENA YES)
endif()

if(COMPA_DEF_SAMPLES)
    set(COMPA_DEF_TOOLS YES)
    set(COMPA_DEF_CLIENT YES)
    set(COMPA_DEF_GENA YES)
endif()

if(COMPA_DEF_GENA)
    set(COMPA_DEF_WEBSERVER YES)
    set(COMPA_DEF_CLIENT YES)
endif()

if(COMPA_DEF_DEVICE)
    set(COMPA_DEF_WEBSERVER YES)
endif()

if(COMPA_DEF_WEBSERVER)
    set(COMPA_DEF_DEVICE YES)
    set(COMPA_DEF_SOAP YES)
endif()

if(COMPA_DEF_SOAP)
    set(COMPA_DEF_SSDP YES)
endif()


# Source files and compile settings
#----------------------------------

set(COMPA_SOURCE_FILES
    ./src/init.cpp

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
    $<$<AND:$<BOOL:${COMPA_DEF_DEVICE}>,$<BOOL:${COMPA_DEF_SOAP}>>:./src/genlib/util/util.cpp>

    $<$<BOOL:${COMPA_DEF_DEVICE}>:./src/genlib/service-table/service_table.cpp>
    $<$<AND:$<BOOL:${COMPA_DEF_DEVICE}>,$<BOOL:${COMPA_DEF_GENA}>>:./src/genlib/client_table/GenlibClientSubscription.cpp>
    $<$<AND:$<BOOL:${COMPA_DEF_DEVICE}>,$<BOOL:${COMPA_DEF_GENA}>>:./src/genlib/client_table/client_table.cpp>
    ./src/genlib/miniserver/miniserver.cpp

    $<$<BOOL:${COMPA_DEF_SSDP}>:./src/ssdp/ssdp_server.cpp>
    $<$<BOOL:${COMPA_DEF_SSDP}>:./src/ssdp/ssdp_device.cpp>
    $<$<BOOL:${COMPA_DEF_SSDP}>:./src/ssdp/ssdp_ctrlpt.cpp>
    $<$<BOOL:${COMPA_DEF_SSDP}>:./src/ssdp/SSDPResultData.cpp>
    $<$<BOOL:${COMPA_DEF_SSDP}>:./src/ssdp/SSDPResultDataCallback.cpp>

    $<$<AND:$<BOOL:${COMPA_DEF_DEVICE}>,$<BOOL:${COMPA_DEF_SOAP}>>:./src/soap/soap_device.cpp>
    $<$<AND:$<BOOL:${COMPA_DEF_DEVICE}>,$<BOOL:${COMPA_DEF_SOAP}>>:./src/soap/soap_common.cpp>
    $<$<AND:$<BOOL:${COMPA_DEF_DEVICE}>,$<BOOL:${COMPA_DEF_SOAP}>>:./src/soap/soap_ctrlpt.cpp>

    $<$<AND:$<BOOL:${COMPA_DEF_DEVICE}>,$<BOOL:${COMPA_DEF_GENA}>>:./src/gena/gena_device.cpp>
    $<$<AND:$<BOOL:${COMPA_DEF_DEVICE}>,$<BOOL:${COMPA_DEF_GENA}>>:./src/gena/gena_callback2.cpp>
    $<$<AND:$<BOOL:${COMPA_DEF_DEVICE}>,$<BOOL:${COMPA_DEF_GENA}>>:./src/gena/gena_ctrlpt.cpp>

    $<$<OR:$<AND:$<BOOL:${COMPA_DEF_DEVICE}>,$<BOOL:${COMPA_DEF_GENA}>>,$<BOOL:${COMPA_DEF_OPTSSDP}>>:./src/uuid/uuid.cpp>
    $<$<OR:$<AND:$<BOOL:${COMPA_DEF_DEVICE}>,$<BOOL:${COMPA_DEF_GENA}>>,$<BOOL:${COMPA_DEF_OPTSSDP}>>:./src/uuid/md5.cpp>
    $<$<OR:$<AND:$<BOOL:${COMPA_DEF_DEVICE}>,$<BOOL:${COMPA_DEF_GENA}>>,$<BOOL:${COMPA_DEF_OPTSSDP}>>:./src/uuid/sysdep.cpp>

    ./src/api/UpnpExtraHeaders.cpp
    ./src/api/UpnpString.cpp
    ./src/api/upnpapi.cpp
    $<$<OR:$<BOOL:${COMPA_DEF_TOOLS}>,$<BOOL:${COMPA_DEF_DEBUG}>>:./src/api/upnptools.cpp>
    $<$<BOOL:${COMPA_DEF_WEBSERVER}>:./src/api/UpnpFileInfo.cpp>
    #$<$<BOOL:${COMPA_DEF_SSDP}>:./src/api/UpnpDiscovery.cpp>
    $<$<OR:$<BOOL:${COMPA_DEF_SSDP}>,$<BOOL:${COMPA_DEF_DEBUG}>>:./src/api/UpnpDiscovery.cpp>

    $<$<OR:$<AND:$<BOOL:${COMPA_DEF_DEVICE}>,$<BOOL:${COMPA_DEF_SOAP}>>,$<BOOL:${COMPA_DEF_DEBUG}>>:./src/api/UpnpActionRequest.cpp>
    $<$<OR:$<AND:$<BOOL:${COMPA_DEF_DEVICE}>,$<BOOL:${COMPA_DEF_SOAP}>>,$<BOOL:${COMPA_DEF_DEBUG}>>:./src/api/UpnpActionComplete.cpp>
    $<$<OR:$<AND:$<BOOL:${COMPA_DEF_DEVICE}>,$<BOOL:${COMPA_DEF_SOAP}>>,$<BOOL:${COMPA_DEF_DEBUG}>>:./src/api/UpnpStateVarRequest.cpp>
    $<$<OR:$<AND:$<BOOL:${COMPA_DEF_DEVICE}>,$<BOOL:${COMPA_DEF_SOAP}>>,$<BOOL:${COMPA_DEF_DEBUG}>>:./src/api/UpnpStateVarComplete.cpp>
    $<$<OR:$<AND:$<BOOL:${COMPA_DEF_DEVICE}>,$<BOOL:${COMPA_DEF_SOAP}>>,$<BOOL:${COMPA_DEF_DEBUG}>>:./src/api/UpnpSubscriptionRequest.cpp>
    $<$<OR:$<AND:$<BOOL:${COMPA_DEF_DEVICE}>,$<BOOL:${COMPA_DEF_SOAP}>>,$<BOOL:${COMPA_DEF_DEBUG}>>:./src/api/UpnpEvent.cpp>
    $<$<OR:$<AND:$<BOOL:${COMPA_DEF_DEVICE}>,$<BOOL:${COMPA_DEF_SOAP}>>,$<BOOL:${COMPA_DEF_DEBUG}>>:./src/api/UpnpEventSubscribe.cpp>
    $<$<AND:$<BOOL:${COMPA_DEF_DEVICE}>,$<BOOL:${COMPA_DEF_SOAP}>>:./src/api/UpnpStateVarRequest.cpp>
    $<$<AND:$<BOOL:${COMPA_DEF_DEVICE}>,$<BOOL:${COMPA_DEF_SOAP}>>:./src/api/UpnpActionComplete.cpp>
    $<$<AND:$<BOOL:${COMPA_DEF_DEVICE}>,$<BOOL:${COMPA_DEF_SOAP}>>:./src/api/UpnpStateVarComplete.cpp>
    $<$<AND:$<BOOL:${COMPA_DEF_DEVICE}>,$<BOOL:${COMPA_DEF_GENA}>>:./src/api/UpnpSubscriptionRequest.cpp>
    $<$<AND:$<BOOL:${COMPA_DEF_DEVICE}>,$<BOOL:${COMPA_DEF_GENA}>>:./src/api/UpnpEventSubscribe.cpp>
    $<$<AND:$<BOOL:${COMPA_DEF_DEVICE}>,$<BOOL:${COMPA_DEF_GENA}>>:./src/api/UpnpEvent.cpp>
    $<$<CONFIG:Debug>:./src/api/upnpdebug.cpp>

    $<$<AND:$<BOOL:${COMPA_DEF_DEVICE}>,$<BOOL:${COMPA_DEF_WEBSERVER}>>:./src/urlconfig/urlconfig.cpp>

    ${UMOCK_SOURCE_DIR}/src/pupnp_sock.cpp
    ${UPNPLIB_PROJECT_SOURCE_DIR}/interface/src/pupnp-sock.cpp

    ${PUPNP_SOURCE_DIR}/src/api/upnpdebug.cpp
)

set(COMPA_INCLUDE_DIRECTORIES
    PUBLIC inc
    PUBLIC src/inc
    PUBLIC src/threadutil
    PUBLIC ${PUPNP_SOURCE_DIR}/include
    PUBLIC ${UPNPLIB_PROJECT_SOURCE_DIR}/interface/include
    PRIVATE ${PUPNP_IXML_SOURCE_DIR}/inc
)

set(COMPA_COMPILE_DEFINITIONS
    PUBLIC
        $<$<BOOL:${COMPA_DEF_DEBUG}>:UPNP_HAVE_DEBUG>
        UPNP_ENABLE_IPV6
        $<$<BOOL:${COMPA_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
        $<$<BOOL:${COMPA_DEF_TOOLS}>:UPNP_HAVE_TOOLS>
        $<$<BOOL:${COMPA_DEF_WEBSERVER}>:UPNP_HAVE_WEBSERVER>
        $<$<BOOL:${COMPA_DEF_SSDP}>:UPNP_HAVE_SSDP>
        $<$<BOOL:${COMPA_DEF_OPTSSDP}>:UPNP_HAVE_OPTSSDP>
        $<$<BOOL:${COMPA_DEF_SOAP}>:UPNP_HAVE_SOAP>
        $<$<BOOL:${COMPA_DEF_GENA}>:UPNP_HAVE_GENA>
        $<$<BOOL:${COMPA_DEF_CLIENT}>:UPNP_HAVE_CLIENT>
        $<$<BOOL:${COMPA_DEF_DEVICE}>:UPNP_HAVE_DEVICE>
        $<$<BOOL:${COMPA_DEF_BLOCKTCP}>:UPNP_ENABLE_BLOCKING_TCP_CONNECTIONS>
)

set_source_files_properties(
        src/threadutil/ThreadPool.cpp
        # disable warning C4244: 'argument': conversion from 'unsigned __int64'
        # to 'unsigned int', possible loss of data
    PROPERTIES COMPILE_OPTIONS $<$<CXX_COMPILER_ID:MSVC>:/wd4244>
)


# Create libraries
#=================
if(NOT WIN32)

    # Compa SHared library: suffix -csh
    #----------------------------------
    add_library(compa_shared SHARED
            ${COMPA_SOURCE_FILES}
    )
    target_include_directories(compa_shared
        ${COMPA_INCLUDE_DIRECTORIES}
        PUBLIC
            # if we are not on WIN32 the linker finds empty "headers.h" and
            # will not complain an error but will not use <headers.h> that
            # isn't available.
            inc/not_win32
    )
    target_compile_definitions(compa_shared
            ${COMPA_COMPILE_DEFINITIONS}
        PRIVATE UPNPLIB_EXPORTS
    )
    target_link_libraries(compa_shared
        PUBLIC
            umock_shared
            ixml_shared
            $<$<BOOL:${COMPA_DEF_OPENSSL}>:upnplib_ssl>
        PRIVATE
            upnplib_shared
    )

    # Compa STatic library: suffix -cst
    #----------------------------------
    add_library(compa_static STATIC
            ${COMPA_SOURCE_FILES}
    )
    target_include_directories(compa_static
        ${COMPA_INCLUDE_DIRECTORIES}
        PUBLIC
            # if we are not on WIN32 the linker finds empty "headers.h" and
            # will not complain an error but will not use <headers.h> that
            # isn't available.
            inc/not_win32
    )
    target_compile_definitions(compa_static
            ${COMPA_COMPILE_DEFINITIONS}
    )
    target_link_libraries(compa_static
        PUBLIC
            umock_static
            $<$<BOOL:${COMPA_DEF_OPENSSL}>:upnplib_ssl>
        INTERFACE
            ixml_static
    )

else() # WIN32

    # Link with POSIX Threads for Windows (pthreads4w)
    #=================================================
    # Pthreads are not supported by MS Windows. So we have installed the
    # third party library pthreads4w.

    # WIN32 Compa SHared library: -csh
    #---------------------------------
    add_library(compa_shared SHARED
            ${COMPA_SOURCE_FILES}
    )
    target_include_directories(compa_shared
        ${COMPA_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions(compa_shared
            ${COMPA_COMPILE_DEFINITIONS}
        PRIVATE
            UPNPLIB_EXPORTS
            $<$<BOOL:${COMPA_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    target_link_libraries(compa_shared
        PUBLIC
            umock_shared
            ixml_shared
            $<$<BOOL:${COMPA_DEF_OPENSSL}>:upnplib_ssl>
        PRIVATE
            upnplib_shared
            # disable warning LNK4217: symbol 'symbol' defined in
            # 'filename_1.obj' is imported by 'filename_2.obj' in function
            # 'function'.
            # This is why a source is included into a library and a test
            # includes the source and also uses the library.
            -ignore:4217
    )

    # WIN32 Compa STatic library: suffix -cst
    #----------------------------------------
    add_library(compa_static STATIC
            ${COMPA_SOURCE_FILES}
    )
    target_include_directories(compa_static
        ${COMPA_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions(compa_static
            ${COMPA_COMPILE_DEFINITIONS}
        PRIVATE
            $<$<BOOL:${COMPA_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    target_link_libraries(compa_static
        PUBLIC
            umock_static
            $<$<BOOL:${COMPA_DEF_OPENSSL}>:upnplib_ssl>
        INTERFACE
            ixml_static
    )

endif() # WIN32

# vim: nowrap
