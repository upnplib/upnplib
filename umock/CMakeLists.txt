# Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2023-09-14

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(UMOCK VERSION 0004
        DESCRIPTION "UPnP library mocking subproject"
        HOMEPAGE_URL "https://github.com/upnplib")


add_compile_definitions(
    # Disable warnings to use e.g. strncpy_s instead of strncpy
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)

set(UMOCK_SOURCE_FILES
    ./src/stringh.cpp
    ./src/stdlib.cpp
    ./src/sys_select.cpp
    ./src/sys_socket.cpp
    ./src/netdb.cpp
    ./src/pthread.cpp
    ./src/stdio.cpp
    ./src/unistd.cpp
    ./src/sysinfo.cpp
    ./src/arpa_inet.cpp
    ${UPNPLIB_PROJECT_SOURCE_DIR}/upnplib/src/port.cpp
    $<$<NOT:$<PLATFORM_ID:Windows>>:./src/ifaddrs.cpp>
    $<$<NOT:$<PLATFORM_ID:Windows>>:./src/net_if.cpp>
    $<$<PLATFORM_ID:Windows>:./src/iphlpapi.cpp>
    $<$<PLATFORM_ID:Windows>:./src/winsock2.cpp>
)

set(UMOCK_INCLUDE_DIRECTORIES
    PUBLIC ./include
    # We need this variable to find the path before subdir upnplib is scanned.
    PUBLIC ${UPNPLIB_PROJECT_SOURCE_DIR}/upnplib/include
    PRIVATE ${UPNPLIB_PROJECT_BINARY_DIR}}/include
    # if pthreads4w isn't installed this path is empty. The linker doesn't
    # find "pthread.h" and falls back to look at <pthread.h>.
    PUBLIC ${pthreads4w_SOURCE_DIR} # needed for pthread.h
)


# uMock SHared library: suffix -mst
#----------------------------------
add_library(umock_shared SHARED
    ${UMOCK_SOURCE_FILES}
)
target_include_directories(umock_shared
    ${UMOCK_INCLUDE_DIRECTORIES}
)
target_compile_definitions(umock_shared
    PUBLIC
        UPNPLIB_SHARED
    PRIVATE
        UPNPLIB_EXPORTS
)
target_link_libraries(umock_shared
    PUBLIC
        # In addition to pthreads we need some more system libraries on WIN32
        $<$<PLATFORM_ID:Windows>:ws2_32>   # winsock to support sockets
        $<$<PLATFORM_ID:Windows>:iphlpapi> # ip helper interface to get network
                                           # management tools
        # On MS Windows we link with the static pthreads4w library to avoid
        # error prone managing access to its .dll file.
        # On Unix platforms we cannot link with the static pthreads library
        # because then we have also to link with a static libc that's not
        # available by default.
        $<IF:$<PLATFORM_ID:Windows>,${UPNP_PTHREADS_STATIC_LIBRARY},${UPNP_PTHREADS_SHARED_LIBRARY}>
)

# UMock STatic library: suffix -mst
#----------------------------------
add_library(umock_static STATIC
    ${UMOCK_SOURCE_FILES}
)
target_include_directories(umock_static
    ${UMOCK_INCLUDE_DIRECTORIES}
)
target_link_libraries(umock_static
    PUBLIC
        # In addition to pthreads we need some more system libraries on WIN32
        $<$<PLATFORM_ID:Windows>:ws2_32>   # winsock to support sockets
        $<$<PLATFORM_ID:Windows>:iphlpapi> # ip helper interface to get network
                                           # management tools
    INTERFACE
        # On MS Windows we link with the static pthreads4w library to avoid
        # error prone managing access to its .dll file.
        # On Unix platforms we cannot link with the static pthreads library
        # because then we have also to link with a static libc that's not
        # available by default.
        $<IF:$<PLATFORM_ID:Windows>,${UPNP_PTHREADS_STATIC_LIBRARY},${UPNP_PTHREADS_SHARED_LIBRARY}>
)
