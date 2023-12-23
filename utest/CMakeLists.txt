# Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2023-12-26

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

# set the project name and version
project(UTEST VERSION 0034
              DESCRIPTION "Unit Tests using googletest"
              HOMEPAGE_URL "https://github.com/upnplib")


#################################
# Build the Unit Tests          #
#################################
# For old builds with direct build from source files look at git commit:
# git show 741a85fbc88:gtests/CMakeLists.txt

include_directories(${UTEST_SOURCE_DIR}/include
)
# To link googletest we need of course its libraries.
# gmock libraries also contain gtest libraries.
link_libraries(gmock gmock_main
)
add_compile_definitions(
    # If linking with shared build gtest libs we need to tell it the compiler.
    # I have found a vague hint about the flag GTEST_LINKED_AS_SHARED_LIBRARY at
    # build/_deps/googletest-src/googletest/README.md
    $<$<AND:$<BOOL:${UPNPLIB_WITH_GOOGLETEST}>,$<BOOL:${BUILD_SHARED_LIBS}>>:GTEST_LINKED_AS_SHARED_LIBRARY>
    $<$<PLATFORM_ID:Windows>:_CRT_NONSTDC_NO_DEPRECATE>
)
add_compile_options(
    # disable warning C4702: unreachable code
    # This warning comes with many googletest macros EXPECT_*/ASSERT_* and is
    # false there. --Ingo
    $<$<CXX_COMPILER_ID:MSVC>:/wd4702>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wconversion>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wsign-conversion>
)


# Upnplib helper library for gtests
# =================================
# We have version
# utest for all operating systems,
# utest_unix for Unix operating systems and
# utest_win32 for Microsoft Windows

set(UTEST_SOURCE_FILES
    tools/utest.cpp
    $<$<PLATFORM_ID:Windows>:tools/utest_win32.cpp>
    $<$<NOT:$<PLATFORM_ID:Windows>>:tools/utest_unix.cpp>
    ${UMOCK_SOURCE_DIR}/src/arpa_inet_mock.cpp
    ${UMOCK_SOURCE_DIR}/src/stringh_mock.cpp
    ${UMOCK_SOURCE_DIR}/src/netdb_mock.cpp
    ${UMOCK_SOURCE_DIR}/src/pthread_mock.cpp
    ${UMOCK_SOURCE_DIR}/src/stdio_mock.cpp
    ${UMOCK_SOURCE_DIR}/src/stdlib_mock.cpp
    ${UMOCK_SOURCE_DIR}/src/sys_socket_mock.cpp
    ${UMOCK_SOURCE_DIR}/src/sysinfo_mock.cpp
    ${UMOCK_SOURCE_DIR}/src/unistd_mock.cpp
    $<$<PLATFORM_ID:Windows>:${UMOCK_SOURCE_DIR}/src/iphlpapi_mock.cpp>
    $<$<NOT:$<PLATFORM_ID:Windows>>:${UMOCK_SOURCE_DIR}/src/ifaddrs_mock.cpp>
    $<$<NOT:$<PLATFORM_ID:Windows>>:${UMOCK_SOURCE_DIR}/src/net_if_mock.cpp>
    $<$<BOOL:${UPNPLIB_WITH_OPENSSL}>:${UMOCK_SOURCE_DIR}/src/ssl_mock.cpp>
)

add_library(utest_shared SHARED
#------------------------------
    ${UTEST_SOURCE_FILES}
)
target_compile_definitions(utest_shared
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_EXPORTS
)
target_link_libraries(utest_shared
    PUBLIC $<$<PLATFORM_ID:Windows>:ws2_32>      # winsock to support sockets
    INTERFACE $<$<PLATFORM_ID:Windows>:iphlpapi> # ip helper interface to get network management tools
    PRIVATE umock_shared;
)

add_library(utest_static STATIC
#------------------------------
    ${UTEST_SOURCE_FILES}
)
target_link_libraries(utest_static
    PUBLIC $<$<PLATFORM_ID:Windows>:ws2_32>      # winsock to support sockets
    INTERFACE $<$<PLATFORM_ID:Windows>:iphlpapi> # ip helper interface to get network management tools
    PRIVATE umock_static;
)


#link_libraries(utest_shared utest_static
#)

#
# Reset output directories to default means to use the subprojects directories.
# This should be done after build of utest.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "")


#################################
# Unit Tests                    #
#################################
# The tests are build in reverse order. test_template.cpp will be build first.

# gtests for utest helper
#========================
add_executable(test_utest-ush
#----------------------------
    tools/test_utest.cpp
)
target_include_directories(test_utest-ush
    PRIVATE ${UPNPLIB_SOURCE_DIR}/include
)
target_compile_definitions(test_utest-ush
    PRIVATE UPNPLIB_SHARED
)
target_link_libraries(test_utest-ush
    PRIVATE utest_shared
)
add_test(NAME ctest_utest-ush COMMAND test_utest-ush --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


add_executable(test_utest_platform-ush
#-----------------------------------------------
    $<$<PLATFORM_ID:Windows>:./tools/test_utest_win32.cpp>
    $<$<NOT:$<PLATFORM_ID:Windows>>:./tools/test_utest_unix.cpp>
)
target_include_directories(test_utest_platform-ush
    PRIVATE ${UPNPLIB_SOURCE_DIR}/include
)
target_compile_definitions(test_utest_platform-ush
    PRIVATE UPNPLIB_SHARED
)
target_link_libraries(test_utest_platform-ush
    PRIVATE utest_shared
)
add_test(NAME ctest_utest_platform-ush COMMAND test_utest_platform-ush --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# simple test
#============
add_executable(test_simple-ush
#-----------------------------
        test_simple.cpp
)
target_link_libraries(test_simple-ush
    PRIVATE upnplib_shared
)
add_test(NAME ctest_simple-ush COMMAND test_simple-ush --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

# Check also static build for this simple test
add_executable(test_simple-ust
        test_simple.cpp
)
target_link_libraries(test_simple-ust
    PRIVATE upnplib_static
)
add_test(NAME ctest_simple-ust COMMAND test_simple-ust --gtest_shuffle
    WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# template
#=========
add_executable(test_template
        test_template.cpp
)
target_include_directories(test_template
    PRIVATE ${UPNPLIB_SOURCE_DIR}/include
)
add_test(NAME ctest_template COMMAND test_template --gtest_shuffle
    WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


add_subdirectory(./compa)
add_subdirectory(./upnplib)
if(NOT DEFINED ENV{GITHUB_ACTIONS})
    add_subdirectory(./sample)
endif()


# set(ENV{GITHUB_ACTIONS} 1)
# if(NOT APPLE)
# if(NOT DEFINED ENV{GITHUB_ACTIONS})
# target_compile_definitions(test_httpparser PRIVATE
#                            $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>)

# set_tests_properties( ctest_tv_device PROPERTIES
#     ENVIRONMENT "PATH=$ENV{PATH};${UPNPLIB_PROJECT_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}" )
