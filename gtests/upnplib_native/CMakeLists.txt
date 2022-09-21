# Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-09-21

cmake_minimum_required(VERSION 3.18)
include(../../cmake/project-header.cmake)

project(UPNPLIB_NATIVE_GTEST5 VERSION 0002
      DESCRIPTION "Tests for native upnplib programs"
      HOMEPAGE_URL "https://github.com/upnplib")


# upnptools
#==========
set(TEST_UPNPTOOLS_INCLUDE_DIRECTORIES
    PRIVATE
        ./include
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_IXML_SOURCE_DIR}/inc
)

add_executable(test_upnptools-unsh
#---------------------------------
        ./test_upnptools.cpp
)
target_include_directories(test_upnptools-unsh
        ${TEST_UPNPTOOLS_INCLUDE_DIRECTORIES}
)
target_compile_definitions(test_upnptools-unsh
    PRIVATE
        UPNPLIB_SHARED
)
target_link_libraries(test_upnptools-unsh
    PRIVATE
        upnplib_native_shared # prefix -unsh
)
add_test(NAME ctest_upnptools-unsh COMMAND test_upnptools-unsh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

add_executable(test_upnptools-unst
#---------------------------------
        ./test_upnptools.cpp
)
target_include_directories(test_upnptools-unst
        ${TEST_UPNPTOOLS_INCLUDE_DIRECTORIES}
)
target_link_libraries(test_upnptools-unst
    PRIVATE
        upnplib_native_static # prefix -unst
)
add_test(NAME ctest_upnptools-unst COMMAND test_upnptools-unst
)


# sock
# ====
add_executable(test_sock-unsh
        ./test_sock.cpp
)
target_include_directories(test_sock-unsh
    PRIVATE
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
)
target_compile_definitions(test_sock-unsh
    PRIVATE
        UPNPLIB_SHARED
)
target_link_libraries(test_sock-unsh
    PRIVATE
        upnplib_native_shared
)
add_test(NAME ctest_sock-unsh COMMAND test_sock-unsh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)
