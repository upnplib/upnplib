# Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-11-29

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(COMPA VERSION 0002
              DESCRIPTION "UPnPlib compatible library subproject"
              HOMEPAGE_URL "https://github.com/upnplib")


set(COMPATIBLE_SOURCE_FILES
# There are additional source files at different add_library() statements.
    ./src/api/UpnpString.cpp
    ./src/api/UpnpFileInfo.cpp
    ./src/genlib/miniserver/miniserver.cpp
)

set(COMPATIBLE_INCLUDE_DIRECTORIES
    PRIVATE
        ./inc
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_IXML_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        $<$<NOT:$<PLATFORM_ID:Windows>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
)

add_compile_definitions(
    UPNP_HAVE_WEBSERVER   # Needed to have it always available in the library
    # Disable warnings to use e.g. strncpy_s instead of strncpy
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)

add_compile_options(
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wconversion>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wsign-conversion>
)

if(NOT WIN32)

    # pupnp compatible libraries
    #===========================

    # pupnp Compatible SHared library: postfix -csh
    #----------------------------------------------
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
            $<$<PLATFORM_ID:Darwin>:umock_shared>
            $<$<PLATFORM_ID:Darwin>:upnplib_shared>
        PUBLIC   # needed on macOS
            # If we do not have compatible code we use the old pupnp code.
            pupnp_shared
    )

    # pupnp Compatible STatic library: postfix -cst
    #----------------------------------------------
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
        # Static does not use a linker, no matter to use PRIVATE or PUBLIC.
        # For propagation only INTERFACE is needed.
        INTERFACE
            # If we do not have compatible code we use the old pupnp code.
            pupnp_static
    )

else() # WIN32

    # WIN32 pupnp compatible libraries
    #=================================

    # WIN32 pupnp Compatible SHared library: postfix -csh
    #----------------------------------------------------
    add_library(compa_shared SHARED
        ${COMPATIBLE_SOURCE_FILES}
        ${UPNPLIB_SOURCE_DIR}/src/strndup.cpp
    )
    target_include_directories(compa_shared
            ${COMPATIBLE_INCLUDE_DIRECTORIES}
        PRIVATE
            # if pthreads4w isn't installed this path is empty. The linker
            # doesn't find "pthread.h" and falls back to look at <pthread.h>.
            ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
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
        PUBLIC   # needed on macOS
            # If we do not have compatible code we use the old pupnp code.
            pupnp_shared
    )

    # WIN32 pupnp Compatible STatic library: postfix -cst
    #----------------------------------------------------
    add_library(compa_static STATIC
        ${COMPATIBLE_SOURCE_FILES}
    )
    target_include_directories(compa_static
            ${COMPATIBLE_INCLUDE_DIRECTORIES}
        PRIVATE
            # if pthreads4w isn't installed this path is empty. The linker
            # doesn't find "pthread.h" and falls back to look at <pthread.h>.
            ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
    )
    target_compile_definitions(compa_static
        PRIVATE
            $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    target_link_libraries(compa_static
        # Static does not use a linker, no matter to use PRIVATE or PUBLIC.
        # For propagation only INTERFACE is needed.
        INTERFACE
            # If we do not have compatible code we use the old pupnp code.
            pupnp_static
    )

endif() # WIN32
