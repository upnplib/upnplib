# Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2023-04-17

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(UPNPLIB VERSION 0011
        DESCRIPTION "UPnP library program"
        HOMEPAGE_URL "https://github.com/upnplib")

add_compile_definitions(
    $<$<BOOL:${UPNPLIB_WITH_TRACE}>:UPNPLIB_WITH_TRACE>
    # Disable warnings to use e.g. strncpy_s instead of strncpy
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)

add_compile_options(
    # Most compiler warnings enabled
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    # all compiler warnings as errors
    $<$<CXX_COMPILER_ID:MSVC>:/WX>
)

set(UPNPLIB_SOURCE_FILES
    ./src/init.cpp
    ./src/api/upnptools.cpp
    ./src/net/addrinfo.cpp
    ./src/net/sock.cpp
    ./src/net/socket.cpp
    ./src/net/http/webserver.cpp
)

set(UPNPLIB_INCLUDE_DIRECTORIES
    PUBLIC
        ./include
    INTERFACE
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR} # needed for pthread.h
)

set(UPNPLIBEXE_INCLUDE_DIRECTORIES
    # Next variable is needed for one step configure. Don't use
    # PRIVATE ${COMPA_SOURCE_DIR}/include
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}/compa/include
    PRIVATE ${UPNPLIB_PROJECT_BINARY_DIR}/include
)

if(NOT WIN32)

    # upnplib libraries
    # =================

    # Upnplib SHared library: suffix -ush
    #------------------------------------
    add_library(upnplib_shared SHARED
            ${UPNPLIB_SOURCE_FILES}
            ./src/strndup.cpp
    )
    target_include_directories(upnplib_shared
            ${UPNPLIB_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions(upnplib_shared
        PRIVATE
            UPNPLIB_SHARED
            UPNPLIB_EXPORTS
            $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    target_link_libraries(upnplib_shared
        PUBLIC
            umock_shared
        INTERFACE
            ixml_shared
            # On Linux we cannot link with the static pthreads library because then
            # we have also to link with a static libc that's not available by default.
            ${UPNP_PTHREADS_SHARED_LIBRARY}
    )

    # Upnplib STatic library: suffix -ust
    #------------------------------------
    add_library(upnplib_static STATIC
            ${UPNPLIB_SOURCE_FILES}
    )
    target_include_directories(upnplib_static
            ${UPNPLIB_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions(upnplib_static
        PRIVATE
            $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    target_link_libraries(upnplib_static
        PUBLIC
            umock_static
        INTERFACE
            ixml_static
            # On Linux we cannot link with the static pthreads library because then
            # we have also to link with a static libc that's not available by default.
            ${UPNP_PTHREADS_SHARED_LIBRARY}
    )

    # Info executable linked with the pupnp shared library
    # ----------------------------------------------------
    add_executable(upnplib-psh
            ./src/upnplib.cpp
    )
    target_include_directories(upnplib-psh
        ${UPNPLIBEXE_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions (upnplib-psh
        PRIVATE
            UPNPLIB_WITH_NATIVE_PUPNP
            UPNPLIB_SHARED
    )
    target_link_libraries(upnplib-psh
        PRIVATE
            pupnp_shared
    )

    # Info executable linked with the pupnp static library
    #-----------------------------------------------------
    add_executable(upnplib-pst
            ./src/upnplib.cpp
    )
    target_include_directories(upnplib-pst
        ${UPNPLIBEXE_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions (upnplib-pst
        PRIVATE
            UPNPLIB_WITH_NATIVE_PUPNP
    )
    target_link_libraries(upnplib-pst
        PRIVATE
            pupnp_static
            # On Linux we cannot link with the static pthreads library
            # because then we have also link with a static libc that's not
            # available by default.
            ${UPNP_PTHREADS_SHARED_LIBRARY}
    )

    # Info executable linked with the compatible shared library
    # ---------------------------------------------------------
    add_executable(upnplib-csh
            ./src/upnplib.cpp
    )
    target_include_directories(upnplib-csh
        ${UPNPLIBEXE_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions (upnplib-csh
        PRIVATE
            UPNPLIB_SHARED
    )
    target_link_libraries(upnplib-csh
        PRIVATE
            compa_shared
    )

    # Info executable linked with the compatible static library
    #----------------------------------------------------------
    add_executable(upnplib-cst
            ./src/upnplib.cpp
    )
    target_include_directories(upnplib-cst
        ${UPNPLIBEXE_INCLUDE_DIRECTORIES}
    )
    target_link_libraries(upnplib-cst
        PRIVATE
            compa_static
            # On Linux we cannot link with the static pthreads library
            # because then we have also link with a static libc that's not
            # available by default.
            ${UPNP_PTHREADS_SHARED_LIBRARY}
    )

else() # WIN32

    # WIN32 upnplib libraries
    #========================

    # WIN32 Upnplib SHared library: suffix -ush
    #------------------------------------------
    add_library(upnplib_shared SHARED
            ${UPNPLIB_SOURCE_FILES}
            ./src/strndup.cpp
    )
    target_include_directories(upnplib_shared
        ${UPNPLIB_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions(upnplib_shared
        PRIVATE
            UPNPLIB_SHARED
            UPNPLIB_EXPORTS
            $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    target_link_libraries(upnplib_shared
        PUBLIC
            umock_shared
            # In addition to pthreads we need some more system libraries here
            ws2_32   # winsock to support sockets
            iphlpapi # ip helper interface to get network management tools
            # On MS Windows we link with the static pthreads4w library to
            # avoid error prone managing access to its .dll file.
            ${UPNP_PTHREADS_STATIC_LIBRARY}
    )

    # WIN32 Upnplib STatic library: suffix -ust
    #------------------------------------------
    add_library(upnplib_static STATIC
            ${UPNPLIB_SOURCE_FILES}
    )
    target_include_directories(upnplib_static
        ${UPNPLIB_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions(upnplib_static
        PRIVATE
            $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    target_link_libraries(upnplib_static
        PUBLIC
            umock_static
        INTERFACE
            ws2_32   # winsock to support sockets
            iphlpapi # ip helper interface to get network management tools
            # On MS Windows we link with the static pthreads4w library to
            # avoid error prone managing access to its .dll file.
            ${UPNP_PTHREADS_STATIC_LIBRARY}
    )

    # WIN32 Info executable linked with the pupnp shared library
    #-----------------------------------------------------------
    add_executable(upnplib-psh
            ./src/upnplib.cpp
    )
    target_include_directories(upnplib-psh
        ${UPNPLIBEXE_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions (upnplib-psh
        PRIVATE
            UPNPLIB_WITH_NATIVE_PUPNP
            UPNPLIB_SHARED
    )
    target_link_libraries(upnplib-psh
        PRIVATE
            pupnp_shared
    )

    # WIN32 Info executable linked with the pupnp static library
    #-----------------------------------------------------------
    add_executable(upnplib-pst
            ./src/upnplib.cpp
    )
    target_include_directories(upnplib-pst
        ${UPNPLIBEXE_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions (upnplib-pst
        PRIVATE
            UPNPLIB_WITH_NATIVE_PUPNP
    )
    target_link_libraries(upnplib-pst
        PRIVATE
            pupnp_static
    )

    # WIN32 Info executable linked with the compatible shared library
    #----------------------------------------------------------------
    add_executable(upnplib-csh
            ./src/upnplib.cpp
    )
    target_include_directories(upnplib-csh
        ${UPNPLIBEXE_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions (upnplib-csh
        PRIVATE
            UPNPLIB_SHARED
    )
    target_link_libraries(upnplib-csh
        PRIVATE
            compa_shared
    )

    # WIN32 Info executable linked with the compatible static library
    #----------------------------------------------------------------
    add_executable(upnplib-cst
            ./src/upnplib.cpp
    )
    target_include_directories(upnplib-cst
        ${UPNPLIBEXE_INCLUDE_DIRECTORIES}
    )
    target_link_libraries(upnplib-cst
        PRIVATE
            compa_static
    )

endif() # WIN32
