# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-07-15

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(UPNPLIB_CORE VERSION 0006
                     DESCRIPTION "UPnP library core program"
                     HOMEPAGE_URL "https://github.com/upnplib")

if(NOT WIN32)

    # upnplib compatible shared library
    #----------------------------------
    add_library(upnplib_compa_shared SHARED   # prefix -ucsh
            ./compa/src/global.cpp
            ./compa/src/api/upnptools.cpp
            ./src/net/miniserver.cpp
            ./src/init.cpp
    )
    set_target_properties(upnplib_compa_shared PROPERTIES
            OUTPUT_NAME upnplib_compa
    )
    target_include_directories(upnplib_compa_shared
        PRIVATE
            ${UPnPlib_SOURCE_DIR}
            ${UPnPlib_SOURCE_DIR}/include
            ${PUPNP_UPNP_SOURCE_DIR}/inc
            ${PUPNP_UPNP_SOURCE_DIR}/src/inc
            ${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32
            ${PUPNP_IXML_SOURCE_DIR}/inc
    )
    target_compile_definitions(upnplib_compa_shared
        PRIVATE
            UPNPLIB_SHARED
            UPNPLIB_EXPORTS
    )
    # If we do not have compatible code we use the old native pupnp code
    target_link_libraries(upnplib_compa_shared
        PUBLIC
            pupnp_native_shared
    )

    # upnplib compatible static library
    #----------------------------------
    add_library(upnplib_compa_static STATIC   # prefix -ucst
            ./compa/src/global.cpp
            ./compa/src/api/upnptools.cpp
            ./src/net/miniserver.cpp
            ./src/init.cpp
    )
    set_target_properties(upnplib_compa_static PROPERTIES
            OUTPUT_NAME upnplib_compa
    )
    target_include_directories(upnplib_compa_static
        PRIVATE
            ${UPnPlib_SOURCE_DIR}
            ${UPnPlib_SOURCE_DIR}/include
            ${PUPNP_UPNP_SOURCE_DIR}/inc
            ${PUPNP_UPNP_SOURCE_DIR}/src/inc
            ${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32
            ${PUPNP_IXML_SOURCE_DIR}/inc
    )
    # If we do not have compatible code we use the old native pupnp code
    target_link_libraries(upnplib_compa_static
        # Static always link needed object files, no matter to use PRIVATE or PUBLIC.
        # For propagation only INTERFACE is needed.
        INTERFACE
            pupnp_native_static
    )

    # upnplib native shared library
    #------------------------------
    add_library(upnplib_native_shared SHARED   # prefix -unsh
            ./src/global.cpp
            ./src/api/upnptools.cpp
            ./src/net/sock.cpp
    )
    set_target_properties(upnplib_native_shared PROPERTIES
            OUTPUT_NAME upnplib_native
    )
    target_include_directories(upnplib_native_shared
        PRIVATE
            ${UPnPlib_SOURCE_DIR}/include
            ${PUPNP_UPNP_SOURCE_DIR}/inc
            ${PUPNP_IXML_SOURCE_DIR}/inc
    )
    target_compile_definitions(upnplib_native_shared
        PRIVATE
            UPNPLIB_SHARED
            UPNPLIB_EXPORTS
    )
    target_link_libraries(upnplib_native_shared
        INTERFACE
            ixml_shared
            # On Linux we cannot link with the static pthreads library because then
            # we have also link with a static libc that's not available by default.
            ${UPNP_PTHREADS_SHARED_LIBRARY}
    )

    # upnplib native static library
    #------------------------------
    add_library(upnplib_native_static STATIC   # prefix -unst
            ./src/global.cpp
            ./src/api/upnptools.cpp
            ./src/net/sock.cpp
    )
    set_target_properties(upnplib_native_static PROPERTIES
            OUTPUT_NAME upnplib_native
    )
    target_include_directories(upnplib_native_static
        PRIVATE
            ${UPnPlib_SOURCE_DIR}/include
            ${PUPNP_UPNP_SOURCE_DIR}/inc
            ${PUPNP_IXML_SOURCE_DIR}/inc
    )
    target_link_libraries(upnplib_native_static
        # Static always link needed object files, no matter to use PRIVATE or PUBLIC.
        # For propagation only INTERFACE is needed.
        INTERFACE
            ixml_static
            # On Linux we cannot link with the static pthreads library because then
            # we have also link with a static libc that's not available by default.
            ${UPNP_PTHREADS_SHARED_LIBRARY}
    )

    # Info executable linked with the upnplib compatible shared library
    #------------------------------------------------------------------
    add_executable(upnplibInfo_shared
            ./src/upnplibInfo.cpp
    )
    target_include_directories(upnplibInfo_shared
        PRIVATE
            ${UPnPlib_SOURCE_DIR}/include
            ${UPnPlib_BINARY_DIR}/include
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
            upnplib_compa_shared
            ${UPNP_PTHREADS_SHARED_LIBRARY}
    )

    # Info executable linked with the pupnp static library
    #-----------------------------------------------------
    add_executable(upnplibInfo_static
            ./src/upnplibInfo.cpp
    )
    target_include_directories(upnplibInfo_static
        PRIVATE
            ${UPnPlib_SOURCE_DIR}/include
            ${UPnPlib_BINARY_DIR}/include
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
            pupnp_native_static
            # On Linux we cannot link with the static pthreads library
            # because then we have also link with a static libc that's not
            # available by default.
            ${UPNP_PTHREADS_SHARED_LIBRARY}
    )

else() # WIN32

    # WIN32 upnplib compatible shared library
    #----------------------------------------
    add_library(upnplib_compa_shared SHARED   # prefix -ucsh
            ./compa/src/global.cpp
            ./compa/src/api/upnptools.cpp
            ./src/init.cpp
    )
    target_include_directories(upnplib_compa_shared
        PRIVATE
            ${UPnPlib_SOURCE_DIR}
            ${UPnPlib_SOURCE_DIR}/include
            ${PUPNP_UPNP_SOURCE_DIR}/inc
            ${PUPNP_UPNP_SOURCE_DIR}/src/inc
            ${PUPNP_IXML_SOURCE_DIR}/inc
            # if pthreads4w isn't installed this path is empty. The linker
            # doesn't find "pthread.h" and falls back to look at <pthread.h>.
            # (Only for general use. It doesn't matter here because we are
            # already on WIN32).
        INTERFACE
            ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
    )
    target_compile_definitions(upnplib_compa_shared
        PRIVATE
            UPNPLIB_SHARED
            UPNPLIB_EXPORTS
            $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    # If we do not have compatible code we use the old native pupnp code
    target_link_libraries(upnplib_compa_shared
        PUBLIC
            pupnp_native_shared
    )

    # WIN32 upnplib compatible static library
    #----------------------------------------
    add_library(upnplib_compa_static STATIC   # prefix -ucst
            ./compa/src/global.cpp
            ./compa/src/api/upnptools.cpp
            ./src/init.cpp
    )
    target_include_directories(upnplib_compa_static
        PRIVATE
            ${UPnPlib_SOURCE_DIR}
            ${UPnPlib_SOURCE_DIR}/include
            ${PUPNP_UPNP_SOURCE_DIR}/inc
            ${PUPNP_UPNP_SOURCE_DIR}/src/inc
            ${PUPNP_IXML_SOURCE_DIR}/inc
    )
    target_compile_definitions(upnplib_compa_static
        PRIVATE
            $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    # If we do not have compatible code we use the old native pupnp code
    target_link_libraries(upnplib_compa_static
        # Static always link needed object files, no matter to use PRIVATE or PUBLIC.
        # For propagation only INTERFACE is needed.
        INTERFACE
            pupnp_native_static
    )

    # WIN32 upnplib native shared library
    #------------------------------------
    add_library(upnplib_native_shared SHARED   # prefix -unsh
            ./src/global.cpp
            ./src/api/upnptools.cpp
            ./src/net/sock.cpp
    )
    target_include_directories(upnplib_native_shared
        PRIVATE
            ${UPnPlib_SOURCE_DIR}/include
            ${PUPNP_UPNP_SOURCE_DIR}/inc
            ${PUPNP_IXML_SOURCE_DIR}/inc
    )
    target_compile_definitions(upnplib_native_shared
        PRIVATE
            UPNPLIB_SHARED
            UPNPLIB_EXPORTS
            $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    # In addition to pthreads we need some more system libraries here
    target_link_libraries(upnplib_native_shared
        PUBLIC
            ws2_32   # winsock to support sockets
            iphlpapi # ip helper interface to get network management tools
            # On MS Windows we link with the static pthreads4w library to
            # avoid error prone managing access to its .dll file.
            ${UPNP_PTHREADS_STATIC_LIBRARY}
    )

    # WIN32 upnplib native static library
    #------------------------------------
    add_library(upnplib_native_static STATIC   # prefix unst
            ./src/global.cpp
            ./src/api/upnptools.cpp
            ./src/net/sock.cpp
    )
    target_include_directories(upnplib_native_static
        PRIVATE
            ${UPnPlib_SOURCE_DIR}/include
            ${PUPNP_UPNP_SOURCE_DIR}/inc
            ${PUPNP_IXML_SOURCE_DIR}/inc
    )
    target_compile_definitions(upnplib_native_static
        PRIVATE
            $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    target_link_libraries(upnplib_native_static
        # Static always link needed object files, no matter to use PRIVATE or PUBLIC.
        # For propagation only INTERFACE is needed.
        PUBLIC
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
            ${UPnPlib_SOURCE_DIR}/include
            ${UPnPlib_BINARY_DIR}/include
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
            upnplib_compa_shared
    )

    # WIN32 Info executable linked with the pupnp static library
    #-----------------------------------------------------------
    add_executable(upnplibInfo_static
            ./src/upnplibInfo.cpp
    )
    target_include_directories(upnplibInfo_static
        PRIVATE
            ${UPnPlib_SOURCE_DIR}/include
            ${UPnPlib_BINARY_DIR}/include
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
            pupnp_native_static
    )

endif() # WIN32
