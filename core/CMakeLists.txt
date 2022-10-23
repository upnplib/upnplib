# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-10-23

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(UPNPLIB_CORE VERSION 0009
                     DESCRIPTION "UPnP library core program"
                     HOMEPAGE_URL "https://github.com/upnplib")


set(UPNPLIB_COMPATIBLE_SOURCE_FILES
# There are additional source files at different add_library() statements.
    ./src/init.cpp
    ./compa/src/api/UpnpString.cpp
)

set(UPNPLIB_NATIVE_SOURCE_FILES
    ./src/global.cpp
    ./src/api/upnptools.cpp
    ./src/net/sock.cpp
    ./src/net/http/webserver.cpp

    #${UPnPlib_SOURCE_DIR}/umock/src/string.cpp
    #${UPnPlib_SOURCE_DIR}/umock/src/stdlib.cpp
    #${UPnPlib_SOURCE_DIR}/umock/src/sys_socket.cpp
    #${UPnPlib_SOURCE_DIR}/umock/src/sys_select.cpp
    #${UPnPlib_SOURCE_DIR}/umock/src/netdb.cpp
    #${UPnPlib_SOURCE_DIR}/umock/src/pthread.cpp
    #${UPnPlib_SOURCE_DIR}/umock/src/stdio.cpp
    #${UPnPlib_SOURCE_DIR}/umock/src/unistd.cpp
    $<$<NOT:$<PLATFORM_ID:Windows>>:./src/mocking/ifaddrs.cpp>
    $<$<NOT:$<PLATFORM_ID:Windows>>:./src/mocking/net_if.cpp>
    $<$<PLATFORM_ID:Windows>:./src/mocking/iphlpapi.cpp>
    $<$<PLATFORM_ID:Windows>:./src/mocking/winsock2.cpp>
)


set(UPNPLIB_COMPATIBLE_INCLUDE_DIRECTORIES
    PRIVATE
        ${UPnPlib_SOURCE_DIR}/include
        ${UPNPLIB_CORE_SOURCE_DIR}/compa/inc
)

set(UPNPLIB_NATIVE_INCLUDE_DIRECTORIES
    PRIVATE
        ${UPnPlib_SOURCE_DIR}/include
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        $<$<PLATFORM_ID:Windows>:${pthreads4w_SOURCE_DIR}> # needed for pthread.h
)

if(NOT WIN32)

    # upnplib compatible libraries
    #=============================

    # Upnplib Compatible SHared library: postfix -ucsh
    #-------------------------------------------------
    add_library(upnplib_compa_shared SHARED
        ${UPNPLIB_COMPATIBLE_SOURCE_FILES}
        ./src/strndup.cpp
    )
    set_target_properties(upnplib_compa_shared PROPERTIES
            OUTPUT_NAME upnplib_compa
    )
    target_include_directories(upnplib_compa_shared
            ${UPNPLIB_COMPATIBLE_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions(upnplib_compa_shared
        PRIVATE
            UPNPLIB_SHARED
            UPNPLIB_EXPORTS
            $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    target_link_libraries(upnplib_compa_shared
        PRIVATE
            $<$<PLATFORM_ID:Darwin>:umock_shared>
        INTERFACE
            # If we do not have compatible code we use the old native pupnp code.
            pupnp_native_shared
    )

    # Upnplib Compatible STatic library: postfix -ucst
    #-------------------------------------------------
    add_library(upnplib_compa_static STATIC
            ${UPNPLIB_COMPATIBLE_SOURCE_FILES}
    )
    set_target_properties(upnplib_compa_static PROPERTIES
            OUTPUT_NAME upnplib_compa
    )
    target_include_directories(upnplib_compa_static
            ${UPNPLIB_COMPATIBLE_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions(upnplib_compa_static
        PRIVATE
            $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    target_link_libraries(upnplib_compa_static
        INTERFACE
            # If we do not have compatible code we use the old native pupnp code.
            pupnp_native_static
    )

    # upnplib native libraries
    #=========================

    # Upnplib Native SHared library: postfix -unsh
    #---------------------------------------------
    add_library(upnplib_native_shared SHARED
            ${UPNPLIB_NATIVE_SOURCE_FILES}
            ./src/strndup.cpp
    )
    set_target_properties(upnplib_native_shared PROPERTIES
            OUTPUT_NAME upnplib_native
    )
    target_include_directories(upnplib_native_shared
            ${UPNPLIB_NATIVE_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions(upnplib_native_shared
        PRIVATE
            UPNPLIB_SHARED
            UPNPLIB_EXPORTS
            $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    target_link_libraries(upnplib_native_shared
        PRIVATE
            $<$<PLATFORM_ID:Darwin>:umock_shared>
        INTERFACE
            ixml_shared
            # On Linux we cannot link with the static pthreads library because then
            # we have also to link with a static libc that's not available by default.
            ${UPNP_PTHREADS_SHARED_LIBRARY}
    )

    # Upnplib Native STatic library: postfix -unst
    #---------------------------------------------
    add_library(upnplib_native_static STATIC
            ${UPNPLIB_NATIVE_SOURCE_FILES}
    )
    set_target_properties(upnplib_native_static PROPERTIES
            OUTPUT_NAME upnplib_native
    )
    target_include_directories(upnplib_native_static
            ${UPNPLIB_NATIVE_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions(upnplib_native_static
        PRIVATE
            $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    target_link_libraries(upnplib_native_static
        # Static always link needed object files, no matter to use PRIVATE or PUBLIC.
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
            umock_shared
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
            umock_static
            # On Linux we cannot link with the static pthreads library
            # because then we have also link with a static libc that's not
            # available by default.
            ${UPNP_PTHREADS_SHARED_LIBRARY}
    )

else() # WIN32

    # WIN32 upnplib compatible libraries
    #===================================

    # WIN32 Upnplib Compatible SHared library: postfix -ucsh
    #-------------------------------------------------------
    add_library(upnplib_compa_shared SHARED
        ${UPNPLIB_COMPATIBLE_SOURCE_FILES}
        ./src/strndup.cpp
    )
    target_include_directories(upnplib_compa_shared
            ${UPNPLIB_COMPATIBLE_INCLUDE_DIRECTORIES}
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
    target_link_libraries(upnplib_compa_shared
        PRIVATE
            umock_shared
        INTERFACE
            # If we do not have compatible code we use the old native pupnp code.
            pupnp_native_shared
    )

    # WIN32 Upnplib Compatible STatic library: postfix -ucst
    #-------------------------------------------------------
    add_library(upnplib_compa_static STATIC
        ${UPNPLIB_COMPATIBLE_SOURCE_FILES}
    )
    target_include_directories(upnplib_compa_static
            ${UPNPLIB_COMPATIBLE_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions(upnplib_compa_static
        PRIVATE
            $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    target_link_libraries(upnplib_compa_static
        # Static always link needed object files, no matter to use PRIVATE or PUBLIC.
        # For propagation only INTERFACE is needed.
        INTERFACE
            # If we do not have compatible code we use the old native pupnp code.
            pupnp_native_static
    )


    # WIN32 upnplib native libraries
    #===============================

    # WIN32 Upnplib Native SHared library: postfix -unsh
    #---------------------------------------------------
    add_library(upnplib_native_shared SHARED
            ${UPNPLIB_NATIVE_SOURCE_FILES}
            ./src/strndup.cpp
    )
    target_include_directories(upnplib_native_shared
            ${UPNPLIB_NATIVE_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions(upnplib_native_shared
        PRIVATE
            UPNPLIB_SHARED
            UPNPLIB_EXPORTS
            $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    target_link_libraries(upnplib_native_shared
        PRIVATE
            umock_shared
        PUBLIC
            # In addition to pthreads we need some more system libraries here
            ws2_32   # winsock to support sockets
            iphlpapi # ip helper interface to get network management tools
            # On MS Windows we link with the static pthreads4w library to
            # avoid error prone managing access to its .dll file.
            ${UPNP_PTHREADS_STATIC_LIBRARY}
    )

    # WIN32 Upnplib Native STatic library: postfix -unst
    #---------------------------------------------------
    add_library(upnplib_native_static STATIC
            ${UPNPLIB_NATIVE_SOURCE_FILES}
    )
    target_include_directories(upnplib_native_static
            ${UPNPLIB_NATIVE_INCLUDE_DIRECTORIES}
    )
    target_compile_definitions(upnplib_native_static
        PRIVATE
            $<$<BOOL:${UPNPLIB_DEF_IXML}>:IXML_HAVE_SCRIPTSUPPORT>
    )
    target_link_libraries(upnplib_native_static
        # Static always link needed object files, no matter to use PRIVATE or PUBLIC.
        # For propagation only INTERFACE is needed.
        INTERFACE
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
            umock_static
    )

endif() # WIN32
