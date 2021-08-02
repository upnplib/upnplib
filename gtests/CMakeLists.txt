# Copyright (C) 2021 GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Last modified: 2021-08-02

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

# set the project name and version
project(GTEST VERSION 0003
               DESCRIPTION "Unit Tests using googletest"
               HOMEPAGE_URL "https://github.com/upnplib")


set(UPNP_GOOGLETEST_SOURCE_DIR ${PROJECT_SOURCE_DIR}/googletest)
set(UPNP_GOOGLETEST_BINARY_DIR ${PROJECT_SOURCE_DIR}/googletest/build)

# Install GoogleTest libraries if not available
#----------------------------------------------
# Check if the GoogleTest libraries are available. If not then download and
# install them. The problem is that we need the libraries already build to get
# its paths in this configure stage. We do it with an isolated project
# (no add_subdirectory()). So we can configure and build them direct in two
# external processes and have them available immediately.
if(   NOT UPNP_GTEST_STATIC_LIBRARY OR NOT UPNP_GMOCK_STATIC_LIBRARY
   OR NOT UPNP_GTEST_MAIN_STATIC_LIBRARY OR NOT UPNP_GMOCK_MAIN_STATIC_LIBRARY)

    execute_process(COMMAND cmake -S . -B build
                    WORKING_DIRECTORY ${UPNP_GOOGLETEST_SOURCE_DIR}
                    RESULT_VARIABLE RETURN_CODE
                    ERROR_VARIABLE PTHREAD_ERROR_MESSAGE
                    COMMAND_ECHO STDOUT)
    if(NOT ${RETURN_CODE} EQUAL 0)
        message("${RETURN_CODE}: ${PTHREAD_ERROR_MESSAGE}")
        return()
    endif()

# TODO: manage build option Release or Debug
    execute_process(COMMAND cmake --build build
                    WORKING_DIRECTORY ${UPNP_GOOGLETEST_SOURCE_DIR}
                    RESULT_VARIABLE RETURN_CODE
                    ERROR_VARIABLE PTHREAD_ERROR_MESSAGE
                    COMMAND_ECHO STDOUT)
    if(NOT ${RETURN_CODE} EQUAL 0)
        message("${RETURN_CODE}: ${PTHREAD_ERROR_MESSAGE}")
        return()
    endif()

    find_library(UPNP_GTEST_STATIC_LIBRARY gtest gtestd
                 PATHS ${UPNP_GOOGLETEST_BINARY_DIR}/lib
                 PATH_SUFFIXES Debug Release MinSizeRel RelWithDebInfo
                 NO_DEFAULT_PATH)
    find_library(UPNP_GMOCK_STATIC_LIBRARY gmock gmockd
                 PATHS ${UPNP_GOOGLETEST_BINARY_DIR}/lib
                 PATH_SUFFIXES Debug Release MinSizeRel RelWithDebInfo
                 NO_DEFAULT_PATH)

    find_library(UPNP_GTEST_MAIN_STATIC_LIBRARY gtest_main gtest_maind
                 PATHS ${UPNP_GOOGLETEST_BINARY_DIR}/lib
                 PATH_SUFFIXES Debug Release MinSizeRel RelWithDebInfo
                 NO_DEFAULT_PATH)
    find_library(UPNP_GMOCK_MAIN_STATIC_LIBRARY gmock_main gmock_maind
                 PATHS ${UPNP_GOOGLETEST_BINARY_DIR}/lib
                 PATH_SUFFIXES Debug Release MinSizeRel RelWithDebInfo
                 NO_DEFAULT_PATH)
endif()


# Create the Unit Tests
#----------------------
include_directories(
    ${PROJECT_SOURCE_DIR}/inc
    ${UPNP_GOOGLETEST_BINARY_DIR}/_deps/googletest-src/googletest/include
    ${UPNP_GOOGLETEST_BINARY_DIR}/_deps/googletest-src/googlemock/include
    ${CMAKE_SOURCE_DIR}/upnp/inc
    ${CMAKE_SOURCE_DIR}/upnp/src
    ${CMAKE_SOURCE_DIR}/upnp/src/inc
    ${CMAKE_SOURCE_DIR}/upnp/src/threadutil
    ${CMAKE_BINARY_DIR}/upnp/inc
    ${CMAKE_SOURCE_DIR}/ixml/inc
)

link_libraries(${UPNP_GTEST_STATIC_LIBRARY}
               ${UPNP_GMOCK_STATIC_LIBRARY}
               ${UPNP_GTEST_MAIN_STATIC_LIBRARY}
               ${UPNP_GMOCK_MAIN_STATIC_LIBRARY}
)

add_executable(test_template
    ${PROJECT_SOURCE_DIR}/test_template.cpp)
