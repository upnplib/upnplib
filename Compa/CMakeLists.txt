# Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2024-08-05

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(COMPA VERSION 0008
              DESCRIPTION "UPnPlib compatible library subproject"
              HOMEPAGE_URL "https://github.com/upnplib")

# Switches for this Subproject
#-----------------------------
# These switches are all handled only in this configuration file.

# Options selectable by user configuration
set(COMPA_DEF_OPENSSL ${UPNPLIB_WITH_OPENSSL})
set(COMPA_DEF_MINISERVER ${UPNPLIB_WITH_MINISERVER})
set(COMPA_DEF_WEBSERVER ${UPNPLIB_WITH_WEBSERVER})
set(COMPA_DEF_CTRLPT_SSDP ${UPNPLIB_WITH_CTRLPT_SSDP})  # client -> UPNP_ENABLE_CLIENT_API
set(COMPA_DEF_DEVICE_SSDP ${UPNPLIB_WITH_DEVICE_SSDP})
set(COMPA_DEF_OPTION_SSDP ${UPNPLIB_WITH_OPTION_SSDP}) # optssdp -> UPNP_ENABLE_OPTIONAL_SSDP_HEADERS
set(COMPA_DEF_CTRLPT_DESCRIPTION ${UPNPLIB_WITH_CTRLPT_DESCRIPTION})
set(COMPA_DEF_DEVICE_DESCRIPTION ${UPNPLIB_WITH_DEVICE_DESCRIPTION})
set(COMPA_DEF_CTRLPT_SOAP ${UPNPLIB_WITH_CTRLPT_SOAP})
set(COMPA_DEF_DEVICE_SOAP ${UPNPLIB_WITH_DEVICE_SOAP})
set(COMPA_DEF_CTRLPT_GENA ${UPNPLIB_WITH_CTRLPT_GENA})
set(COMPA_DEF_DEVICE_GENA ${UPNPLIB_WITH_DEVICE_GENA})
set(COMPA_DEF_TOOLS ${UPNPLIB_WITH_TOOLS})         # tools -> UPNP_ENABLE_HELPER_API_TOOLS
set(COMPA_DEF_IXML ${UPNPLIB_WITH_IXML})           # scriptsupport -> IXML_HAVE_SCRIPTSUPPORT
set(COMPA_DEF_GOOGLETEST ${UPNPLIB_WITH_GOOGLETEST})
#set(COMPA_DEF_BLOCKTCP ${UPNPLIB_WITH_BLOCKTCP})  # blocking_tcp_connections -> COMPA_ENABLE_BLOCKING_TCP_CONNECTIONS
#set(COMPA_DEF_UNSPECSVR ${UPNPLIB_WITH_UNSPECIFIED_SERVER}) # unspecified_server -> UPNP_ENABLE_UNSPECIFIED_SERVER
#set(COMPA_DEF_REUSEADDR NO)                       # reuseaddr -> UPNP_MINISERVER_REUSEADDR
if(UPNPLIB_WITH_SAMPLES)                           # all samples -> UPNP_BUILD_SAMPLES
    set(COMPA_DEF_SAMPLES YES) # To make it separate selectable don't set it direct
endif()


# Dependencies of options. The order is important.
#-------------------------------------------------
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(COMPA_DEF_DEBUG YES)
    set(COMPA_DEF_TOOLS YES)
endif()

if(COMPA_DEF_GOOGLETEST)
    set(COMPA_DEF_CTRLPT_GENA YES)
    set(COMPA_DEF_DEVICE_GENA YES)
    set(COMPA_DEF_OPTION_SSDP YES)
endif()

if(COMPA_DEF_SAMPLES)
    set(COMPA_DEF_CTRLPT_GENA YES)
    set(COMPA_DEF_DEVICE_GENA YES)
    set(COMPA_DEF_TOOLS YES)
endif()

if(COMPA_DEF_CTRLPT_GENA)
    set(COMPA_DEF_CTRLPT_SOAP YES)
endif()

if(COMPA_DEF_DEVICE_GENA)
    set(COMPA_DEF_DEVICE_SOAP YES)
endif()

if(COMPA_DEF_CTRLPT_SOAP)
    set(COMPA_DEF_CTRLPT_DESCRIPTION YES)
endif()

if(COMPA_DEF_DEVICE_SOAP)
    set(COMPA_DEF_DEVICE_DESCRIPTION YES)
endif()

if(COMPA_DEF_CTRLPT_DESCRIPTION)
    set(COMPA_DEF_CTRLPT_SSDP YES)
endif()

if(COMPA_DEF_DEVICE_DESCRIPTION)
    set(COMPA_DEF_DEVICE_SSDP YES)
endif()

if(COMPA_DEF_CTRLPT_SSDP OR COMPA_DEF_DEVICE_SSDP)
    set(COMPA_DEF_WEBSERVER YES)
endif()

if(COMPA_DEF_WEBSERVER)
    set(COMPA_DEF_MINISERVER YES)
endif()


# Source files and compile settings
# =================================

set(COMPA_SOURCE_FILES
    src/genlib/util/membuffer.cpp
    src/genlib/util/list.cpp
    src/genlib/util/strintmap.cpp
    src/api/UpnpString.cpp
    src/genlib/net/sock.cpp
    src/api/upnpapi.cpp

    src/threadutil/FreeList.cpp # Only for LinkedList, ThreadPool, TimerThread
    src/threadutil/LinkedList.cpp
    src/threadutil/ThreadPool.cpp
    src/threadutil/TimerThread.cpp

    # Miniserver
    $<$<BOOL:${COMPA_DEF_MINISERVER}>:src/genlib/miniserver/miniserver.cpp>

    # Webserver
    $<$<BOOL:${COMPA_DEF_WEBSERVER}>:src/genlib/net/http/webserver.cpp>
    $<$<BOOL:${COMPA_DEF_WEBSERVER}>:src/genlib/net/http/httpparser.cpp>
    $<$<BOOL:${COMPA_DEF_WEBSERVER}>:src/genlib/net/http/httpreadwrite.cpp>
    $<$<BOOL:${COMPA_DEF_WEBSERVER}>:src/genlib/net/http/statcodes.cpp>
    $<$<BOOL:${COMPA_DEF_WEBSERVER}>:src/genlib/net/http/parsetools.cpp>
    $<$<BOOL:${COMPA_DEF_WEBSERVER}>:src/api/UpnpExtraHeaders.cpp>
    $<$<BOOL:${COMPA_DEF_WEBSERVER}>:src/api/UpnpFileInfo.cpp>
    $<$<OR:$<BOOL:${COMPA_DEF_WEBSERVER}>,$<BOOL:${COMPA_DEF_TOOLS}>>:src/genlib/net/uri/uri.cpp>

    # Common
    # -------------------------------------------------------------------------
    # SSDP
    $<$<OR:$<BOOL:${COMPA_DEF_CTRLPT_SSDP}>,$<BOOL:${COMPA_DEF_DEVICE_SSDP}>>:src/ssdp/ssdp_common.cpp>
    # GENA
    $<$<OR:$<BOOL:${COMPA_DEF_CTRLPT_GENA}>,$<BOOL:${COMPA_DEF_DEVICE_GENA}>>:src/gena/gena_callback2.cpp>
    $<$<OR:$<BOOL:${COMPA_DEF_CTRLPT_GENA}>,$<BOOL:${COMPA_DEF_DEVICE_GENA}>,$<BOOL:${COMPA_DEF_OPTION_SSDP}>>:src/uuid/md5.cpp> # needed only by uuid.cpp and sysdep.cpp
    $<$<OR:$<BOOL:${COMPA_DEF_CTRLPT_GENA}>,$<BOOL:${COMPA_DEF_DEVICE_GENA}>,$<BOOL:${COMPA_DEF_OPTION_SSDP}>>:src/uuid/sysdep.cpp> # needed only by uuid.cpp
    $<$<OR:$<BOOL:${COMPA_DEF_CTRLPT_GENA}>,$<BOOL:${COMPA_DEF_DEVICE_GENA}>,$<BOOL:${COMPA_DEF_OPTION_SSDP}>>:src/uuid/uuid.cpp>

    #Control Point
    # -------------------------------------------------------------------------
    # SSDP
    $<$<BOOL:${COMPA_DEF_CTRLPT_SSDP}>:src/ssdp/ssdp_ctrlpt.cpp>
    $<$<BOOL:${COMPA_DEF_CTRLPT_SSDP}>:src/ssdp/SSDPResultData.cpp>
    $<$<BOOL:${COMPA_DEF_CTRLPT_SSDP}>:src/ssdp/SSDPResultDataCallback.cpp>
    $<$<BOOL:${COMPA_DEF_CTRLPT_SSDP}>:src/api/UpnpDiscovery.cpp>
    # SOAP
    $<$<BOOL:${COMPA_DEF_CTRLPT_SOAP}>:src/soap/soap_ctrlpt.cpp>
    $<$<BOOL:${COMPA_DEF_CTRLPT_SOAP}>:src/api/UpnpActionComplete.cpp>
    $<$<BOOL:${COMPA_DEF_CTRLPT_SOAP}>:src/api/UpnpStateVarComplete.cpp>
    # GENA
    $<$<BOOL:${COMPA_DEF_CTRLPT_GENA}>:src/gena/gena_ctrlpt.cpp>
    $<$<BOOL:${COMPA_DEF_CTRLPT_GENA}>:src/genlib/client_table/GenlibClientSubscription.cpp>
    $<$<BOOL:${COMPA_DEF_CTRLPT_GENA}>:src/genlib/client_table/client_table.cpp>
    $<$<BOOL:${COMPA_DEF_CTRLPT_GENA}>:src/api/UpnpEventSubscribe.cpp>
    $<$<BOOL:${COMPA_DEF_CTRLPT_GENA}>:src/api/UpnpEvent.cpp>

    # UPnP Device
    # -------------------------------------------------------------------------
    # SSDP
    $<$<BOOL:${COMPA_DEF_DEVICE_SSDP}>:src/ssdp/ssdp_device.cpp>
    $<$<BOOL:${COMPA_DEF_DEVICE_SSDP}>:src/genlib/service-table/service_table.cpp>
    # Description
    $<$<BOOL:${COMPA_DEF_DEVICE_DESCRIPTION}>:src/urlconfig/urlconfig.cpp>
    # SOAP
    $<$<BOOL:${COMPA_DEF_DEVICE_SOAP}>:src/soap/soap_device.cpp>
    $<$<BOOL:${COMPA_DEF_DEVICE_SOAP}>:src/api/UpnpActionRequest.cpp>
    $<$<BOOL:${COMPA_DEF_DEVICE_SOAP}>:src/api/UpnpStateVarRequest.cpp>
    # GENA
    $<$<BOOL:${COMPA_DEF_DEVICE_GENA}>:src/gena/gena_device.cpp>
    $<$<BOOL:${COMPA_DEF_DEVICE_GENA}>:src/api/UpnpSubscriptionRequest.cpp>

    # Other sources (need rework)
    # -------------------------------------------------------------------------
    $<$<BOOL:${COMPA_DEF_TOOLS}>:src/api/upnptools.cpp>
    $<$<CONFIG:Debug>:src/api/upnpdebug.cpp>

    ${UMOCK_SOURCE_DIR}/src/pupnp_sock.cpp
    ${UPNPLIB_PROJECT_SOURCE_DIR}/Interface/src/pupnp-sock.cpp

    # Helper classes added here. They are assigned with the UPnPlib rules, e.g.
    # throwing exceptions etc. They only encapsulate old functions mainly for
    # testing with init and shutdown following the RAII paradigm.
    ${PUPNP_SOURCE_DIR}/src/api/upnpdebug.cpp
    ${PUPNP_SOURCE_DIR}/src/threadpool_init.cpp
)

set(COMPA_INCLUDE_DIRECTORIES
    PUBLIC inc
    PUBLIC src/inc
    PUBLIC src/threadutil
    PUBLIC ${PUPNP_SOURCE_DIR}/include
    PUBLIC ${UPNPLIB_PROJECT_SOURCE_DIR}/Interface/include
    PRIVATE ${PUPNP_IXML_SOURCE_DIR}/inc
)

set(COMPA_COMPILE_DEFINITIONS
    PUBLIC
        UPNP_ENABLE_IPV6
        COMPA_ENABLE_BLOCKING_TCP_CONNECTIONS

        $<$<BOOL:${COMPA_DEF_MINISERVER}>:COMPA_HAVE_MINISERVER>

        $<$<BOOL:${COMPA_DEF_WEBSERVER}>:COMPA_HAVE_WEBSERVER>

        $<$<BOOL:${COMPA_DEF_CTRLPT_SSDP}>:COMPA_HAVE_CTRLPT_SSDP>
        $<$<BOOL:${COMPA_DEF_DEVICE_SSDP}>:COMPA_HAVE_DEVICE_SSDP>
        $<$<BOOL:${COMPA_DEF_OPTION_SSDP}>:COMPA_HAVE_OPTION_SSDP>

        $<$<BOOL:${COMPA_DEF_CTRLPT_DESCRIPTION}>:COMPA_HAVE_CTRLPT_DESCRIPTION>
        $<$<BOOL:${COMPA_DEF_DEVICE_DESCRIPTION}>:COMPA_HAVE_DEVICE_DESCRIPTION>

        $<$<BOOL:${COMPA_DEF_CTRLPT_SOAP}>:COMPA_HAVE_CTRLPT_SOAP>
        $<$<BOOL:${COMPA_DEF_DEVICE_SOAP}>:COMPA_HAVE_DEVICE_SOAP>

        $<$<BOOL:${COMPA_DEF_CTRLPT_GENA}>:COMPA_HAVE_CTRLPT_GENA>
        $<$<BOOL:${COMPA_DEF_DEVICE_GENA}>:COMPA_HAVE_DEVICE_GENA>

        $<$<BOOL:${COMPA_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
        $<$<BOOL:${COMPA_DEF_TOOLS}>:UPNP_HAVE_TOOLS>
        $<$<BOOL:${COMPA_DEF_DEBUG}>:UPNP_HAVE_DEBUG>
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
        PUBLIC umock_static
        INTERFACE ixml_static
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
        PUBLIC umock_static
        INTERFACE ixml_static
    )

endif() # WIN32

# vim: nowrap
