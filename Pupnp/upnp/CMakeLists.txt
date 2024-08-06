# Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2024-08-15

cmake_minimum_required(VERSION 3.23) # for FILE_SET
include(../../cmake/project-header.cmake)

project(PUPNP_UPNP VERSION 0030
        DESCRIPTION "Binary build of the UPnP library core"
        HOMEPAGE_URL "https://github.com/upnplib")

# This contains all settings to compile the library dynamically and static.
#==========================================================================

# Switches for this Subproject
#-----------------------------
# These switches are all handled only in this configuration file.

# Options selectable by user configuration
set(PUPNP_DEF_GOOGLETEST ${UPNPLIB_WITH_GOOGLETEST})
set(PUPNP_DEF_TOOLS YES) #${UPNPLIB_WITH_TOOLS})         # tools -> UPNP_ENABLE_HELPER_API_TOOLS
set(PUPNP_DEF_CLIENT YES) #${UPNPLIB_WITH_CLIENT})       # client -> UPNP_ENABLE_CLIENT_API
set(PUPNP_DEF_DEVICE YES) #${UPNPLIB_WITH_DEVICE})
set(PUPNP_DEF_WEBSERVER YES) #${UPNPLIB_WITH_WEBSERVER}) # webserver -> UPNP_ENABLE_WEBSERVER
                                                     #   only enabled with PUPNP_DEF_DEVICE
set(PUPNP_DEF_OPTSSDP YES) #${UPNPLIB_WITH_OPTSSDP})     # optssdp -> UPNP_ENABLE_OPTIONAL_SSDP_HEADERS
set(PUPNP_DEF_SOAP YES) #${UPNPLIB_WITH_SOAP})           # soap -> UPNP_ENABLE_SOAP
                                                     #   only enabled with PUPNP_DEF_WEBSERVER
set(PUPNP_DEF_GENA YES) #${UPNPLIB_WITH_GENA})           # gena -> UPNP_ENABLE_GENA
                                                     #   compiles only with PUPNP_DEF_CLIENT and PUPNP_DEF_DEVICE
set(PUPNP_DEF_OPENSSL ${UPNPLIB_WITH_OPENSSL})     # open_ssl -> UPNP_ENABLE_OPEN_SSL
set(PUPNP_DEF_BLOCKTCP NO) #${UPNPLIB_WITH_BLOCKTCP})   # blocking_tcp_connections -> UPNP_ENABLE_BLOCKING_TCP_CONNECTIONS
set(PUPNP_DEF_IXML YES) #${UPNPLIB_WITH_IXML})           # scriptsupport -> IXML_HAVE_SCRIPTSUPPORT
#set(PUPNP_DEF_UNSPECSVR NO)                        # unspecified_server -> UPNP_ENABLE_UNSPECIFIED_SERVER
#set(PUPNP_DEF_REUSEADDR NO)                        # reuseaddr -> UPNP_MINISERVER_REUSEADDR
if(UPNPLIB_WITH_SAMPLES)                             # all samples -> UPNP_BUILD_SAMPLES
    set(PUPNP_DEF_SAMPLES YES) # To make it separate selectable don't set it direct
endif()

# Dependencies of options
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(PUPNP_DEF_DEBUG YES)
endif()

if(PUPNP_DEF_GOOGLETEST)
    set(PUPNP_DEF_CLIENT YES)
    set(PUPNP_DEF_OPTSSDP YES)
    set(PUPNP_DEF_GENA YES)
endif()

if(PUPNP_DEF_SAMPLES)
    set(PUPNP_DEF_TOOLS YES)
    set(PUPNP_DEF_CLIENT YES)
    set(PUPNP_DEF_GENA YES)
endif()

if(PUPNP_DEF_GENA)
    set(PUPNP_DEF_WEBSERVER YES)
    set(PUPNP_DEF_CLIENT YES)
endif()

if(PUPNP_DEF_CLIENT)
    set(PUPNP_DEF_SSDP YES)
endif()

if(PUPNP_DEF_DEVICE)
    set(PUPNP_DEF_WEBSERVER YES)
    set(PUPNP_DEF_SSDP YES)
endif()

if(PUPNP_DEF_WEBSERVER)
    set(PUPNP_DEF_DEVICE YES)
    set(PUPNP_DEF_SOAP YES)
endif()

if(PUPNP_DEF_SOAP)
    set(PUPNP_DEF_SSDP YES)
endif()


# Source files and compile settings
#----------------------------------

set(PUPNP_SOURCE_FILES
    src/threadutil/FreeList.cpp
    src/threadutil/LinkedList.cpp
    src/threadutil/ThreadPool.cpp
    src/threadutil/TimerThread.cpp

    src/genlib/net/sock.cpp
    src/genlib/net/uri/uri.cpp

    src/genlib/net/http/httpparser.cpp
    src/genlib/net/http/httpreadwrite.cpp
    src/genlib/net/http/statcodes.cpp
    src/genlib/net/http/webserver.cpp
    src/genlib/net/http/parsetools.cpp

    src/genlib/util/list.cpp
    src/genlib/util/membuffer.cpp
    src/genlib/util/strintmap.cpp
    $<$<AND:$<BOOL:${PUPNP_DEF_DEVICE}>,$<BOOL:${PUPNP_DEF_SOAP}>>:src/genlib/util/util.cpp>

    $<$<BOOL:${PUPNP_DEF_DEVICE}>:src/genlib/service-table/service_table.cpp>
    $<$<AND:$<BOOL:${PUPNP_DEF_DEVICE}>,$<BOOL:${PUPNP_DEF_GENA}>>:src/genlib/client_table/GenlibClientSubscription.cpp>
    $<$<AND:$<BOOL:${PUPNP_DEF_DEVICE}>,$<BOOL:${PUPNP_DEF_GENA}>>:src/genlib/client_table/client_table.cpp>
    src/genlib/miniserver/miniserver.cpp

    $<$<BOOL:${PUPNP_DEF_SSDP}>:src/ssdp/ssdp_server.cpp>
    $<$<BOOL:${PUPNP_DEF_DEVICE}>:src/ssdp/ssdp_device.cpp>
    $<$<BOOL:${PUPNP_DEF_CLIENT}>:src/ssdp/ssdp_ctrlpt.cpp>
    $<$<BOOL:${PUPNP_DEF_CLIENT}>:src/ssdp/SSDPResultData.cpp>
    $<$<BOOL:${PUPNP_DEF_CLIENT}>:src/ssdp/SSDPResultDataCallback.cpp>

    $<$<AND:$<BOOL:${PUPNP_DEF_DEVICE}>,$<BOOL:${PUPNP_DEF_SOAP}>>:src/soap/soap_device.cpp>
    $<$<AND:$<BOOL:${PUPNP_DEF_DEVICE}>,$<BOOL:${PUPNP_DEF_SOAP}>>:src/soap/soap_common.cpp>
    $<$<AND:$<BOOL:${PUPNP_DEF_DEVICE}>,$<BOOL:${PUPNP_DEF_SOAP}>>:src/soap/soap_ctrlpt.cpp>

    $<$<AND:$<BOOL:${PUPNP_DEF_DEVICE}>,$<BOOL:${PUPNP_DEF_GENA}>>:src/gena/gena_device.cpp>
    $<$<AND:$<BOOL:${PUPNP_DEF_DEVICE}>,$<BOOL:${PUPNP_DEF_GENA}>>:src/gena/gena_callback2.cpp>
    $<$<AND:$<BOOL:${PUPNP_DEF_DEVICE}>,$<BOOL:${PUPNP_DEF_GENA}>>:src/gena/gena_ctrlpt.cpp>

    $<$<OR:$<AND:$<BOOL:${PUPNP_DEF_DEVICE}>,$<BOOL:${PUPNP_DEF_GENA}>>,$<BOOL:${PUPNP_DEF_OPTSSDP}>>:src/uuid/uuid.cpp>
    $<$<OR:$<AND:$<BOOL:${PUPNP_DEF_DEVICE}>,$<BOOL:${PUPNP_DEF_GENA}>>,$<BOOL:${PUPNP_DEF_OPTSSDP}>>:src/uuid/md5.cpp>
    $<$<OR:$<AND:$<BOOL:${PUPNP_DEF_DEVICE}>,$<BOOL:${PUPNP_DEF_GENA}>>,$<BOOL:${PUPNP_DEF_OPTSSDP}>>:src/uuid/sysdep.cpp>

    src/api/UpnpExtraHeaders.cpp
    src/api/UpnpString.cpp
    src/api/upnpapi.cpp
    $<$<OR:$<BOOL:${PUPNP_DEF_TOOLS}>,$<BOOL:${PUPNP_DEF_DEBUG}>>:src/api/upnptools.cpp>
    $<$<BOOL:${PUPNP_DEF_WEBSERVER}>:src/api/UpnpFileInfo.cpp>
    $<$<BOOL:${PUPNP_DEF_CLIENT}>:src/api/UpnpDiscovery.cpp>
    #$<$<OR:$<BOOL:${PUPNP_DEF_CLIENT}>,$<BOOL:${PUPNP_DEF_DEBUG}>>:src/api/UpnpDiscovery.cpp>

    $<$<OR:$<AND:$<BOOL:${PUPNP_DEF_DEVICE}>,$<BOOL:${PUPNP_DEF_SOAP}>>,$<BOOL:${PUPNP_DEF_DEBUG}>>:src/api/UpnpActionRequest.cpp>
    $<$<OR:$<AND:$<BOOL:${PUPNP_DEF_DEVICE}>,$<BOOL:${PUPNP_DEF_SOAP}>>,$<BOOL:${PUPNP_DEF_DEBUG}>>:src/api/UpnpActionComplete.cpp>
    $<$<OR:$<AND:$<BOOL:${PUPNP_DEF_DEVICE}>,$<BOOL:${PUPNP_DEF_SOAP}>>,$<BOOL:${PUPNP_DEF_DEBUG}>>:src/api/UpnpStateVarRequest.cpp>
    $<$<OR:$<AND:$<BOOL:${PUPNP_DEF_DEVICE}>,$<BOOL:${PUPNP_DEF_SOAP}>>,$<BOOL:${PUPNP_DEF_DEBUG}>>:src/api/UpnpStateVarComplete.cpp>
    $<$<OR:$<AND:$<BOOL:${PUPNP_DEF_DEVICE}>,$<BOOL:${PUPNP_DEF_SOAP}>>,$<BOOL:${PUPNP_DEF_DEBUG}>>:src/api/UpnpSubscriptionRequest.cpp>
    $<$<OR:$<AND:$<BOOL:${PUPNP_DEF_DEVICE}>,$<BOOL:${PUPNP_DEF_SOAP}>>,$<BOOL:${PUPNP_DEF_DEBUG}>>:src/api/UpnpEvent.cpp>
    $<$<OR:$<AND:$<BOOL:${PUPNP_DEF_DEVICE}>,$<BOOL:${PUPNP_DEF_SOAP}>>,$<BOOL:${PUPNP_DEF_DEBUG}>>:src/api/UpnpEventSubscribe.cpp>
    $<$<AND:$<BOOL:${PUPNP_DEF_DEVICE}>,$<BOOL:${PUPNP_DEF_SOAP}>>:src/api/UpnpStateVarRequest.cpp>
    $<$<AND:$<BOOL:${PUPNP_DEF_DEVICE}>,$<BOOL:${PUPNP_DEF_SOAP}>>:src/api/UpnpActionComplete.cpp>
    $<$<AND:$<BOOL:${PUPNP_DEF_DEVICE}>,$<BOOL:${PUPNP_DEF_SOAP}>>:src/api/UpnpStateVarComplete.cpp>
    $<$<AND:$<BOOL:${PUPNP_DEF_DEVICE}>,$<BOOL:${PUPNP_DEF_GENA}>>:src/api/UpnpSubscriptionRequest.cpp>
    $<$<AND:$<BOOL:${PUPNP_DEF_DEVICE}>,$<BOOL:${PUPNP_DEF_GENA}>>:src/api/UpnpEventSubscribe.cpp>
    $<$<AND:$<BOOL:${PUPNP_DEF_DEVICE}>,$<BOOL:${PUPNP_DEF_GENA}>>:src/api/UpnpEvent.cpp>
    src/api/upnpdebug.cpp

    $<$<AND:$<BOOL:${PUPNP_DEF_DEVICE}>,$<BOOL:${PUPNP_DEF_WEBSERVER}>>:src/urlconfig/urlconfig.cpp>

    ${UMOCK_SOURCE_DIR}/src/pupnp_sock.cpp
    ${CMAKE_SOURCE_DIR}/Interface/src/pupnp-sock.cpp

    # Helper classes added but not original old pupnp code. To separate them,
    # they are in a parent directory of the original code. They only
    # encapsulate old fuctions mainly for testing with init and shutdown
    # following the RAII paradigm. They are assigned with the UPnPlib rules,
    # e.g. throwing exceptions etc.
    ../src/api/upnpdebug.cpp
    ../src/threadpool_init.cpp
)

set(PUPNP_PUBLIC_HEADERS
    inc/Callback.hpp
    inc/list.hpp
    inc/upnp.h
    inc/upnp.hpp
    inc/UpnpActionComplete.hpp
    inc/UpnpActionRequest.hpp
    inc/UpnpDiscovery.hpp
    inc/UpnpEvent.hpp
    inc/UpnpEventSubscribe.hpp
    inc/UpnpExtraHeaders.hpp
    inc/UpnpFileInfo.hpp
    inc/UpnpGlobal.hpp
    inc/UpnpInet.hpp
    inc/UpnpIntTypes.hpp
    inc/UpnpStateVarComplete.hpp
    inc/UpnpStateVarRequest.hpp
    inc/UpnpStdInt.hpp
    inc/UpnpString.hpp
    inc/UpnpSubscriptionRequest.hpp
    inc/UpnpUniStd.hpp
    inc/upnptools.h
    inc/upnptools.hpp
    inc/ithread.h
    inc/ithread.hpp
    inc/upnpdebug.h
    inc/upnpdebug.hpp
    ${CMAKE_BINARY_DIR}/include/cmake_vars.hpp
)

set(PUPNP_INCLUDE_DIRECTORIES
    PUBLIC inc
    PUBLIC src/inc
    PUBLIC src/threadutil
    PUBLIC ${PUPNP_SOURCE_DIR}/include
    PUBLIC ${CMAKE_SOURCE_DIR}/Interface/include
    PRIVATE ${PUPNP_IXML_SOURCE_DIR}/inc
)

set(PUPNP_COMPILE_DEFINITIONS
    PUBLIC
        UPNP_ENABLE_IPV6
        $<$<BOOL:${PUPNP_DEF_DEBUG}>:UPNP_HAVE_DEBUG>
        $<$<BOOL:${PUPNP_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
        $<$<BOOL:${PUPNP_DEF_TOOLS}>:UPNP_HAVE_TOOLS>
        $<$<BOOL:${PUPNP_DEF_WEBSERVER}>:UPNP_HAVE_WEBSERVER>
        $<$<BOOL:${PUPNP_DEF_SSDP}>:UPNP_HAVE_SSDP>
        $<$<BOOL:${PUPNP_DEF_OPTSSDP}>:UPNP_HAVE_OPTSSDP>
        $<$<BOOL:${PUPNP_DEF_SOAP}>:UPNP_HAVE_SOAP>
        $<$<BOOL:${PUPNP_DEF_GENA}>:UPNP_HAVE_GENA>
        $<$<BOOL:${PUPNP_DEF_CLIENT}>:UPNP_HAVE_CLIENT>
        $<$<BOOL:${PUPNP_DEF_DEVICE}>:UPNP_HAVE_DEVICE>
        $<$<BOOL:${PUPNP_DEF_BLOCKTCP}>:UPNP_ENABLE_BLOCKING_TCP_CONNECTIONS>
)


# Create libraries
#=================
if(NOT WIN32)

    # Pupnp SHared library: suffix -psh
    #----------------------------------
    add_library(pupnp_shared SHARED
            ${PUPNP_SOURCE_FILES}
    )
    set_target_properties(pupnp_shared PROPERTIES
            OUTPUT_NAME upnpsdk-pupnp
    )
    target_include_directories(pupnp_shared
        ${PUPNP_INCLUDE_DIRECTORIES}
        PUBLIC
            # if we are not on WIN32 the linker finds empty "headers.h" and will
            # not complain an error but will not use <headers.h> that isn't available.
            inc/not_win32
    )
    target_compile_definitions(pupnp_shared
        ${PUPNP_COMPILE_DEFINITIONS}
        PUBLIC UPNPLIB_WITH_NATIVE_PUPNP
        PRIVATE UPNPLIB_EXPORTS
    )
    target_link_libraries(pupnp_shared
        PUBLIC
            umock_shared
            ixml_shared
    )
    target_sources(pupnp_shared
        INTERFACE FILE_SET pupnp_public_headers TYPE HEADERS
                  BASE_DIRS inc
                            ${CMAKE_BINARY_DIR}/include
                  FILES ${PUPNP_PUBLIC_HEADERS}
    )
    install(TARGETS pupnp_shared
        FILE_SET pupnp_public_headers
        DESTINATION include/upnp
    )

    # Pupnp STatic library: suffix -pst
    #----------------------------------
    add_library(pupnp_static STATIC
            ${PUPNP_SOURCE_FILES}
    )
    set_target_properties(pupnp_static PROPERTIES
            OUTPUT_NAME upnpsdk-pupnp
    )
    target_include_directories(pupnp_static
        ${PUPNP_INCLUDE_DIRECTORIES}
        PUBLIC
            # if we are not on WIN32 the linker finds empty "headers.h" and will
            # not complain an error but will not use <headers.h> that isn't available.
            inc/not_win32
    )
    target_compile_definitions(pupnp_static
        ${PUPNP_COMPILE_DEFINITIONS}
        PUBLIC UPNPLIB_WITH_NATIVE_PUPNP
    )
    target_link_libraries(pupnp_static
        PUBLIC umock_static
        INTERFACE ixml_static
    )

    install(TARGETS pupnp_static)

else() # WIN32

    # Link with POSIX Threads for Windows (pthreads4w)
    #=================================================
    # Pthreads are not supported by MS Windows. So we have installed the
    # third party library pthreads4w.

    # WIN32 Pupnp SHared library: -psh
    #---------------------------------
    add_library(pupnp_shared SHARED
            ${PUPNP_SOURCE_FILES}
    )
    set_target_properties(pupnp_shared PROPERTIES
            OUTPUT_NAME upnpsdk-pupnp
    )
    target_include_directories(pupnp_shared
        ${PUPNP_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions(pupnp_shared
        ${PUPNP_COMPILE_DEFINITIONS}
        PUBLIC UPNPLIB_WITH_NATIVE_PUPNP
        PRIVATE
            UPNPLIB_EXPORTS
            $<$<BOOL:${PUPNP_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    target_link_libraries(pupnp_shared
        PUBLIC
            umock_shared
            ixml_shared
    )

    # WIN32 Pupnp STatic library: suffix -pst
    #----------------------------------------
    add_library(pupnp_static STATIC
            ${PUPNP_SOURCE_FILES}
    )
    set_target_properties(pupnp_static PROPERTIES
            OUTPUT_NAME upnpsdk-pupnp-static
    )
    target_include_directories(pupnp_static
        ${PUPNP_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions(pupnp_static
        ${PUPNP_COMPILE_DEFINITIONS}
        PUBLIC UPNPLIB_WITH_NATIVE_PUPNP
        PRIVATE
            $<$<BOOL:${PUPNP_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    target_link_libraries(pupnp_static
        PUBLIC umock_static
        INTERFACE ixml_static
    )

    install(TARGETS pupnp_shared pupnp_static)

endif() # WIN32

# vim: nowrap
