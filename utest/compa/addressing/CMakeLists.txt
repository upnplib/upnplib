# Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2023-11-14

cmake_minimum_required(VERSION 3.18)
include(../../../cmake/project-header.cmake)

project(UTEST_ADDRESSING VERSION 0010
                  DESCRIPTION "Tests for UPnP Addressing pupnp and compatible code"
                  HOMEPAGE_URL "https://github.com/upnplib")


# miniserver
#===========
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts. We must use static libraries.
#
# Direct linking with source files can be last found in
# commit b9c8fd4ad5f721a893de2181f5638b72b3014f0e.

add_executable(test_miniserver-pst
#---------------------------------
    ./test_miniserver.cpp
)
target_include_directories(test_miniserver-pst
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_link_libraries(test_miniserver-pst
    PRIVATE
        pupnp_static
        upnplib_static
        utest_static
)
add_test(NAME ctest_miniserver-pst COMMAND test_miniserver-pst --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

add_executable(test_miniserver-cst
#---------------------------------
    ./test_miniserver.cpp
)
target_include_directories(test_miniserver-cst
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_link_libraries(test_miniserver-cst
    PRIVATE
        compa_static
        upnplib_static
        utest_static
)
add_test(NAME ctest_miniserver-cst COMMAND test_miniserver-cst --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# miniserver_run
#===============
# disable warning C4273: inconsistent dll linkage.
# This warning on Microsoft Windows (MSVC) is because the source code is
# included into the test code to be able to test static functions. The compiled
# function in the link library is exported and it is also decorated to export
# with the included code. That's the warning. Because I know what I'm doing, I
# suppress it. It's not productive code. --Ingo
add_executable(test_miniserver_run-psh
#-------------------------------------
    ./test_miniserver_run.cpp
)
target_include_directories(test_miniserver_run-psh
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_compile_options(test_miniserver_run-psh
    # disable warning C4273: inconsistent dll linkage.
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/wd4273>
)
target_link_libraries(test_miniserver_run-psh
    PRIVATE
        pupnp_shared
        upnplib_shared
        utest_shared
)
add_test(NAME ctest_miniserver_run-psh COMMAND test_miniserver_run-psh --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

add_executable(test_miniserver_run-csh
#-------------------------------------
    ./test_miniserver_run.cpp
)
target_include_directories(test_miniserver_run-csh
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_compile_options(test_miniserver_run-csh
    # disable warning C4273: inconsistent dll linkage.
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/wd4273>
)
target_link_libraries(test_miniserver_run-csh
    PRIVATE
        compa_shared
        upnplib_shared
        utest_shared
)
add_test(NAME ctest_miniserver_run-csh COMMAND test_miniserver_run-csh --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)
