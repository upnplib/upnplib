# Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2024-08-06

cmake_minimum_required(VERSION 3.18)
include(../../cmake/project-header.cmake)

project(GTEST5_UPNPLIB VERSION 0003
      DESCRIPTION "Tests for upnplib programs"
      HOMEPAGE_URL "https://github.com/upnplib")


# upnptools
#==========
add_executable(test_upnptools-ush
#--------------------------------
    test_upnptools.cpp
)
target_link_libraries(test_upnptools-ush
    PRIVATE upnplib_shared
)
add_test(NAME ctest_upnptools-ush COMMAND test_upnptools-ush --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

add_executable(test_upnptools-ust
#---------------------------------
    test_upnptools.cpp
)
target_link_libraries(test_upnptools-ust
    PRIVATE upnplib_static
)
add_test(NAME ctest_upnptools-ust COMMAND test_upnptools-ust --gtest_shuffle
    WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# netaddr
# =======
add_executable(test_netaddr-ust
    test_netaddr.cpp
)
target_link_libraries(test_netaddr-ust
    PRIVATE upnplib_static
    PRIVATE utest_static
)
add_test(NAME ctest_netaddr-ust COMMAND test_netaddr-ust --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# sockaddr
# ========
add_executable(test_sockaddr-ust
    test_sockaddr.cpp
)
target_include_directories(test_sockaddr-ust
    PRIVATE ${CMAKE_SOURCE_DIR}
)
target_link_libraries(test_sockaddr-ust
    PRIVATE upnplib_static
    PRIVATE utest_static
)
add_test(NAME ctest_sockaddr-ust COMMAND test_sockaddr-ust --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# CAddrinfo
# =========
add_executable(test_addrinfo-ust
    test_addrinfo.cpp
)
target_include_directories(test_addrinfo-ust
    PRIVATE ${CMAKE_SOURCE_DIR}
)
target_link_libraries(test_addrinfo-ust
    PRIVATE upnplib_static
    PRIVATE utest_static
)
add_test(NAME ctest_addrinfo-ust COMMAND test_addrinfo-ust --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# CSocket
# =======
add_executable(test_socket-ush
    test_socket.cpp
)
target_link_libraries(test_socket-ush
    PRIVATE upnplib_shared
    PRIVATE utest_shared
)
add_test(NAME ctest_socket-ush COMMAND test_socket-ush --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# webserver
# =========
add_executable(test_webserver-ush
    test_webserver.cpp
)
target_include_directories(test_webserver-ush
    PRIVATE ${CMAKE_SOURCE_DIR}
)
target_link_libraries(test_webserver-ush
    PRIVATE upnplib_shared
)
add_test(NAME ctest_webserver-ush COMMAND test_webserver-ush --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# general
# =======
add_executable(test_general-ush
    test_general.cpp
)
target_link_libraries(test_general-ush
    PRIVATE upnplib_shared
    PRIVATE utest_shared
)
add_test(NAME ctest_general-ush COMMAND test_general-ush --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


add_subdirectory(./test_uri.d)
