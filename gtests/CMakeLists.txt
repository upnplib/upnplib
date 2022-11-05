# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-11-05

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

# set the project name and version
project(UPNPLIB_GTESTS VERSION 0033
                       DESCRIPTION "Unit Tests using googletest"
                       HOMEPAGE_URL "https://github.com/upnplib")


#################################
# Build the Unit Tests          #
#################################
# For old builds with direct build from source files look at git commit:
# git show 741a85fbc88:gtests/CMakeLists.txt

# To link googletest we need of course its libraries.
# gmock libraries also contain gtest libraries.
link_libraries(gmock gmock_main
)
# If linking with shared build gtest libs we need to tell it the compiler.
# I have found a vague hint about the flag GTEST_LINKED_AS_SHARED_LIBRARY at
# build/_deps/googletest-src/googletest/README.md
add_compile_definitions(
    PRIVATE
        $<$<AND:$<BOOL:${UPNPLIB_WITH_GOOGLETEST}>,$<BOOL:${BUILD_SHARED_LIBS}>>:GTEST_LINKED_AS_SHARED_LIBRARY>
)

# Create upnplib helper tools library for gtests. We have version
# upnplib_gtest_tools for all operating systems,
# upnplib_gtest_tools_unix for Unix operating systems and
# upnplib_gtest_tools_win32 for Microsoft Windows
add_library(upnplib_gtest_tools_shared SHARED
        ./tools/upnplib_gtest_tools.cpp
        $<$<PLATFORM_ID:Windows>:./tools/upnplib_gtest_tools_win32.cpp>
        $<$<NOT:$<PLATFORM_ID:Windows>>:./tools/upnplib_gtest_tools_unix.cpp>
)
target_include_directories(upnplib_gtest_tools_shared
    PRIVATE
        ./include
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
)
target_compile_definitions(upnplib_gtest_tools_shared
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_EXPORTS
)
target_link_libraries(upnplib_gtest_tools_shared
    PUBLIC
        $<$<PLATFORM_ID:Windows>:ws2_32>   # winsock to support sockets
    INTERFACE
        $<$<PLATFORM_ID:Windows>:iphlpapi> # ip helper interface to get network management tools
)

add_library(upnplib_gtest_tools_static STATIC
        ./tools/upnplib_gtest_tools.cpp
        $<$<PLATFORM_ID:Windows>:./tools/upnplib_gtest_tools_win32.cpp>
        $<$<NOT:$<PLATFORM_ID:Windows>>:./tools/upnplib_gtest_tools_unix.cpp>
)
target_include_directories(upnplib_gtest_tools_static
    PRIVATE
        ./include
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
)
target_link_libraries(upnplib_gtest_tools_static
    PUBLIC
        $<$<PLATFORM_ID:Windows>:ws2_32>   # winsock to support sockets
    INTERFACE
        $<$<PLATFORM_ID:Windows>:iphlpapi> # ip helper interface to get network management tools
)

# Reset output directories to default means to use the subprojects directories.
# This should be done after build of upnplib_gtest_tools.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "")


#################################
# Unit Tests                    #
#################################
# The tests are build in reverse order. test_template.cpp will be build first.

# upnplib_gtest_tools
#====================
add_executable(test_upnplib_gtest_tools
#--------------------------------------
        ./tools/test_tools.cpp
)
target_include_directories(test_upnplib_gtest_tools
    PRIVATE
        ./include
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
)
target_compile_definitions(test_upnplib_gtest_tools
        PRIVATE
            UPNPLIB_SHARED
)
target_link_libraries(test_upnplib_gtest_tools
    PRIVATE
        upnplib_gtest_tools_shared
)
add_test(NAME ctest_upnplib_gtest_tools COMMAND test_upnplib_gtest_tools
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


add_executable(test_upnplib_gtest_tools_platform
#-----------------------------------------------
        $<$<PLATFORM_ID:Windows>:./tools/test_tools_win32.cpp>
        $<$<NOT:$<PLATFORM_ID:Windows>>:./tools/test_tools_unix.cpp>
)
target_include_directories(test_upnplib_gtest_tools_platform
    PRIVATE
        ./include
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
)
target_compile_definitions(test_upnplib_gtest_tools_platform
    PRIVATE
        UPNPLIB_SHARED
)
target_link_libraries(test_upnplib_gtest_tools_platform
    PRIVATE
        upnplib_gtest_tools_shared
)
add_test(NAME ctest_upnplib_gtest_tools_platform COMMAND test_upnplib_gtest_tools_platform
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# simple test
#============
set(TEST_SIMPLE_INCLUDE_DIRECTORIES
    PRIVATE
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h on MS Windows
)

add_executable(test_simple-ush
#-----------------------------
        ./test_simple.cpp
)
target_include_directories(test_simple-ush
    ${TEST_SIMPLE_INCLUDE_DIRECTORIES}
)
target_compile_definitions(test_simple-ush
    PRIVATE
        UPNPLIB_SHARED
)
target_link_libraries(test_simple-ush
    PRIVATE
        upnplib_shared
)
add_test(NAME ctest_simple-ush COMMAND test_simple-ush
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

# Check also static build for this simple test
add_executable(test_simple-ust
        ./test_simple.cpp
)
target_include_directories(test_simple-ust
    ${TEST_SIMPLE_INCLUDE_DIRECTORIES}
)
target_link_libraries(test_simple-ust
    PRIVATE
        upnplib_static
)
add_test(NAME ctest_simple-ust COMMAND test_simple-ust
)


# template
#=========
add_executable(test_template
        test_template.cpp)
add_test(NAME ctest_template COMMAND test_template)


add_subdirectory(./compa)
add_subdirectory(./upnplib)


# set(ENV{GITHUB_ACTIONS} 1)
# if(NOT APPLE)
# if(NOT DEFINED ENV{GITHUB_ACTIONS})
# target_compile_definitions(test_httpparser PRIVATE
#                            $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>)

# set_tests_properties( ctest_tv_device PROPERTIES
#     ENVIRONMENT "PATH=$ENV{PATH};${UPNPLIB_PROJECT_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}" )
