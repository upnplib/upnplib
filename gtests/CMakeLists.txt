# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-10-25

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

# set the project name and version
project(UPNPLIB_GTESTS VERSION 0032
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
        # Compile tests for the old (pupnp) source code.
        OLD_TEST
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

# sock
#=====
add_executable(test_sock-psh
        ./test_sock.cpp
)
target_include_directories(test_sock-psh
    PRIVATE
        ./include
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_IXML_SOURCE_DIR}/inc
)
target_compile_definitions(test_sock-psh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_sock-psh
    PRIVATE
        pupnp_shared
        upnplib_native_shared
)
add_test(NAME ctest_sock-psh COMMAND test_sock-psh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# strintmap
#==========
add_executable(test_strintmap-psh
        ./test_strintmap.cpp
)
target_include_directories(test_strintmap-psh
    PRIVATE
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        $<$<NOT:$<PLATFORM_ID:Windows>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${PUPNP_IXML_SOURCE_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_strintmap-psh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_strintmap-psh
    PRIVATE
        pupnp_shared
)
add_test(NAME ctest_strintmap-psh COMMAND test_strintmap-psh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# list
#=====
add_executable(test_list
        ./test_list.cpp
)
target_include_directories(test_list
    PRIVATE
        ${UPNPLIB_PROJECT_SOURCE_DIR}
        ${PUPNP_UPNP_SOURCE_DIR}/inc
)
add_test(NAME ctest_list COMMAND test_list
)


# membuffer
#==========
add_executable(test_membuffer
        ./test_membuffer.cpp
)
target_include_directories(test_membuffer
    PRIVATE
        ${UPNPLIB_PROJECT_SOURCE_DIR}
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_IXML_SOURCE_DIR}/inc
)
target_compile_definitions(test_membuffer
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
add_test(NAME ctest_membuffer COMMAND test_membuffer
)


# upnpdebug
#==========
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts. We must use static libraries.
add_executable(test_upnpdebug-pst
        ./test_upnpdebug.cpp
)
target_include_directories(test_upnpdebug-pst
    PRIVATE
        ./include
        ${UPNPLIB_PROJECT_SOURCE_DIR}
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_IXML_SOURCE_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_upnpdebug-pst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_upnpdebug-pst
    PRIVATE
        pupnp_static
        upnplib_native_static
        upnplib_gtest_tools_static
)
add_test(NAME ctest_upnpdebug-pst COMMAND test_upnpdebug-pst
)

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

# upnptools
#==========
if(UPNPLIB_WITH_TOOLS)
add_executable(test_upnptools-psh
#--------------------------------
        ./test_upnptools.cpp
)
target_include_directories(test_upnptools-psh
    PRIVATE
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_IXML_SOURCE_DIR}/inc
)
target_compile_definitions(test_upnptools-psh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
)
target_link_libraries(test_upnptools-psh
    PRIVATE
        # pupnp native shared: prefix -psh
        pupnp_shared
        upnplib_native_shared
)
add_test(NAME ctest_upnptools-psh COMMAND test_upnptools-psh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)
endif()


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

add_executable(test_simple-unsh
#------------------------------
        ./test_simple.cpp
)
target_include_directories(test_simple-unsh
    ${TEST_SIMPLE_INCLUDE_DIRECTORIES}
)
target_compile_definitions(test_simple-unsh
    PRIVATE
        UPNPLIB_SHARED
)
target_link_libraries(test_simple-unsh
    PRIVATE
        upnplib_native_shared # prefix -unsh
)
add_test(NAME ctest_simple-unsh COMMAND test_simple-unsh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

# Check also static build for this simple test
add_executable(test_simple-unst
        ./test_simple.cpp
)
target_include_directories(test_simple-unst
    ${TEST_SIMPLE_INCLUDE_DIRECTORIES}
)
target_link_libraries(test_simple-unst
    PRIVATE
        upnplib_native_static # prefix -unst
)
add_test(NAME ctest_simple-unst COMMAND test_simple-unst
)


# template
#=========
add_executable(test_template
        test_template.cpp)
add_test(NAME ctest_template COMMAND test_template)


# miniserver
#===========
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts. We must use static libraries.
if(FALSE)
# Direct linking with source files currently not used but having available for
# reference. We link against the internal upnplib libraries (see below).
set(MINISERVER_SOURCE_FILES
#--------------------------
    ./test_miniserver.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/global.cpp
    ./tools/mockObj.cpp
    $<$<PLATFORM_ID:Windows>:./tools/mockObj_win32.cpp>

    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/FreeList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/LinkedList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/ThreadPool.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/TimerThread.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/sock.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/uri/uri.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/httpparser.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/httpreadwrite.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/statcodes.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/parsetools.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/webserver.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/list.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/membuffer.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/strintmap.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/ssdp/ssdp_server.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/ssdp/ssdp_ctrlpt.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/ssdp/SSDPResultData.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/ssdp/SSDPResultDataCallback.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/soap/soap_common.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/soap/soap_ctrlpt.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/uuid/uuid.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/uuid/sysdep.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/uuid/md5.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpExtraHeaders.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpString.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpFileInfo.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpDiscovery.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpActionComplete.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpStateVarComplete.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpapi.cpp
#    ${UPNPLIB_SOURCE_DIR}/src/api/upnptools.cpp
    $<$<CONFIG:Debug>:${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpdebug.cpp>
)
endif()

add_executable(test_miniserver-pst
#---------------------------------
# Needed to also compile additional sources instead of using that from the
# internal library. Otherwise the compiler complains 'multiple definition' of
# several symbols. This is because the compiler find symbols two times, one
# time with the included source file and one time from the library. If we use the
# source file here, the compiler doesn't look at the library. On static compiling
# the linker isn't used.
    ./test_miniserver.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/ssdp/ssdp_server.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpapi.cpp
    $<$<BOOL:${UPNPLIB_WITH_IPV6}>:${PUPNP_UPNP_SOURCE_DIR}/src/ssdp/ssdp_ctrlpt.cpp>
)
target_include_directories(test_miniserver-pst
    PRIVATE
        ./include
        ${UPNPLIB_PROJECT_SOURCE_DIR}
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${PUPNP_IXML_SOURCE_DIR}/inc
        $<$<NOT:$<PLATFORM_ID:Windows>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_miniserver-pst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
        UPNP_HAVE_CLIENT
        UPNP_HAVE_WEBSERVER
        UPNP_HAVE_SSDP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_miniserver-pst
    PRIVATE
        pupnp_static
        upnplib_native_static
        upnplib_gtest_tools_static
)
add_test(NAME ctest_miniserver-pst COMMAND test_miniserver-pst
)

add_executable(test_miniserver-cst
#---------------------------------
# Needed to also compile additional sources instead of using that from the
# internal library. Otherwise the compiler complains 'multiple definition' of
# several symbols. This is because the compiler find symbols two times, one
# time with the included source file and one time from the library. If we use the
# source file here, the compiler doesn't look at the library. On static compiling
# the linker isn't used.
    ./test_miniserver.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/ssdp/ssdp_server.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpapi.cpp
    $<$<BOOL:${UPNPLIB_WITH_IPV6}>:${PUPNP_UPNP_SOURCE_DIR}/src/ssdp/ssdp_ctrlpt.cpp>
)
target_include_directories(test_miniserver-cst
    PRIVATE
        ./include
        ${UPNPLIB_PROJECT_SOURCE_DIR}
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${PUPNP_IXML_SOURCE_DIR}/inc
        $<$<NOT:$<PLATFORM_ID:Windows>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_miniserver-cst
    PRIVATE
        UPNP_HAVE_CLIENT
        UPNP_HAVE_WEBSERVER
        UPNP_HAVE_SSDP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_miniserver-cst
    PRIVATE
        compa_static
        upnplib_native_static
        upnplib_gtest_tools_static
)
add_test(NAME ctest_miniserver-cst COMMAND test_miniserver-cst
)


add_subdirectory(./compa)
add_subdirectory(./test_uri.d)
add_subdirectory(./test_http.d)
add_subdirectory(./test_api.d)
add_subdirectory(./test_threadutil.d)
add_subdirectory(./upnplib_native)

if(UPNPLIB_WITH_SAMPLES)
    add_subdirectory(./test_sample.d)
endif()


# set(ENV{GITHUB_ACTIONS} 1)
# if(NOT APPLE)
# if(NOT DEFINED ENV{GITHUB_ACTIONS})
# target_compile_definitions(test_httpparser PRIVATE
#                            $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>)

# set_tests_properties( ctest_tv_device PROPERTIES
#     ENVIRONMENT "PATH=$ENV{PATH};${UPNPLIB_PROJECT_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}" )
