# Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-10-25

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(COMPA VERSION 0001
              DESCRIPTION "UPnPlib compatible library subproject"
              HOMEPAGE_URL "https://github.com/upnplib")


set(COMPATIBLE_SOURCE_FILES
# There are additional source files at different add_library() statements.
    ${UPNPLIB_SOURCE_DIR}/src/init.cpp
    ${UPNPLIB_PROJECT_SOURCE_DIR}/compa/src/api/UpnpString.cpp
)

set(UPNPLIB_COMPATIBLE_INCLUDE_DIRECTORIES
    PRIVATE
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
        ${UPNPLIB_PROJECT_SOURCE_DIR}/compa/inc
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
            ${UPNPLIB_COMPATIBLE_INCLUDE_DIRECTORIES}
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
        INTERFACE
            # If we do not have compatible code we use the old pupnp code.
            pupnp_shared
    )

    # pupnp Compatible STatic library: postfix -cst
    #----------------------------------------------
    add_library(compa_static STATIC
            ${COMPATIBLE_SOURCE_FILES}
    )
    target_include_directories(compa_static
            ${UPNPLIB_COMPATIBLE_INCLUDE_DIRECTORIES}
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
            ${UPNPLIB_COMPATIBLE_INCLUDE_DIRECTORIES}
            # if pthreads4w isn't installed this path is empty. The linker
            # doesn't find "pthread.h" and falls back to look at <pthread.h>.
            # (Only for general use. It doesn't matter here because we are
            # already on WIN32).
        INTERFACE
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
        INTERFACE
            # If we do not have compatible code we use the old pupnp code.
            pupnp_shared
    )

    # WIN32 pupnp Compatible STatic library: postfix -cst
    #----------------------------------------------------
    add_library(compa_static STATIC
        ${COMPATIBLE_SOURCE_FILES}
    )
    target_include_directories(compa_static
            ${UPNPLIB_COMPATIBLE_INCLUDE_DIRECTORIES}
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
