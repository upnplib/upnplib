# Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2023-06-26

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

# set the project name and version
project(GTESTS VERSION 0033
               DESCRIPTION "Unit Tests using googletest"
               HOMEPAGE_URL "https://github.com/upnplib")


#################################
# Build the Unit Tests          #
#################################
# For old builds with direct build from source files look at git commit:
# git show 741a85fbc88:gtests/CMakeLists.txt

include_directories(${GTESTS_SOURCE_DIR}/include
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
    # Most compiler warnings enabled
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    # all compiler warnings as errors
    $<$<CXX_COMPILER_ID:MSVC>:/WX>
    # disable warning C4702: unreachable code
    $<$<CXX_COMPILER_ID:MSVC>:/wd4702>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wconversion>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wsign-conversion>
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
    PRIVATE ./include
    PUBLIC ${UPNPLIB_PROJECT_SOURCE_DIR}/include
)
target_compile_definitions(upnplib_gtest_tools_shared
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_EXPORTS
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)
target_compile_options(upnplib_gtest_tools_shared
    PRIVATE
        # disable warning C4251: 'testing::internal::RE::pattern_': class
        # 'std::basic_string<char,std::char_traits<char>,std::allocator<char>>'
        # needs to have dll-interface to be used by clients of class
        # 'testing::internal::RE'
        $<$<CXX_COMPILER_ID:MSVC>:/wd4251>
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
target_compile_definitions(upnplib_gtest_tools_static
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)
target_compile_options(upnplib_gtest_tools_static
    PRIVATE
        # disable warning C4251: 'testing::internal::RE::pattern_': class
        # 'std::basic_string<char,std::char_traits<char>,std::allocator<char>>'
        # needs to have dll-interface to be used by clients of class
        # 'testing::internal::RE'
        $<$<CXX_COMPILER_ID:MSVC>:/wd4251>
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
    PRIVATE ./include
    PRIVATE ${UPNPLIB_SOURCE_DIR}/include
)
target_compile_definitions(test_upnplib_gtest_tools
        PRIVATE
            UPNPLIB_SHARED
)
target_compile_options(test_upnplib_gtest_tools
    PRIVATE
        # disable warning C4100: 'result_listener': unreferenced formal parameter
        # This is from Googlemock MATCHER_P. Don't know how to fix it. Seems to
        # be a side effect of the macro definition. --Ingo
        $<$<CXX_COMPILER_ID:MSVC>:/wd4100>
        # disable warning C4251: 'testing::internal::RE::pattern_': class
        # 'std::basic_string<char,std::char_traits<char>,std::allocator<char>>'
        # needs to have dll-interface to be used by clients of class
        # 'testing::internal::RE'
        $<$<CXX_COMPILER_ID:MSVC>:/wd4251>
)
target_link_libraries(test_upnplib_gtest_tools
    PRIVATE
        upnplib_gtest_tools_shared
)
add_test(NAME ctest_upnplib_gtest_tools COMMAND test_upnplib_gtest_tools --gtest_shuffle
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
)
target_compile_definitions(test_upnplib_gtest_tools_platform
    PRIVATE
        UPNPLIB_SHARED
)
target_compile_options(test_upnplib_gtest_tools_platform
    PRIVATE
        # disable warning C4251: 'testing::internal::RE::pattern_': class
        # 'std::basic_string<char,std::char_traits<char>,std::allocator<char>>'
        # needs to have dll-interface to be used by clients of class
        # 'testing::internal::RE'
        $<$<CXX_COMPILER_ID:MSVC>:/wd4251>
)
target_link_libraries(test_upnplib_gtest_tools_platform
    PRIVATE
        upnplib_gtest_tools_shared
)
add_test(NAME ctest_upnplib_gtest_tools_platform COMMAND test_upnplib_gtest_tools_platform --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# simple test
#============
add_executable(test_simple-ush
#-----------------------------
        ./test_simple.cpp
)
target_compile_definitions(test_simple-ush
    PRIVATE
        UPNPLIB_SHARED
)
target_compile_options(test_simple-ush
    PRIVATE
        # disable warning C4251: 'testing::internal::RE::pattern_': class
        # 'std::basic_string<char,std::char_traits<char>,std::allocator<char>>'
        # needs to have dll-interface to be used by clients of class
        # 'testing::internal::RE'
        $<$<CXX_COMPILER_ID:MSVC>:/wd4251>
)
target_link_libraries(test_simple-ush
    PRIVATE
        upnplib_shared
)
add_test(NAME ctest_simple-ush COMMAND test_simple-ush --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

# Check also static build for this simple test
add_executable(test_simple-ust
        ./test_simple.cpp
)
target_compile_options(test_simple-ust
    PRIVATE
        # disable warning C4251: 'testing::internal::RE::pattern_': class
        # 'std::basic_string<char,std::char_traits<char>,std::allocator<char>>'
        # needs to have dll-interface to be used by clients of class
        # 'testing::internal::RE'
        $<$<CXX_COMPILER_ID:MSVC>:/wd4251>
)
target_link_libraries(test_simple-ust
    PRIVATE
        upnplib_static
)
add_test(NAME ctest_simple-ust COMMAND test_simple-ust --gtest_shuffle
)


# template
#=========
add_executable(test_template
        test_template.cpp
)
target_compile_definitions(test_template
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)
target_compile_options(test_template
    PRIVATE
        # disable warning C4251: 'testing::internal::RE::pattern_': class
        # 'std::basic_string<char,std::char_traits<char>,std::allocator<char>>'
        # needs to have dll-interface to be used by clients of class
        # 'testing::internal::RE'
        $<$<CXX_COMPILER_ID:MSVC>:/wd4251>
)
add_test(NAME ctest_template COMMAND test_template --gtest_shuffle
)


add_subdirectory(./compa)
add_subdirectory(./upnplib)
if(UPNPLIB_WITH_SAMPLES)
    add_subdirectory(./sample)
endif()


# set(ENV{GITHUB_ACTIONS} 1)
# if(NOT APPLE)
# if(NOT DEFINED ENV{GITHUB_ACTIONS})
# target_compile_definitions(test_httpparser PRIVATE
#                            $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>)

# set_tests_properties( ctest_tv_device PROPERTIES
#     ENVIRONMENT "PATH=$ENV{PATH};${UPNPLIB_PROJECT_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}" )
