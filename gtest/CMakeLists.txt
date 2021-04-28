# Copyright (c) 2021 Ingo Höft, <Ingo@Hoeft-online.de>
# Last modified: 2021-04-24

cmake_minimum_required(VERSION 3.18)

# set the project name and version
project(Gtests VERSION 001
               DESCRIPTION "Unit Tests using googletest"
               HOMEPAGE_URL "https://github.com/upnplib")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    "${PROJECT_SOURCE_DIR}/inc"
    "${PROJECT_SOURCE_DIR}/../build/_deps/googletest-src/googletest/include"
    "${PROJECT_SOURCE_DIR}/../build/_deps/googletest-src/googlemock/include"
    "${PROJECT_SOURCE_DIR}/../build"
    "${PROJECT_SOURCE_DIR}/../upnp/inc"
    "${PROJECT_SOURCE_DIR}/../upnp/src"
    "${PROJECT_SOURCE_DIR}/../upnp/src/inc"
    "${PROJECT_SOURCE_DIR}/../upnp/src/threadutil"
    "${PROJECT_SOURCE_DIR}/../build/upnp/inc"
    "${PROJECT_SOURCE_DIR}/../ixml/inc"
)

link_libraries(
    "${PROJECT_SOURCE_DIR}/../build/lib/libgtest.a"
    "${PROJECT_SOURCE_DIR}/../build/lib/libgmock.a"
    "${PROJECT_SOURCE_DIR}/../build/upnp/libupnp.a"
    "${PROJECT_SOURCE_DIR}/../build/ixml/libixml.a"
    "pthread"
)

#add_executable(test_template.a
#    ${PROJECT_SOURCE_DIR}/test_template.cpp
#)
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
