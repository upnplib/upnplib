# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-10-25

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(UPNPLIB VERSION 0010
                DESCRIPTION "UPnP library program"
                HOMEPAGE_URL "https://github.com/upnplib")


set(UPNPLIB_SOURCE_FILES
    ./src/global.cpp
    ./src/api/upnptools.cpp
    ./src/net/sock.cpp
    ./src/net/http/webserver.cpp
)

set(UPNPLIB_INCLUDE_DIRECTORIES
    PRIVATE
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        $<$<PLATFORM_ID:Windows>:${pthreads4w_SOURCE_DIR}> # needed for pthread.h
)

if(NOT WIN32)

    # upnplib libraries
    #==================

    # Upnplib SHared library: postfix -ush
    #-------------------------------------
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

    # Upnplib STatic library: postfix -ust
    #-------------------------------------
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
        # Static does not use a linker, no matter to use PRIVATE or PUBLIC.
        # For propagation only INTERFACE is needed.
        INTERFACE
            ixml_static
            # On Linux we cannot link with the static pthreads library because then
            # we have also to link with a static libc that's not available by default.
            ${UPNP_PTHREADS_SHARED_LIBRARY}
    )

    # Info executable linked with the upnplib compatible shared library
    #------------------------------------------------------------------
    add_executable(upnplibInfo_shared
            ./src/upnplibInfo.cpp
    )
    target_include_directories(upnplibInfo_shared
        PRIVATE
            ${UPNPLIB_PROJECT_SOURCE_DIR}/include
            ${UPNPLIB_PROJECT_BINARY_DIR}/include
            ${PUPNP_UPNP_SOURCE_DIR}/inc
            ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
            ${PUPNP_IXML_SOURCE_DIR}/inc
    )
    target_compile_definitions (upnplibInfo_shared
        PRIVATE
            UPNPLIB_SHARED
    )
    target_link_libraries(upnplibInfo_shared
        PRIVATE
            compa_shared
    )

    # Info executable linked with the pupnp static library
    #-----------------------------------------------------
    add_executable(upnplibInfo_static
            ./src/upnplibInfo.cpp
    )
    target_include_directories(upnplibInfo_static
        PRIVATE
            ${UPNPLIB_PROJECT_SOURCE_DIR}/include
            ${UPNPLIB_PROJECT_BINARY_DIR}/include
            ${PUPNP_UPNP_SOURCE_DIR}/inc
            ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
            ${PUPNP_IXML_SOURCE_DIR}/inc
    )
    target_compile_definitions (upnplibInfo_static
        PRIVATE
            UPNPLIB_WITH_NATIVE_PUPNP
    )
    target_link_libraries(upnplibInfo_static
        PRIVATE
            pupnp_static
            # On Linux we cannot link with the static pthreads library
            # because then we have also link with a static libc that's not
            # available by default.
            ${UPNP_PTHREADS_SHARED_LIBRARY}
    )

else() # WIN32

    # WIN32 upnplib libraries
    #========================

    # WIN32 Upnplib SHared library: postfix -ush
    #-------------------------------------------
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

    # WIN32 Upnplib STatic library: postfix -ust
    #-------------------------------------------
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
        # Static does not use a linker, no matter to use PRIVATE or PUBLIC.
        # For propagation only INTERFACE is needed.
        INTERFACE
            umock_static
            ws2_32   # winsock to support sockets
            iphlpapi # ip helper interface to get network management tools
            # On MS Windows we link with the static pthreads4w library to
            # avoid error prone managing access to its .dll file.
            ${UPNP_PTHREADS_STATIC_LIBRARY}
    )

    # WIN32 Info executable linked with the upnplib compatible shared library
    #------------------------------------------------------------------------
    add_executable(upnplibInfo_shared
            ./src/upnplibInfo.cpp
    )
    target_include_directories(upnplibInfo_shared
        PRIVATE
            ${UPNPLIB_PROJECT_SOURCE_DIR}/include
            ${UPNPLIB_PROJECT_BINARY_DIR}/include
            ${PUPNP_UPNP_SOURCE_DIR}/inc   # needed for ithread.hpp
            ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
            ${PUPNP_IXML_SOURCE_DIR}/inc
            ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
    )
    target_compile_definitions (upnplibInfo_shared
        PRIVATE
            UPNPLIB_SHARED
            $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
    )
    target_link_libraries(upnplibInfo_shared
        PRIVATE
            compa_shared
    )

    # WIN32 Info executable linked with the pupnp static library
    #-----------------------------------------------------------
    add_executable(upnplibInfo_static
            ./src/upnplibInfo.cpp
    )
    target_include_directories(upnplibInfo_static
        PRIVATE
            ${UPNPLIB_PROJECT_SOURCE_DIR}/include
            ${UPNPLIB_PROJECT_BINARY_DIR}/include
            ${PUPNP_UPNP_SOURCE_DIR}/inc  # needed for ithread.hpp
            ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
            ${PUPNP_IXML_SOURCE_DIR}/inc
            ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
    )
    target_compile_definitions (upnplibInfo_static
        PRIVATE
            UPNPLIB_WITH_NATIVE_PUPNP
            $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
    )
    target_link_libraries(upnplibInfo_static
        PRIVATE
            pupnp_static
    )

endif() # WIN32
