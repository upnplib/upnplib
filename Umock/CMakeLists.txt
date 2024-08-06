# Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2024-08-15

cmake_minimum_required(VERSION 3.23) # for FILE_SET
include(../cmake/project-header.cmake)

project(UMOCK VERSION 0004
        DESCRIPTION "UPnP library mocking subproject"
        HOMEPAGE_URL "https://github.com/upnplib")


add_compile_definitions(
    # Disable warnings to use e.g. strncpy_s instead of strncpy
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)

set(UMOCK_SOURCE_FILES
    src/stringh.cpp
    src/stdlib.cpp
    src/sys_socket.cpp
    src/netdb.cpp
    src/pthread.cpp
    src/stdio.cpp
    src/unistd.cpp
    src/sysinfo.cpp
    src/arpa_inet.cpp
    $<$<BOOL:${UPNPLIB_WITH_OPENSSL}>:src/ssl.cpp>
    ${CMAKE_SOURCE_DIR}/Upnplib/src/global.cpp
    $<$<NOT:$<PLATFORM_ID:Windows>>:src/ifaddrs.cpp>
    $<$<NOT:$<PLATFORM_ID:Windows>>:src/net_if.cpp>
    $<$<PLATFORM_ID:Windows>:src/iphlpapi.cpp>
    $<$<PLATFORM_ID:Windows>:src/winsock2.cpp>
)

set(UMOCK_PUBLIC_HEADERS
    include/umock/unistd.hpp
)

set(UMOCK_INCLUDE_DIRECTORIES
    PUBLIC ./include
    # We need this variable to find the path before subdir Upnplib is scanned.
    PUBLIC ${CMAKE_SOURCE_DIR}/Upnplib/include
    PUBLIC ${CMAKE_BINARY_DIR}/include
    # if pthreads4w isn't installed this path is empty. The linker doesn't
    # find "pthread.h" and falls back to look at <pthread.h>.
    PUBLIC ${pthreads4w_SOURCE_DIR} # needed for pthread.h
)


# uMock SHared library: suffix -mst
#----------------------------------
add_library(umock_shared SHARED
    ${UMOCK_SOURCE_FILES}
)
set_target_properties(umock_shared PROPERTIES
        OUTPUT_NAME upnpsdk-mock
)
target_include_directories(umock_shared
    ${UMOCK_INCLUDE_DIRECTORIES}
)
target_compile_definitions(umock_shared
    PUBLIC UPNPLIB_SHARED
    PRIVATE UPNPLIB_EXPORTS
)
target_link_libraries(umock_shared
    PUBLIC $<$<BOOL:${UPNPLIB_WITH_OPENSSL}>:upnplib_ssl>
    # In addition to pthreads we need some more system libraries on WIN32
    PUBLIC $<$<PLATFORM_ID:Windows>:ws2_32>   # winsock to support sockets
    PUBLIC $<$<PLATFORM_ID:Windows>:iphlpapi> # ip helper interface to get network
                                       # management tools
    # On MS Windows we link with the static pthreads4w library to avoid
    # error prone managing access to its .dll file.
    # On Unix platforms we cannot link with the static pthreads library
    # because then we have also to link with a static libc that's not
    # available by default.
    PUBLIC $<IF:$<PLATFORM_ID:Windows>,${UPNP_PTHREADS_STATIC_LIBRARY},${UPNP_PTHREADS_SHARED_LIBRARY}>
)
target_sources(umock_shared
    INTERFACE FILE_SET umock_public_headers TYPE HEADERS
              BASE_DIRS include
              FILES ${UMOCK_PUBLIC_HEADERS}
)
install(TARGETS umock_shared
    FILE_SET umock_public_headers
    DESTINATION include/upnp
)

# UMock STatic library: suffix -mst
#----------------------------------
add_library(umock_static STATIC
    ${UMOCK_SOURCE_FILES}
)
set_target_properties(umock_static PROPERTIES
        OUTPUT_NAME $<IF:$<PLATFORM_ID:Windows>,upnpsdk-mock-static,upnpsdk-mock>
)
target_include_directories(umock_static
    ${UMOCK_INCLUDE_DIRECTORIES}
)
target_link_libraries(umock_static
    PUBLIC $<$<BOOL:${UPNPLIB_WITH_OPENSSL}>:upnplib_ssl>
    # In addition to pthreads we need some more system libraries on WIN32
    PUBLIC $<$<PLATFORM_ID:Windows>:ws2_32>   # winsock to support sockets
    PUBLIC $<$<PLATFORM_ID:Windows>:iphlpapi> # ip helper interface to get network
                                       # management tools
    # On MS Windows we link with the static pthreads4w library to avoid
    # error prone managing access to its .dll file.
    # On Unix platforms we cannot link with the static pthreads library
    # because then we have also to link with a static libc that's not
    # available by default.
    INTERFACE $<IF:$<PLATFORM_ID:Windows>,${UPNP_PTHREADS_STATIC_LIBRARY},${UPNP_PTHREADS_SHARED_LIBRARY}>
)

install(TARGETS umock_shared umock_static)
