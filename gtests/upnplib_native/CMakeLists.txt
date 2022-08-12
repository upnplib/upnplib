# Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-06-25

cmake_minimum_required(VERSION 3.18)
include(../../cmake/project-header.cmake)

project(UPNPLIB_NATIVE_GTEST5 VERSION 0001
      DESCRIPTION "Tests for native upnplib programs"
      HOMEPAGE_URL "https://github.com/upnplib")


# sock
# ====
add_executable(test_sock-unsh
        ./test_sock.cpp
)
target_include_directories(test_sock-unsh
    PRIVATE
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
        ${UPnPlib_SOURCE_DIR}/include
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
