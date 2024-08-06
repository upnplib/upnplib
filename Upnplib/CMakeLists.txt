# Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2024-08-15

cmake_minimum_required(VERSION 3.23) # for FILE_SET
include(../cmake/project-header.cmake)

project(UPNPLIB VERSION 0012
        DESCRIPTION "UPnP library program"
        HOMEPAGE_URL "https://github.com/upnplib")

add_compile_definitions(
    # Disable warnings to use e.g. strncpy_s instead of strncpy
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)

set(UPNPLIB_SOURCE_FILES
    src/api/upnptools.cpp
    src/net/netaddr.cpp
    src/net/sockaddr.cpp
    src/net/addrinfo.cpp
    src/net/socket.cpp
    src/net/connection_common.cpp
    src/net/http/webserver.cpp
    src/net/http/httpreadwrite.cpp
)

set(UPNPLIB_PUBLIC_HEADERS
    include/upnplib/port_sock.hpp
    include/upnplib/visibility.hpp
)

set(UPNPLIB_INCLUDE_DIRECTORIES
    PUBLIC include
)


if(NOT WIN32)

    # upnplib libraries
    # =================

    # Upnplib SHared library: suffix -ush
    #------------------------------------
    add_library(upnplib_shared SHARED
            ${UPNPLIB_SOURCE_FILES}
    )
    set_target_properties(upnplib_shared PROPERTIES
            OUTPUT_NAME upnpsdk
    )
    target_include_directories(upnplib_shared
            ${UPNPLIB_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions(upnplib_shared
        PRIVATE
            UPNPLIB_EXPORTS
            $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    target_link_libraries(upnplib_shared
        PUBLIC umock_shared
        INTERFACE ixml_shared
    )
    target_sources(upnplib_shared
        INTERFACE FILE_SET upnplib_public_headers TYPE HEADERS
                  BASE_DIRS include
                  FILES ${UPNPLIB_PUBLIC_HEADERS}
    )
    install(TARGETS upnplib_shared
        FILE_SET upnplib_public_headers
        DESTINATION include/upnp
    )

    # Upnplib STatic library: suffix -ust
    #------------------------------------
    add_library(upnplib_static STATIC
            ${UPNPLIB_SOURCE_FILES}
    )
    set_target_properties(upnplib_static PROPERTIES
            OUTPUT_NAME upnpsdk
    )
    target_include_directories(upnplib_static
            ${UPNPLIB_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions(upnplib_static
        PRIVATE
            $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    target_link_libraries(upnplib_static
        PUBLIC umock_static
        INTERFACE ixml_static
    )

    install(TARGETS upnplib_shared upnplib_static)

else() # WIN32

    # WIN32 upnplib libraries
    #========================

    # WIN32 Upnplib SHared library: suffix -ush
    #------------------------------------------
    add_library(upnplib_shared SHARED
            ${UPNPLIB_SOURCE_FILES}
    )
    set_target_properties(upnplib_shared PROPERTIES
            OUTPUT_NAME upnpsdk
    )
    target_include_directories(upnplib_shared
        ${UPNPLIB_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions(upnplib_shared
        PRIVATE
            UPNPLIB_EXPORTS
            $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    target_link_libraries(upnplib_shared
        PUBLIC umock_shared
    )

    # WIN32 Upnplib STatic library: suffix -ust
    #------------------------------------------
    add_library(upnplib_static STATIC
            ${UPNPLIB_SOURCE_FILES}
    )
    set_target_properties(upnplib_static PROPERTIES
            OUTPUT_NAME upnpsdk-static
    )
    target_include_directories(upnplib_static
        ${UPNPLIB_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions(upnplib_static
        PRIVATE
            $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    target_link_libraries(upnplib_static
        PUBLIC umock_static
    )

    install(TARGETS upnplib_shared upnplib_static)


endif() # WIN32
