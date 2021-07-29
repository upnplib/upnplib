# Copyright (C) 2021 GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Last modified: 2021-07-29

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

# set the project name and version
project(GTEST VERSION 0002
               DESCRIPTION "Unit Tests using googletest"
               HOMEPAGE_URL "https://github.com/upnplib")

include_directories(
    ${PROJECT_SOURCE_DIR}/inc
    ${CMAKE_BINARY_DIR}/_deps/googletest-src/googletest/include
    ${CMAKE_BINARY_DIR}/_deps/googletest-src/googlemock/include
    ${CMAKE_SOURCE_DIR}/upnp/inc
    ${CMAKE_SOURCE_DIR}/upnp/src
    ${CMAKE_SOURCE_DIR}/upnp/src/inc
    ${CMAKE_SOURCE_DIR}/upnp/src/threadutil
    ${CMAKE_BINARY_DIR}/upnp/inc
    ${CMAKE_SOURCE_DIR}/ixml/inc
)

link_libraries(
    ${CMAKE_BINARY_DIR}/lib/libgtest.a
    ${CMAKE_BINARY_DIR}/lib/libgmock.a
    ${CMAKE_SOURCE_DIR}/upnp/build/static/libupnplib.a
    pthread
)

add_executable(test_template.a
    ${PROJECT_SOURCE_DIR}/test_template.cpp
)
add_executable(test_upnpapi.a
    ${PROJECT_SOURCE_DIR}/tools/tools.cpp
    ${PROJECT_SOURCE_DIR}/test_upnpapi.cpp
)
add_executable(test_upnpdebug.a
    ${PROJECT_SOURCE_DIR}/tools/tools.cpp
    ${PROJECT_SOURCE_DIR}/test_upnpdebug.cpp
)
add_executable(test_upnpdebug_nomock.a
    ${PROJECT_SOURCE_DIR}/test_upnpdebug_nomock.cpp
)
add_executable(test_threadutil.a
    ${PROJECT_SOURCE_DIR}/test_threadutil.cpp
)
