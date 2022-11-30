# Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-12-01

cmake_minimum_required(VERSION 3.18)
include(../../../cmake/project-header.cmake)

project(GTESTS_COMPA_UTIL VERSION 0001
                  DESCRIPTION "Tests for utilities and helper"
                  HOMEPAGE_URL "https://github.com/upnplib")


# list
#=====
add_executable(test_list-psh
        ./test_list.cpp
)
target_include_directories(test_list-psh
    PRIVATE
        ${GTESTS_SOURCE_DIR}/include
)
target_compile_definitions(test_list-psh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
)
target_link_libraries(test_list-psh
    PRIVATE
        pupnp_shared
)
add_test(NAME ctest_list-psh COMMAND test_list-psh
    WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# membuffer
#==========
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts. We must use static libraries.
add_executable(test_membuffer-pst
        ./test_membuffer.cpp
)
target_include_directories(test_membuffer-pst
    PRIVATE
        ${UPNPLIB_PROJECT_SOURCE_DIR}
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
        ${GTESTS_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_IXML_SOURCE_DIR}/inc
)
target_compile_definitions(test_membuffer-pst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
)
target_compile_options(test_membuffer-pst
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W1>
)
target_link_libraries(test_membuffer-pst
        PRIVATE
            umock_static
)
add_test(NAME ctest_membuffer-pst COMMAND test_membuffer-pst
)


# strintmap
#==========
add_executable(test_strintmap-psh
        ./test_strintmap.cpp
)
target_include_directories(test_strintmap-psh
    PRIVATE
        ${GTESTS_SOURCE_DIR}/include
)
target_compile_definitions(test_strintmap-psh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
)
target_link_libraries(test_strintmap-psh
    PRIVATE
        pupnp_shared
)
add_test(NAME ctest_strintmap-psh COMMAND test_strintmap-psh
    WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)
