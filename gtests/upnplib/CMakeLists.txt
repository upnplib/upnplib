# Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-11-05

cmake_minimum_required(VERSION 3.18)
include(../../cmake/project-header.cmake)

project(UPNPLIB_GTEST5 VERSION 0003
      DESCRIPTION "Tests for upnplib programs"
      HOMEPAGE_URL "https://github.com/upnplib")


# upnptools
#==========
set(TEST_UPNPTOOLS_INCLUDE_DIRECTORIES
    PRIVATE
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
)

add_executable(test_upnptools-ush
#--------------------------------
        ./test_upnptools.cpp
)
target_include_directories(test_upnptools-ush
        ${TEST_UPNPTOOLS_INCLUDE_DIRECTORIES}
)
target_compile_definitions(test_upnptools-ush
    PRIVATE
        UPNPLIB_SHARED
)
target_link_libraries(test_upnptools-ush
    PRIVATE
        upnplib_shared
)
add_test(NAME ctest_upnptools-ush COMMAND test_upnptools-ush
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

add_executable(test_upnptools-ust
#---------------------------------
        ./test_upnptools.cpp
)
target_include_directories(test_upnptools-ust
        ${TEST_UPNPTOOLS_INCLUDE_DIRECTORIES}
)
target_link_libraries(test_upnptools-ust
    PRIVATE
        upnplib_static
)
add_test(NAME ctest_upnptools-ust COMMAND test_upnptools-ust
)


# sock
# ====
add_executable(test_sock-ush
        ./test_sock.cpp
)
target_include_directories(test_sock-ush
    PRIVATE
        ${GTESTS_SOURCE_DIR}/include
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
)
target_compile_definitions(test_sock-ush
    PRIVATE
        UPNPLIB_SHARED
)
target_link_libraries(test_sock-ush
    PRIVATE
        upnplib_shared
)
add_test(NAME ctest_sock-ush COMMAND test_sock-ush
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# webserver
# =========
add_executable(test_webserver-ush
        ./test_webserver.cpp
)
target_include_directories(test_webserver-ush
    PRIVATE
        ${GTESTS_SOURCE_DIR}/include
        ${UPNPLIB_PROJECT_SOURCE_DIR}
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
)
target_compile_definitions(test_webserver-ush
    PRIVATE
        UPNPLIB_SHARED
)
target_link_libraries(test_webserver-ush
    PRIVATE
        upnplib_shared
)
add_test(NAME ctest_webserver-ush COMMAND test_webserver-ush
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


add_subdirectory(./test_uri.d)
if(UPNPLIB_WITH_SAMPLES)
    add_subdirectory(./test_sample.d)
endif()
