# Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2023-12-26

cmake_minimum_required(VERSION 3.18)
include(../../../cmake/project-header.cmake)

project(GTESTS_COMPA_SSDP VERSION 0001
                  DESCRIPTION "Tests for the compa ssdp module"
                  HOMEPAGE_URL "https://github.com/upnplib")


# ssdp_server
#============
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts. We must use static libraries.

add_executable(test_ssdp_server-pst
#----------------------------------
    ./test_ssdp_server.cpp
)
target_include_directories(test_ssdp_server-pst
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_compile_definitions(test_ssdp_server-pst
    PRIVATE
        UPNP_HAVE_CLIENT
        UPNP_HAVE_DEVICE
        UPNP_HAVE_SSDP
)
target_link_libraries(test_ssdp_server-pst
    PRIVATE
        pupnp_static
        upnplib_static
        utest_static
)
add_test(NAME ctest_ssdp_server-pst COMMAND test_ssdp_server-pst --gtest_shuffle
    WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

if(FALSE)
add_executable(test_ssdp_server-cst
#----------------------------------
    ./test_ssdp_server.cpp
)
target_compile_definitions(test_ssdp_server-cst
    PRIVATE
        UPNP_HAVE_CLIENT
        UPNP_HAVE_DEVICE
        UPNP_HAVE_SSDP
)
target_link_libraries(test_ssdp_server-cst
    PRIVATE
        compa_static
        upnplib_static
)
add_test(NAME ctest_ssdp_server-cst COMMAND test_ssdp_server-cst --gtest_shuffle
    WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)
endif()
