# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-03-09

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

# set the project name and version
project(UPNPLIB_GTESTS VERSION 0023
                       DESCRIPTION "Unit Tests using googletest"
                       HOMEPAGE_URL "https://github.com/upnplib")


#################################
# Build the Unit Tests          #
#################################

include_directories(
    # Root project
    ${UPnPlib_SOURCE_DIR}
    ${UPnPlib_SOURCE_DIR}/include
    ${UPnPlib_BINARY_DIR}
    ${UPnPlib_BINARY_DIR}/upnp/inc
    # Googletest
    ${googletest_SOURCE_DIR}/googletest/include
    ${googletest_SOURCE_DIR}/googlemock/include
    # gtests
    ${PROJECT_SOURCE_DIR}/include
    # pupnp old code root project
    ${PUPNP_SOURCE_DIR}
    # pupnp core library project
    ${PUPNP_UPNP_SOURCE_DIR}/inc
    # if we are not on WIN32 the linker finds empty "headers.h" and will
    # not complain an error but will not use <headers.h> that isn't available.
    $<$<NOT:$<BOOL:${WIN32}>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
    ${PUPNP_UPNP_SOURCE_DIR}/src
    ${PUPNP_UPNP_SOURCE_DIR}/src/inc
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
    # pupnp ixml project
    ${PUPNP_IXML_SOURCE_DIR}/inc
    ${PUPNP_IXML_SOURCE_DIR}/src/inc
    # if pthreads4w isn't installed this path is empty. The linker
    # doesn't find "pthread.h" and falls back to look at <pthread.h>.
    ${pthreads4w_SOURCE_DIR}/
)
# If linking with shared build gtest libs we need to tell it the compiler.
# I have found a vague hint about the flag GTEST_LINKED_AS_SHARED_LIBRARY at
# build/_deps/googletest-src/googletest/README.md
add_compile_definitions(PRIVATE
    $<$<AND:$<BOOL:${UPNPLIB_WITH_GOOGLETEST}>,$<BOOL:${BUILD_SHARED_LIBS}>>:GTEST_LINKED_AS_SHARED_LIBRARY>
    # Compile tests for the old (pupnp) source code.
    OLD_TEST
)

# Due to issue https://github.com/google/googletest/issues/1325#issuecomment-903884914
# it should only link with the gmock libraries on WIN32. On Unix it doesn't compile
# without gtest libraries.
link_libraries(gmock
               gmock_main
               $<$<NOT:$<BOOL:${WIN32}>>:gtest>
               $<$<NOT:$<BOOL:${WIN32}>>:gtest_main>
               # On Linux we cannot link with the static pthreads library
               # because then we have also link with a static libc that's not
               # available by default.
               $<$<NOT:$<BOOL:${WIN32}>>:${UPNP_PTHREADS_SHARED_LIBRARY}>
               # On MS Windows we link with the static pthreads4w library to
               # avoid error prone managing access to its .dll file.
               $<$<BOOL:${WIN32}>:${UPNP_PTHREADS_STATIC_LIBRARY}>
               # On MS Windows we need winsocket and ip helper
               $<$<BOOL:${WIN32}>:ws2_32>
               $<$<BOOL:${WIN32}>:iphlpapi>
)
# For macOS we must tell the linker where it finds the external shared libraries.
# This is linked into the executable. Alternative or additional you can also
# set the environment variable DYLD_LIBRARY_PATH.
add_link_options($<$<AND:$<BOOL:${APPLE}>,$<BOOL:${BUILD_SHARED_LIBS}>>:LINKER:-rpath,${UPnPlib_BINARY_DIR}/lib>)

# Maybe we need this on MS Windows, or with /MD[d].
# Reference: https://gitlab.kitware.com/cmake/cmake/-/issues/18390#note_462068
#if(MSVC)
#    add_compile_options(
#        $<$<CONFIG:>:/MT> #------------|
#        $<$<CONFIG:Debug>:/MTd> #------|-- Statically link the runtime libraries
#        $<$<CONFIG:Release>:/MT> #-----|
#        $<$<CONFIG:MinSizeRel>:/MT> #--|
#    )
#endif()

# Create upnplib helper tools library for gtests. We have version
# upnplib_gtest_tools for all operating systems,
# upnplib_gtest_tools_unix for Unix operating systems and
# upnplib_gtest_tools_win32 for Microsoft Windows
add_library(upnplib_gtest_tools SHARED
        tools/upnplib_gtest_tools.cpp
        $<$<BOOL:${WIN32}>:${PROJECT_SOURCE_DIR}/tools/upnplib_gtest_tools_win32.cpp>
        $<$<NOT:$<BOOL:${WIN32}>>:${PROJECT_SOURCE_DIR}/tools/upnplib_gtest_tools_unix.cpp>)
set_target_properties(upnplib_gtest_tools PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON)
target_compile_definitions(upnplib_gtest_tools PRIVATE UPNPLIB_SHARED UPNPLIB_EXPORTS)
add_dependencies(upnplib_gtest_tools gtest gtest_main gmock gmock_main)


#################################
# Unit Tests                    #
#################################
# The tests are build in reverse order. test_template.cpp will be build first.

# sock
#-----
add_executable(test_sock
    ${PUPNP_UPNP_SOURCE_DIR}/src/mockObj.cpp
    ${UPNPLIB_CORE_SOURCE_DIR}/src/api/upnptools.cpp
    $<$<CONFIG:Debug>:${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpdebug.cpp>
    ${PROJECT_SOURCE_DIR}/test_sock.cpp)
target_compile_definitions(test_sock
    PRIVATE $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>)
add_test(NAME ctest_sock COMMAND test_sock)

# strintmap
#----------
add_executable(test_strintmap
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/membuffer.cpp
    ${PROJECT_SOURCE_DIR}/test_strintmap.cpp)
add_test(NAME ctest_strintmap COMMAND test_strintmap)

# list
#-----
add_executable(test_list
    ${PROJECT_SOURCE_DIR}/test_list.cpp)
target_compile_definitions(test_list PRIVATE
                           $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>)
add_test(NAME ctest_list COMMAND test_list)

# membuffer
#----------
add_executable(test_membuffer
    test_membuffer.cpp
)
add_test(NAME ctest_membuffer COMMAND test_membuffer)

# uri
#----
add_executable(test_uri
    ${PUPNP_UPNP_SOURCE_DIR}/src/global.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/mockObj.cpp
    test_uri.cpp)
target_link_libraries(test_uri upnplib_gtest_tools)
add_test(NAME ctest_uri COMMAND test_uri)

# uri: uri_parse
#---------------
add_executable(test_uri_parse
    ${PUPNP_UPNP_SOURCE_DIR}/src/global.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/mockObj.cpp
    ${UPNPLIB_CORE_SOURCE_DIR}/src/api/upnptools.cpp
    test_uri.d/test_uri_parse.cpp)
add_test(NAME ctest_uri_parse COMMAND test_uri_parse)

# uri: url_class
#---------------
add_executable(test_url_class
    ${UPNPLIB_CORE_SOURCE_DIR}/src/genlib/net/uri/url.cpp
    test_uri.d/test_url_class.cpp)
add_test(NAME ctest_url_class COMMAND test_url_class)


# httpparser
#-----------
add_executable(test_httpparser
    ${PUPNP_UPNP_SOURCE_DIR}/src/global.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/mockObj.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/FreeList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/LinkedList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/uri/uri.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/list.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/membuffer.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/strintmap.cpp
    $<$<CONFIG:Debug>:${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpdebug.cpp>

    ${PROJECT_SOURCE_DIR}/test_httpparser.cpp
)
target_compile_definitions(test_httpparser PRIVATE
                            $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>)
add_test(NAME ctest_httpparser COMMAND test_httpparser)


# httpreadwrite incl. statcodes
#------------------------------
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
    test_httpreadwrite.cpp
)
target_compile_definitions(test_httpreadwrite PRIVATE
                            $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>)
add_test(NAME ctest_httpreadwrite COMMAND test_httpreadwrite)

# httpreadwrite: netconnect
#--------------------------
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
target_compile_definitions(test_netconnect PRIVATE
                            $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>)
target_link_libraries(test_netconnect upnplib_gtest_tools)
add_test(NAME ctest_netconnect COMMAND test_netconnect)


# UpnpString
#-----------
add_executable(test_UpnpString ${PROJECT_SOURCE_DIR}/test_UpnpString.cpp)
target_compile_definitions(test_UpnpString PRIVATE
                            $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>)
add_test(NAME ctest_UpnpString COMMAND test_UpnpString)

# upnpdebug
#----------
add_executable(test_upnpdebug
    ${PUPNP_UPNP_SOURCE_DIR}/src/mockObj.cpp
    ${UPNPLIB_CORE_SOURCE_DIR}/src/api/upnptools.cpp
    test_upnpdebug.cpp)
target_compile_definitions(test_upnpdebug
               PRIVATE $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>)
target_compile_options(test_upnpdebug
        # suppress warning C4311: 'type cast': pointer truncation
        PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/wd4311>)
target_link_libraries(test_upnpdebug upnplib_gtest_tools)
add_test(NAME ctest_upnpdebug COMMAND test_upnpdebug)


# upnpapi
#--------
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

add_executable(test_upnpapi ${UPNP_UPNPAPI_SOURCE_FILES}
               $<$<BOOL:${WIN32}>:${PROJECT_SOURCE_DIR}/test_upnpapi_win32.cpp>
               $<$<NOT:$<BOOL:${WIN32}>>:${PROJECT_SOURCE_DIR}/test_upnpapi_unix.cpp>)
target_compile_definitions(test_upnpapi
               PRIVATE $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>)
target_compile_options(test_upnpapi PRIVATE
                       # suppress warning C4311: 'type cast': pointer truncation
                       $<$<CXX_COMPILER_ID:MSVC>:/wd4311>)
target_link_libraries(test_upnpapi
                      upnplib_gtest_tools)
add_test(NAME ctest_upnpapi COMMAND test_upnpapi
)

# LinkedList
#-----------
add_executable(test_LinkedList
        ${PUPNP_UPNP_SOURCE_DIR}/src/mockObj.cpp
        ${PROJECT_SOURCE_DIR}/test_LinkedList.cpp)
add_test(NAME ctest_LinkedList COMMAND test_LinkedList)

# FreeList
#---------
add_executable(test_FreeList
        ${PUPNP_UPNP_SOURCE_DIR}/src/mockObj.cpp
        ${PROJECT_SOURCE_DIR}/test_FreeList.cpp)
add_test(NAME ctest_FreeList COMMAND test_FreeList)

# ThreadPool
#-----------
add_executable(test_ThreadPool
        ${PUPNP_UPNP_SOURCE_DIR}/src/mockObj.cpp
        ${PROJECT_SOURCE_DIR}/test_ThreadPool.cpp)
target_compile_options(test_ThreadPool PRIVATE
        # suppress warning C4311: 'type cast': pointer truncation
        $<$<CXX_COMPILER_ID:MSVC>:/wd4311>)
add_test(NAME ctest_ThreadPool COMMAND test_ThreadPool)

# TimerThread
#------------
add_executable(test_TimerThread
        ${PUPNP_UPNP_SOURCE_DIR}/src/mockObj.cpp
        ${PROJECT_SOURCE_DIR}/test_TimerThread.cpp)
target_compile_options(test_TimerThread PRIVATE
        # suppress warning C4311: 'type cast': pointer truncation
        $<$<CXX_COMPILER_ID:MSVC>:/wd4311>)
add_test(NAME ctest_TimerThread COMMAND test_TimerThread)

# test upnplib_gtest_tools
#-------------------------
add_executable(test_upnplib_gtest_tools
        $<$<BOOL:${WIN32}>:${PROJECT_SOURCE_DIR}/tools/test_tools_win32.cpp>
        $<$<NOT:$<BOOL:${WIN32}>>:${PROJECT_SOURCE_DIR}/tools/test_tools_unix.cpp>)
target_compile_definitions(test_upnplib_gtest_tools
        PRIVATE UPNPLIB_SHARED)
target_link_libraries(test_upnplib_gtest_tools
        upnplib_gtest_tools)
add_test(NAME ctest_upnplib_gtest_tools COMMAND test_upnplib_gtest_tools)

# upnptools
#----------
add_executable(test_upnptools test_upnptools.cpp)
target_compile_definitions(test_upnptools
        PRIVATE $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>)
add_test(NAME ctest_upnptools COMMAND test_upnptools
)
add_executable(test_upnptools_em test_upnptools.cpp)
target_compile_definitions(test_upnptools_em
        PRIVATE UPNPLIB_PUPNP_EMULATION
        PRIVATE $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>)
add_test(NAME ctest_upnptools_em COMMAND test_upnptools_em)

# simple test
#------------
add_executable(test_simple test_simple.cpp)

# template
#---------
add_executable(test_template test_template.cpp)

# set(ENV{GITHUB_ACTIONS} 1)
# if(NOT APPLE)
# if(NOT DEFINED ENV{GITHUB_ACTIONS})
# target_compile_definitions(test_httpparser PRIVATE
#                            $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>)
