# Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2023-04-17

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(COMPA VERSION 0002
              DESCRIPTION "UPnPlib compatible library subproject"
              HOMEPAGE_URL "https://github.com/upnplib")


set(COMPATIBLE_SOURCE_FILES
# There are additional source files at different add_library() statements.
    ./src/init.cpp
    ./src/api/UpnpString.cpp
    ./src/api/UpnpFileInfo.cpp
    ./src/api/upnpdebug.cpp
    ./src/genlib/miniserver/miniserver.cpp
    ./src/genlib/net/sock.cpp
    ./src/ssdp/ssdp_server.cpp
)

set(COMPATIBLE_INCLUDE_DIRECTORIES
    PUBLIC ./include
    PUBLIC ${IXML_SOURCE_DIR}/include
    PRIVATE ${UPNPLIB_SOURCE_DIR}/include
)

add_compile_definitions(
    UPNP_HAVE_WEBSERVER   # Needed to have it always available in the library
    $<$<BOOL:${UPNPLIB_WITH_TRACE}>:UPNPLIB_WITH_TRACE>
    # Disable warnings to use e.g. strncpy_s instead of strncpy
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)

add_compile_options(
    # Most compiler warnings enabled
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    # all compiler warnings as errors
    $<$<CXX_COMPILER_ID:MSVC>:/WX>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wconversion>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wsign-conversion>
)

if(NOT WIN32)

    # pupnp compatible libraries
    #===========================

    # pupnp Compatible SHared library: suffix -csh
    #---------------------------------------------
    add_library(compa_shared SHARED
        ${COMPATIBLE_SOURCE_FILES}
        ${UPNPLIB_SOURCE_DIR}/src/strndup.cpp
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
        PRIVATE
            upnplib_shared
            $<$<PLATFORM_ID:Darwin>:umock_shared>
        PUBLIC
            # If we do not have compatible code we use the old pupnp code.
            pupnp_shared
    )

    # pupnp Compatible STatic library: suffix -cst
    #---------------------------------------------
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
            # If we do not have compatible code we use the old pupnp code.
            pupnp_static
    )

else() # WIN32

    # WIN32 pupnp compatible libraries
    #=================================

    # WIN32 pupnp Compatible SHared library: suffix -csh
    #---------------------------------------------------
    add_library(compa_shared SHARED
        ${COMPATIBLE_SOURCE_FILES}
        ${UPNPLIB_SOURCE_DIR}/src/strndup.cpp
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
        PRIVATE
            umock_shared
            upnplib_shared
        PUBLIC
            # If we do not have compatible code we use the old pupnp code.
            pupnp_shared
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
            # If we do not have compatible code we use the old pupnp code.
            pupnp_static
    )

endif() # WIN32
