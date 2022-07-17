# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-07-28

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

# set the project name and version
project(UPNPLIB_GTESTS VERSION 0027
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
        ${UPnPlib_SOURCE_DIR}/include
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
        ${UPnPlib_SOURCE_DIR}/include
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
add_executable(test_sock-pnsh
        ./test_sock.cpp
)
target_include_directories(test_sock-pnsh
    PRIVATE
        ./include
        ${UPnPlib_SOURCE_DIR}
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_IXML_SOURCE_DIR}/inc
)
target_compile_definitions(test_sock-pnsh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_sock-pnsh
    PRIVATE
        pupnp_native_shared
        upnplib_native_shared
)
add_test(NAME ctest_sock-pnsh COMMAND test_sock-pnsh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# strintmap
#==========
add_executable(test_strintmap-pnsh
        ./test_strintmap.cpp
)
target_include_directories(test_strintmap-pnsh
    PRIVATE
        ${UPnPlib_SOURCE_DIR}
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        $<$<NOT:$<PLATFORM_ID:Windows>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${PUPNP_IXML_SOURCE_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_strintmap-pnsh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_strintmap-pnsh
    PRIVATE
        pupnp_native_shared
)
add_test(NAME ctest_strintmap-pnsh COMMAND test_strintmap-pnsh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# list
#=====
add_executable(test_list
        ./test_list.cpp
)
target_include_directories(test_list
    PRIVATE
        ${UPnPlib_SOURCE_DIR}
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
        ${UPnPlib_SOURCE_DIR}
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


# uri
#====
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts. We must use static libraries.
if(FALSE)
# Direct linking with source files currently not used but having available for
# reference. We link against the internal upnplib libraries (see below).
add_executable(test_uri-pnst
	${PUPNP_UPNP_SOURCE_DIR}/src/global.cpp
	${PUPNP_UPNP_SOURCE_DIR}/src/mockObj.cpp
	./test_uri.cpp
)
endif()

add_executable(test_uri-pnst
        ./test_uri.cpp
)
target_include_directories(test_uri-pnst
    PRIVATE
        ./include
        ${UPnPlib_SOURCE_DIR}
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        $<$<NOT:$<PLATFORM_ID:Windows>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${PUPNP_IXML_SOURCE_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_uri-pnst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_uri-pnst
    PRIVATE
        pupnp_native_static
)
add_test(NAME ctest_uri-pnst COMMAND test_uri-pnst
)


# uri: uri_parse
#===============
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts. We must use static libraries.
if(FALSE)
# Direct linking with source files currently not used but having available for
# reference. We link against the internal upnplib libraries (see below).
add_executable(test_uri_parse-pnst
        ${PUPNP_UPNP_SOURCE_DIR}/src/global.cpp
        ${PUPNP_UPNP_SOURCE_DIR}/src/mockObj.cpp
        ${UPNPLIB_CORE_SOURCE_DIR}/src/api/upnptools.cpp
        ./test_uri.d/test_uri_parse.cpp
)
endif()

add_executable(test_uri_parse-pnst
        ./test_uri.d/test_uri_parse.cpp
)
target_include_directories(test_uri_parse-pnst
    PRIVATE
        ./include
        ${UPnPlib_SOURCE_DIR}
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        $<$<NOT:$<PLATFORM_ID:Windows>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_IXML_SOURCE_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_uri_parse-pnst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_uri_parse-pnst
    PRIVATE
        pupnp_native_static
        upnplib_native_static
)
add_test(NAME ctest_uri_parse-pnst COMMAND test_uri_parse-pnst
)


# uri: urlparser
#===============
add_executable(test_urlparser
    ./test_uri.d/test_urlparser.cpp
)
target_include_directories(test_urlparser
    PRIVATE
        ${UPnPlib_SOURCE_DIR}
        ${UPnPlib_SOURCE_DIR}/include
)
add_test(NAME ctest_urlparser COMMAND test_urlparser
)


# uri: url_class
#===============
# These tests need rework because on MS Windows with Debug enabled the compiler
# asserts boundary errors of the string pointer (iterator). With no Debug flag
# set the tests also run on MS Windows.
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND WIN32)
else()
add_executable(test_url_class
    ./test_uri.d/test_url_class.cpp
)
target_include_directories(test_url_class
    PRIVATE
        ${UPnPlib_SOURCE_DIR}
        ${UPnPlib_SOURCE_DIR}/include
)
add_test(NAME ctest_url_class COMMAND test_url_class
)
endif()


# httpparser
#===========
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts. We must use static libraries.
if(FALSE)
# Direct linking with source files currently not used but having available for
# reference. We link against the internal upnplib libraries (see below).
add_executable(test_httpparser-pnst
    ${PUPNP_UPNP_SOURCE_DIR}/src/global.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/mockObj.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/FreeList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/LinkedList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/uri/uri.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/list.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/membuffer.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/strintmap.cpp
    $<$<CONFIG:Debug>:${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpdebug.cpp>

    ./test_httpparser.cpp
)
endif()

add_executable(test_httpparser-pnst
    ./test_httpparser.cpp
)
target_include_directories(test_httpparser-pnst
    PRIVATE
        ./include
        ${UPnPlib_SOURCE_DIR}
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        $<$<NOT:$<PLATFORM_ID:Windows>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${PUPNP_IXML_SOURCE_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_httpparser-pnst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_httpparser-pnst
        PRIVATE
            pupnp_native_static
)
add_test(NAME ctest_httpparser-pnst COMMAND test_httpparser-pnst
)


# httpreadwrite incl. statcodes
#==============================
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts. We must use static libraries.
if(FALSE)
# Direct linking with source files currently not used but having available for
# reference. We link against the internal upnplib libraries (see below).
add_executable(test_httpreadwrite
    ${PUPNP_UPNP_SOURCE_DIR}/src/global.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/mockObj.cpp
    $<$<BOOL:${WIN32}>:${PUPNP_UPNP_SOURCE_DIR}/src/mockObj_win32.cpp>

    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/FreeList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/LinkedList.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/sock.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/uri/uri.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/httpparser.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/statcodes.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/list.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/membuffer.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/strintmap.cpp

    # ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpExtraHeaders.c
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpString.cpp
    $<$<CONFIG:Debug>:${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpdebug.cpp>

    ${UPNPLIB_CORE_SOURCE_DIR}/src/api/upnptools.cpp
    ./test_httpreadwrite.cpp
)
endif()

add_executable(test_httpreadwrite-pnst
        ./test_httpreadwrite.cpp
)
target_include_directories(test_httpreadwrite-pnst
    PRIVATE
        ./include
        ${UPnPlib_SOURCE_DIR}
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        $<$<NOT:$<PLATFORM_ID:Windows>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${PUPNP_IXML_SOURCE_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_httpreadwrite-pnst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_httpreadwrite-pnst
    PRIVATE
        pupnp_native_static
        upnplib_native_static
)
add_test(NAME ctest_httpreadwrite-pnst COMMAND test_httpreadwrite-pnst
)


# httpreadwrite: netconnect
#==========================
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts. We must use static libraries.
if(FALSE)
# Direct linking with source files currently not used but having available for
# reference. We link against the upnplib libraries (see below).
add_executable(test_netconnect
    ${PUPNP_UPNP_SOURCE_DIR}/src/global.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/mockObj.cpp
    $<$<BOOL:${WIN32}>:${PUPNP_UPNP_SOURCE_DIR}/src/mockObj_win32.cpp>

    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/FreeList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/LinkedList.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/sock.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/uri/uri.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/httpparser.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/statcodes.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/list.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/membuffer.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/strintmap.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpString.cpp
    $<$<CONFIG:Debug>:${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpdebug.cpp>

    test_httpreadwrite.d/test_netconnect.cpp
)
endif()

add_executable(test_netconnect-pnst
        ./test_httpreadwrite.d/test_netconnect.cpp
)
target_include_directories(test_netconnect-pnst
    PRIVATE
        ./include
        ${UPnPlib_SOURCE_DIR}
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        $<$<NOT:$<PLATFORM_ID:Windows>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${PUPNP_IXML_SOURCE_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_netconnect-pnst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_netconnect-pnst
    PRIVATE
        pupnp_native_static
)
add_test(NAME ctest_netconnect-pnst COMMAND test_netconnect-pnst
)


# UpnpString
#===========
add_executable(test_UpnpString-pnst
        ./test_UpnpString.cpp
)
target_include_directories(test_UpnpString-pnst
    PRIVATE
        ${UPnPlib_SOURCE_DIR}
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
)
target_compile_definitions(test_UpnpString-pnst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
add_test(NAME ctest_UpnpString-pnst COMMAND test_UpnpString-pnst
)


# upnpdebug
#==========
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts. We must use static libraries.
add_executable(test_upnpdebug-pnst
        ./test_upnpdebug.cpp
)
target_include_directories(test_upnpdebug-pnst
    PRIVATE
        ./include
        ${UPnPlib_SOURCE_DIR}
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_IXML_SOURCE_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_upnpdebug-pnst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_upnpdebug-pnst
    PRIVATE
        upnplib_gtest_tools_static
        pupnp_native_static
        upnplib_native_static
)
add_test(NAME ctest_upnpdebug-pnst COMMAND test_upnpdebug-pnst
)


# upnpapi
#========
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts. We must use static libraries.
if(FALSE)
# Direct linking with source files currently not used but having available for
# reference. We link against the internal upnplib libraries (see below).
set(UPNP_UPNPAPI_SOURCE_FILES
    ${PUPNP_IXML_SOURCE_DIR}/src/attr.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/document.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/element.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/ixml.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/ixmlmembuf.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/ixmlparser.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/namedNodeMap.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/node.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/nodeList.cpp
    $<$<CONFIG:Debug>:${PUPNP_IXML_SOURCE_DIR}/src/ixmldebug.cpp>

    ${PUPNP_UPNP_SOURCE_DIR}/src/global.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/mockObj.cpp
    $<$<BOOL:${WIN32}>:${PUPNP_UPNP_SOURCE_DIR}/src/mockObj_win32.cpp>
    $<$<NOT:$<BOOL:${WIN32}>>:${PUPNP_UPNP_SOURCE_DIR}/src/mockObj_unix.cpp>

    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/FreeList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/LinkedList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/ThreadPool.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/TimerThread.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/sock.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/uri/uri.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/httpparser.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/httpreadwrite.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/statcodes.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/list.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/membuffer.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/strintmap.cpp

    # ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpExtraHeaders.c
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpString.cpp
    $<$<CONFIG:Debug>:${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpdebug.cpp>

    ${UPNPLIB_CORE_SOURCE_DIR}/src/api/upnptools.cpp
)
endif()

add_executable(test_upnpapi-pnst
#-------------------------------
        ./test_upnpapi.cpp
)
target_include_directories(test_upnpapi-pnst
    PRIVATE
        ./include
        ${UPnPlib_SOURCE_DIR}
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_IXML_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        $<$<NOT:$<PLATFORM_ID:Windows>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_upnpapi-pnst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
        UPNP_HAVE_CLIENT
        UPNP_HAVE_DEVICE
        UPNP_HAVE_WEBSERVER
        UPNP_HAVE_SSDP
        UPNP_HAVE_OPTSSDP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_upnpapi-pnst
    PRIVATE
        pupnp_native_static
        upnplib_native_static
)
add_test(NAME ctest_upnpapi-pnst COMMAND test_upnpapi-pnst
)

add_executable(test_upnpapi_platform-pnsh
#----------------------------------------
        $<$<PLATFORM_ID:Windows>:./test_upnpapi_win32.cpp>
        $<$<NOT:$<PLATFORM_ID:Windows>>:./test_upnpapi_unix.cpp>
)
target_include_directories(test_upnpapi_platform-pnsh
    PRIVATE
        ./include
        ${UPnPlib_SOURCE_DIR}
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_IXML_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        $<$<NOT:$<PLATFORM_ID:Windows>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_upnpapi_platform-pnsh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_upnpapi_platform-pnsh
    PRIVATE
        upnplib_gtest_tools_shared
        pupnp_native_shared
        upnplib_native_shared
)
add_test(NAME ctest_upnpapi_platform-pnsh COMMAND test_upnpapi_platform-pnsh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

# LinkedList
#===========
add_executable(test_LinkedList-pnsh
        ./test_LinkedList.cpp
)
target_include_directories(test_LinkedList-pnsh
    PRIVATE
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_LinkedList-pnsh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_LinkedList-pnsh
    PRIVATE
        pupnp_native_shared
)
add_test(NAME ctest_LinkedList-pnsh COMMAND test_LinkedList-pnsh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

# FreeList
#=========
add_executable(test_FreeList-pnsh
        ./test_FreeList.cpp
)
target_include_directories(test_FreeList-pnsh
    PRIVATE
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_FreeList-pnsh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_FreeList-pnsh
    PRIVATE
        pupnp_native_shared
)
add_test(NAME ctest_FreeList-pnsh COMMAND test_FreeList-pnsh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

# ThreadPool
#===========
add_executable(test_ThreadPool-pnsh
        ./test_ThreadPool.cpp
)
target_include_directories(test_ThreadPool-pnsh
    PRIVATE
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${UPnPlib_SOURCE_DIR}/include
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_ThreadPool-pnsh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_ThreadPool-pnsh
    PRIVATE
        pupnp_native_shared
)
add_test(NAME ctest_ThreadPool-pnsh COMMAND test_ThreadPool-pnsh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

# TimerThread
#============
add_executable(test_TimerThread-pnsh
        ./test_TimerThread.cpp
)
target_include_directories(test_TimerThread-pnsh
    PRIVATE
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${UPnPlib_SOURCE_DIR}/include
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_TimerThread-pnsh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_TimerThread-pnsh
    PRIVATE
        pupnp_native_shared
)
add_test(NAME ctest_TimerThread-pnsh COMMAND test_TimerThread-pnsh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

# upnplib_gtest_tools
#====================
add_executable(test_upnplib_gtest_tools
        $<$<PLATFORM_ID:Windows>:./tools/test_tools_win32.cpp>
        $<$<NOT:$<PLATFORM_ID:Windows>>:./tools/test_tools_unix.cpp>
)
target_include_directories(test_upnplib_gtest_tools
        PRIVATE ./include
        PRIVATE ${UPnPlib_SOURCE_DIR}/include
)
target_compile_definitions(test_upnplib_gtest_tools
        PRIVATE
            UPNPLIB_SHARED
)
target_link_libraries(test_upnplib_gtest_tools
        PRIVATE upnplib_gtest_tools_shared
)
add_test(NAME ctest_upnplib_gtest_tools COMMAND test_upnplib_gtest_tools
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

# upnptools
#==========
set(TEST_UPNPTOOLS_INCLUDE_DIRECTORIES
    PRIVATE
        ./include
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_IXML_SOURCE_DIR}/inc
)

if(UPNPLIB_WITH_TOOLS)
add_executable(test_upnptools-pnsh
#---------------------------------
        ./test_upnptools_compa.cpp
)
target_include_directories(test_upnptools-pnsh
        ${TEST_UPNPTOOLS_INCLUDE_DIRECTORIES}
)
target_compile_definitions(test_upnptools-pnsh
    PRIVATE
        UPNPLIB_SHARED
)
target_link_libraries(test_upnptools-pnsh
    PRIVATE
        # pupnp native shared: prefix -pnsh
        pupnp_native_shared
        upnplib_native_shared
)
add_test(NAME ctest_upnptools-pnsh COMMAND test_upnptools-pnsh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

add_executable(test_upnptools-pnst
#---------------------------------
        ./test_upnptools_compa.cpp
)
target_include_directories(test_upnptools-pnst
        ${TEST_UPNPTOOLS_INCLUDE_DIRECTORIES}
)
target_link_libraries(test_upnptools-pnst
    PRIVATE
        # pupnp native static: prefix -pnst
        pupnp_native_static
        upnplib_native_static
)
add_test(NAME ctest_upnptools-pnst COMMAND test_upnptools-pnst
)
endif()

add_executable(test_upnptools-ucsh
#----------------------------------
        ./test_upnptools_compa.cpp
)
target_include_directories(test_upnptools-ucsh
        ${TEST_UPNPTOOLS_INCLUDE_DIRECTORIES}
)
target_compile_definitions(test_upnptools-ucsh
    PRIVATE
        UPNPLIB_SHARED
)
target_link_libraries(test_upnptools-ucsh
    PRIVATE
        # upnplib compatible shared: prefix -ucsh
        upnplib_compa_shared
        upnplib_native_shared
)
add_test(NAME ctest_upnptools-ucsh COMMAND test_upnptools-ucsh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

add_executable(test_upnptools-ucst
#---------------------------------
        ./test_upnptools_compa.cpp
)
target_include_directories(test_upnptools-ucst
        ${TEST_UPNPTOOLS_INCLUDE_DIRECTORIES}
)
target_link_libraries(test_upnptools-ucst
    PRIVATE
        # upnplib compatibe static: prefix -ucst
        upnplib_compa_static
        upnplib_native_static
)
add_test(NAME ctest_upnptools-ucst COMMAND test_upnptools-ucst
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

# simple test
#============
set(TEST_SIMPLE_INCLUDE_DIRECTORIES
    PRIVATE
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h on MS Windows
)

add_executable(test_simple-pnsh
#------------------------------
        ./test_simple_compa.cpp
)
target_include_directories(test_simple-pnsh
    ${TEST_SIMPLE_INCLUDE_DIRECTORIES}
)
target_compile_definitions(test_simple-pnsh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
)
target_link_libraries(test_simple-pnsh
    PRIVATE
        pupnp_native_shared # prefix -pnsh
        upnplib_native_shared
)
add_test(NAME ctest_simple-pnsh COMMAND test_simple-pnsh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

add_executable(test_simple-pnst
#------------------------------
        ./test_simple_compa.cpp
)
target_include_directories(test_simple-pnst
    ${TEST_SIMPLE_INCLUDE_DIRECTORIES}
)
target_compile_definitions(test_simple-pnst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
)
target_link_libraries(test_simple-pnst
    PRIVATE
        pupnp_native_static # prefix -pnst
        upnplib_native_static
)
add_test(NAME ctest_simple-pnst COMMAND test_simple-pnst
)

add_executable(test_simple-ucsh
#------------------------------
        ./test_simple_compa.cpp
)
target_include_directories(test_simple-ucsh
    ${TEST_SIMPLE_INCLUDE_DIRECTORIES}
)
target_compile_definitions(test_simple-ucsh
    PRIVATE
        UPNPLIB_SHARED
)
target_link_libraries(test_simple-ucsh
    PRIVATE
        upnplib_compa_shared # prefix -ucsh
        upnplib_native_shared
)
add_test(NAME ctest_simple-ucsh COMMAND test_simple-ucsh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

add_executable(test_simple-ucst
#------------------------------
        ./test_simple_compa.cpp
)
target_include_directories(test_simple-ucst
    ${TEST_SIMPLE_INCLUDE_DIRECTORIES}
)
target_link_libraries(test_simple-ucst
    PRIVATE
        upnplib_compa_static # prefix -ucst
        upnplib_native_static
)
add_test(NAME ctest_simple-ucst COMMAND test_simple-ucst
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
target_include_directories(test_template
    PRIVATE
        ./include)
add_test(NAME ctest_template COMMAND test_template)


# miniserver
#===========
# Because we want to include the source file into the test to also test static
# functions, we get definition conflicts if we use the libraries (function
# already defined). That's why we have to compile from sources.
#-----------------------------------------------------------------------------
set(MINISERVER_SOURCE_FILES
    ./test_miniserver.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/global.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/mockObj.cpp
    $<$<PLATFORM_ID:Windows>:${PUPNP_UPNP_SOURCE_DIR}/src/mockObj_win32.cpp>
    $<$<NOT:$<PLATFORM_ID:Windows>>:${PUPNP_UPNP_SOURCE_DIR}/src/mockObj_unix.cpp>

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

#    # ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpExtraHeaders.c
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpString.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpFileInfo.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpDiscovery.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpActionComplete.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpStateVarComplete.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpapi.cpp
#    ${UPNPLIB_CORE_SOURCE_DIR}/src/api/upnptools.cpp
    $<$<CONFIG:Debug>:${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpdebug.cpp>
)

add_executable(test_miniserver-pnsh
#----------------------------------
    ${MINISERVER_SOURCE_FILES}
)
target_include_directories(test_miniserver-pnsh
    PRIVATE
        ./include
        ${UPnPlib_SOURCE_DIR}
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_IXML_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        $<$<NOT:$<PLATFORM_ID:Windows>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_miniserver-pnsh
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
        UPNP_HAVE_CLIENT
        UPNP_HAVE_WEBSERVER
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_miniserver-pnsh
    PRIVATE
        $<$<PLATFORM_ID:Windows>:ixml_shared>
        upnplib_native_shared
)
add_test(NAME ctest_miniserver-pnsh COMMAND test_miniserver-pnsh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

add_executable(test_miniserver-ucsh
#----------------------------------
        ${MINISERVER_SOURCE_FILES}
)
target_include_directories(test_miniserver-ucsh
    PRIVATE
        ./include
        ${UPnPlib_SOURCE_DIR}
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_IXML_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        $<$<NOT:$<PLATFORM_ID:Windows>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_miniserver-ucsh
    PRIVATE
        UPNP_HAVE_CLIENT
        UPNP_HAVE_WEBSERVER
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_miniserver-ucsh
    PRIVATE
        $<$<PLATFORM_ID:Windows>:ixml_shared>
        upnplib_native_shared
)
add_test(NAME ctest_miniserver-ucsh COMMAND test_miniserver-ucsh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


add_subdirectory(./upnplib)

if(UPNPLIB_WITH_SAMPLES)
    add_subdirectory(./test_sample.d)
endif()


# set(ENV{GITHUB_ACTIONS} 1)
# if(NOT APPLE)
# if(NOT DEFINED ENV{GITHUB_ACTIONS})
# target_compile_definitions(test_httpparser PRIVATE
#                            $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>)

# set_tests_properties( ctest_tv_device PROPERTIES
#     ENVIRONMENT "PATH=$ENV{PATH};${UPnPlib_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}" )
