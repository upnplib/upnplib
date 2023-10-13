# Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2023-10-14

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
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts. We must use static libraries.
add_executable(test_miniserver_run-pst
#-------------------------------------
    ./test_miniserver_run.cpp
)
target_include_directories(test_miniserver_run-pst
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_link_libraries(test_miniserver_run-pst
    PRIVATE
        pupnp_static
        upnplib_static
        utest_static
)
add_test(NAME ctest_miniserver_run-pst COMMAND test_miniserver_run-pst --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

add_executable(test_miniserver_run-cst
#-------------------------------------
    ./test_miniserver_run.cpp
)
target_include_directories(test_miniserver_run-cst
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_link_libraries(test_miniserver_run-cst
    PRIVATE
        compa_static
        upnplib_static
        utest_static
)
add_test(NAME ctest_miniserver_run-cst COMMAND test_miniserver_run-cst --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)
