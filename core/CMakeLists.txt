# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-03-11

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(UPNPLIB_CORE VERSION 0002
                     DESCRIPTION "UPnP library core program"
                     HOMEPAGE_URL "https://github.com/upnplib")


include_directories(
    ${UPnPlib_BINARY_DIR}/upnp/inc
    ${PUPNP_UPNP_SOURCE_DIR}/inc
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
    ${PUPNP_IXML_SOURCE_DIR}/inc
    # if pthreads4w isn't installed this path is empty. The linker
    # doesn't find "pthread.h" and falls back to look at <pthread.h>.
    ${pthreads4w_SOURCE_DIR}
)

if(NOT WIN32)
    # Info executable linked with the shared library
    #-----------------------------------------------
    add_executable(upnplibInfo_shared
            ./src/upnplibInfo.cpp
    )
    target_compile_definitions (upnplibInfo_shared
            PRIVATE UPNPLIB_SHARED
    )
    target_link_libraries(upnplibInfo_shared
            PRIVATE upnplib_shared
            PRIVATE ${UPNP_PTHREADS_SHARED_LIBRARY}
    )

    # Info executable linked with static libraries
    #---------------------------------------------
    add_executable(upnplibInfo_static
            ./src/upnplibInfo.cpp
    )
    set_target_properties(upnplibInfo_static PROPERTIES
            OUTPUT_NAME upnplibInfo
    )
    target_link_libraries(upnplibInfo_static
            PRIVATE upnplib_static
            # On Linux we cannot link with the static pthreads library
            # because then we have also link with a static libc that's not
            # available by default.
            PRIVATE ${UPNP_PTHREADS_SHARED_LIBRARY}
    )

else() # WIN32

    # Info executable linked with the shared library
    #-----------------------------------------------
    add_executable(upnplibInfo_shared
            ${UPNPLIB_CORE_SOURCE_DIR}/src/upnplibInfo.cpp
    )
    target_compile_definitions (upnplibInfo_shared
            PRIVATE UPNPLIB_SHARED
    )
    target_link_libraries(upnplibInfo_shared
            PRIVATE upnplib_shared
    )

    # Info executable linked with static libraries
    #---------------------------------------------
    add_executable(upnplibInfo_static
            ${UPNPLIB_CORE_SOURCE_DIR}/src/upnplibInfo.cpp
    )
    set_target_properties(upnplibInfo_static PROPERTIES
            OUTPUT_NAME upnplibInfo
    )
    target_link_libraries(upnplibInfo_static
            PRIVATE ws2_32   # winsock to support sockets
            PRIVATE ixml_static
            PRIVATE upnplib_static
    )

endif() # WIN32
