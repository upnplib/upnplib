# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2021-11-03

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

# set the project name and version
project(UPNP_GTESTS VERSION 0019
                    DESCRIPTION "Unit Tests using googletest"
                    HOMEPAGE_URL "https://github.com/upnplib")


#################################
# Build the Unit Tests          #
#################################
set(WINDOWS_EXPORT_ALL_SYMBOLS YES)
enable_testing()

include_directories(
    ${googletest_SOURCE_DIR}/googletest/include/
    ${googletest_SOURCE_DIR}/googlemock/include/
    ${PROJECT_SOURCE_DIR}/include
    ${UPNPLIB_CORE_SOURCE_DIR}/include
    ${UPNP_CORE_SOURCE_DIR}/inc
    ${UPNP_CORE_SOURCE_DIR}/src
    ${UPNP_CORE_SOURCE_DIR}/src/inc
    ${UPNP_CORE_SOURCE_DIR}/src/threadutil
    ${UPNP_CORE_BINARY_DIR}/inc
    ${UPnPlib_BINARY_DIR}
    ${UPnPlib_BINARY_DIR}/upnp/inc
    ${UPNP_IXML_SOURCE_DIR}/inc
    ${UPNP_IXML_SOURCE_DIR}/src/inc
    # if pthreads4w isn't installed this path is empty. The linker
    # doesn't find "pthread.h" and falls back to look at <pthread.h>.
    ${pthreads4w_SOURCE_DIR}/
)
# If linking with shared build gtest libs we need to tell it the compiler.
# I have found a vague hint about the flag GTEST_LINKED_AS_SHARED_LIBRARY at
# build/_deps/googletest-src/googletest/README.md
add_compile_definitions(
    PRIVATE $<$<AND:$<BOOL:${UPNP_GOOGLETEST}>,$<BOOL:${BUILD_SHARED_LIBS}>>:GTEST_LINKED_AS_SHARED_LIBRARY>
)

# Due to issue https://github.com/google/googletest/issues/1325#issuecomment-903884914
# it should only link with the gmock libraries on WIN32. On Unix it doesn't compile
# without gtest libraries.
link_libraries(${UPNP_GMOCK_LIBRARIES}
               ${UPNP_GMOCK_MAIN_LIBRARIES}
               $<$<NOT:$<BOOL:${WIN32}>>:${UPNP_GTEST_LIBRARIES}>
               $<$<NOT:$<BOOL:${WIN32}>>:${UPNP_GTEST_MAIN_LIBRARIES}>
               # On Linux we cannot link with the static pthreads library
               # because then we have also link with a static libc that's not
               # available by default.
               $<$<NOT:$<BOOL:${WIN32}>>:${UPNP_PTHREADS_SHARED_LIBRARY}>
               # On MS Windows we link with the static pthreads4w library to
               # avoid error prone managing access to its .dll file.
               $<$<BOOL:${WIN32}>:${UPNP_PTHREADS_STATIC_LIBRARY}>
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


#################################
# Unit Tests                    #
#################################
# The tests are build in reverse order. test_template.cpp will be build first.
#set(ENV{GITHUB_ACTIONS} 1)

# UpnpString
#-----------
add_executable(test_UpnpString_old ${PROJECT_SOURCE_DIR}/test_UpnpString.cpp)
add_test(NAME ctest_UpnpString_old COMMAND test_UpnpString_old)
target_compile_definitions(test_UpnpString_old
			   PRIVATE OLD_TEST
			   PRIVATE $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>
                           )
if(NOT DEFINED ENV{GITHUB_ACTIONS})
    add_executable(test_UpnpString ${PROJECT_SOURCE_DIR}/test_UpnpString.cpp)
    add_test(NAME ctest_UpnpString COMMAND test_UpnpString)
    target_compile_definitions(test_UpnpString
			       PRIVATE $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>)
endif()


# upnpdebug
#----------
add_executable(test_upnpdebug_old
    ${UPNP_CORE_SOURCE_DIR}/src/global.cpp
    ${PROJECT_SOURCE_DIR}/tools/tools.cpp
    ${PROJECT_SOURCE_DIR}/test_upnpdebug.cpp)
add_test(NAME ctest_upnpdebug_old COMMAND test_upnpdebug_old)
target_compile_definitions(test_upnpdebug_old
                           PRIVATE OLD_TEST)
if(NOT DEFINED ENV{GITHUB_ACTIONS})
    add_executable(test_upnpdebug
        ${UPNP_CORE_SOURCE_DIR}/src/global.cpp
        ${PROJECT_SOURCE_DIR}/tools/tools.cpp
        ${PROJECT_SOURCE_DIR}/test_upnpdebug.cpp)
    add_test(NAME ctest_upnpdebug COMMAND test_upnpdebug)
endif()


# upnpapi
#--------
set(UPNP_UPNPAPI_SOURCE_FILES
    ${UPNP_IXML_SOURCE_DIR}/src/attr.c
    ${UPNP_IXML_SOURCE_DIR}/src/document.c
    ${UPNP_IXML_SOURCE_DIR}/src/element.c
    ${UPNP_IXML_SOURCE_DIR}/src/ixml.c
    ${UPNP_IXML_SOURCE_DIR}/src/ixmlmembuf.c
    ${UPNP_IXML_SOURCE_DIR}/src/ixmlparser.c
    ${UPNP_IXML_SOURCE_DIR}/src/namedNodeMap.c
    ${UPNP_IXML_SOURCE_DIR}/src/node.c
    ${UPNP_IXML_SOURCE_DIR}/src/nodeList.c
    $<$<CONFIG:Debug>:${UPNP_IXML_SOURCE_DIR}/src/ixmldebug.c>

    ${UPNP_CORE_SOURCE_DIR}/src/global.cpp

    ${UPNP_CORE_SOURCE_DIR}/src/threadutil/FreeList.cpp
    ${UPNP_CORE_SOURCE_DIR}/src/threadutil/LinkedList.cpp
    ${UPNP_CORE_SOURCE_DIR}/src/threadutil/ThreadPool.cpp
    ${UPNP_CORE_SOURCE_DIR}/src/threadutil/TimerThread.cpp

    ${UPNP_CORE_SOURCE_DIR}/src/genlib/net/sock.c

    ${UPNP_CORE_SOURCE_DIR}/src/genlib/net/uri/uri.c

    ${UPNP_CORE_SOURCE_DIR}/src/genlib/net/http/httpparser.c
    ${UPNP_CORE_SOURCE_DIR}/src/genlib/net/http/httpreadwrite.c
    ${UPNP_CORE_SOURCE_DIR}/src/genlib/net/http/statcodes.c

    ${UPNP_CORE_SOURCE_DIR}/src/genlib/util/list.c
    ${UPNP_CORE_SOURCE_DIR}/src/genlib/util/membuffer.c
    ${UPNP_CORE_SOURCE_DIR}/src/genlib/util/strintmap.c

    ${UPNP_CORE_SOURCE_DIR}/src/api/UpnpExtraHeaders.c
    ${UPNP_CORE_SOURCE_DIR}/src/api/UpnpString.cpp
    ${UPNP_CORE_SOURCE_DIR}/src/api/upnpdebug.cpp

    ${PROJECT_SOURCE_DIR}/tools/tools.cpp
    ${PROJECT_SOURCE_DIR}/tools/upnpifaddrs.cpp
    ${PROJECT_SOURCE_DIR}/test_upnpapi.cpp
)
if(NOT DEFINED ENV{GITHUB_ACTIONS} AND NOT WIN32)
    # Not usable on MS Windows at time because <ifaddrs.h> isn't available
    # there. We need it for struct ifaddrs and functions getifaddrs(),
    # freeifaddrs().
    add_executable(test_upnpapi_old ${UPNP_UPNPAPI_SOURCE_FILES})
    target_compile_definitions(test_upnpapi_old PRIVATE OLD_TEST)
    add_test(NAME ctest_upnpapi_old COMMAND test_upnpapi_old)
    add_executable(test_upnpapi ${UPNP_UPNPAPI_SOURCE_FILES})
    add_test(NAME ctest_upnpapi COMMAND test_upnpapi)
    target_compile_definitions(test_upnpapi
                               PRIVATE $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>)
endif()

# LinkedList
#-----------
add_executable(test_LinkedList_old
        ${UPNP_CORE_SOURCE_DIR}/src/global.cpp
        ${PROJECT_SOURCE_DIR}/test_LinkedList.cpp)
add_test(NAME ctest_LinkedList_old COMMAND test_LinkedList_old)
target_compile_definitions(test_LinkedList_old
                           PRIVATE OLD_TEST)
if(NOT DEFINED ENV{GITHUB_ACTIONS})
add_executable(test_LinkedList
        ${UPNP_CORE_SOURCE_DIR}/src/global.cpp
        ${PROJECT_SOURCE_DIR}/test_LinkedList.cpp)
add_test(NAME ctest_LinkedList COMMAND test_LinkedList)
endif()

# FreeList
#---------
add_executable(test_FreeList_old
        ${UPNP_CORE_SOURCE_DIR}/src/global.cpp
        ${PROJECT_SOURCE_DIR}/test_FreeList.cpp)
add_test(NAME ctest_FreeList_old COMMAND test_FreeList_old)
target_compile_definitions(test_FreeList_old
                           PRIVATE OLD_TEST)
if(NOT DEFINED ENV{GITHUB_ACTIONS})
add_executable(test_FreeList
        ${UPNP_CORE_SOURCE_DIR}/src/global.cpp
        ${PROJECT_SOURCE_DIR}/test_FreeList.cpp)
add_test(NAME ctest_FreeList COMMAND test_FreeList)
endif()

# ThreadPool
#-----------
add_executable(test_ThreadPool_old
        ${UPNP_CORE_SOURCE_DIR}/src/global.cpp
        ${PROJECT_SOURCE_DIR}/test_ThreadPool.cpp)
add_test(NAME ctest_ThreadPool_old COMMAND test_ThreadPool_old)
target_compile_definitions(test_ThreadPool_old
                           PRIVATE OLD_TEST
                           #PRIVATE $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>
                           )
if(NOT DEFINED ENV{GITHUB_ACTIONS})
    add_executable(test_ThreadPool
            ${UPNP_CORE_SOURCE_DIR}/src/global.cpp
            ${PROJECT_SOURCE_DIR}/test_ThreadPool.cpp)
    add_test(NAME ctest_ThreadPool COMMAND test_ThreadPool)
endif()

# TimerThread
#------------
add_executable(test_TimerThread_old
        ${UPNP_CORE_SOURCE_DIR}/src/global.cpp
        ${PROJECT_SOURCE_DIR}/test_TimerThread.cpp)
add_test(NAME ctest_TimerThread_old COMMAND test_TimerThread_old)
target_compile_definitions(test_TimerThread_old
                           PRIVATE OLD_TEST
                           #PRIVATE $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>
                           )
if(NOT DEFINED ENV{GITHUB_ACTIONS})
    add_executable(test_TimerThread
            ${UPNP_CORE_SOURCE_DIR}/src/global.cpp
            ${PROJECT_SOURCE_DIR}/test_TimerThread.cpp)
    add_test(NAME ctest_TimerThread COMMAND test_TimerThread)
endif()

# simple test
#------------
add_executable(test_simple ${PROJECT_SOURCE_DIR}/test_simple.cpp)

# template
#---------
add_executable(test_template_old ${PROJECT_SOURCE_DIR}/test_template.cpp)
target_compile_definitions(test_template_old PRIVATE OLD_TEST)
add_executable(test_template ${PROJECT_SOURCE_DIR}/test_template.cpp)
