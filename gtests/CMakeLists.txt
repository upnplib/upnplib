# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2021-10-08

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

# set the project name and version
project(UPNP_GTESTS VERSION 0017
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
    ${UPNP_CORE_SOURCE_DIR}/include
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
        ${PROJECT_SOURCE_DIR}/tools/tools.cpp
        ${PROJECT_SOURCE_DIR}/test_upnpdebug.cpp)
    add_test(NAME ctest_upnpdebug_old COMMAND test_upnpdebug_old)
    target_compile_definitions(test_upnpdebug_old
                               PRIVATE OLD_TEST)
if(NOT DEFINED ENV{GITHUB_ACTIONS})
    add_executable(test_upnpdebug
        ${PROJECT_SOURCE_DIR}/tools/tools.cpp
        ${PROJECT_SOURCE_DIR}/test_upnpdebug.cpp)
    add_test(NAME ctest_upnpdebug COMMAND test_upnpdebug)
endif()


# upnpapi
set(UPNP_UPNPAPI_SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/ixml/src/attr.c
    ${CMAKE_SOURCE_DIR}/ixml/src/document.c
    ${CMAKE_SOURCE_DIR}/ixml/src/element.c
    ${CMAKE_SOURCE_DIR}/ixml/src/ixml.c
    ${CMAKE_SOURCE_DIR}/ixml/src/ixmlmembuf.c
    ${CMAKE_SOURCE_DIR}/ixml/src/ixmlparser.c
    ${CMAKE_SOURCE_DIR}/ixml/src/namedNodeMap.c
    ${CMAKE_SOURCE_DIR}/ixml/src/node.c
    ${CMAKE_SOURCE_DIR}/ixml/src/nodeList.c

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

# threadutil
#-----------
if(NOT DEFINED ENV{GITHUB_ACTIONS} AND NOT APPLE)
# These tests randomly segfault on macOS so they mostly fail
add_executable(test_threadutil_old ${PROJECT_SOURCE_DIR}/test_threadutil.cpp)
add_test(NAME ctest_threadutil_old COMMAND test_threadutil_old)
target_compile_definitions(test_threadutil_old
                           PRIVATE OLD_TEST
                           PRIVATE $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>)
endif()
if(NOT DEFINED ENV{GITHUB_ACTIONS})
    # test_threadutil segfaults before passing
    add_executable(test_threadutil ${PROJECT_SOURCE_DIR}/test_threadutil.cpp)
    add_test(NAME ctest_threadutil COMMAND test_threadutil)
    target_compile_definitions(test_threadutil
                               PRIVATE $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>)
endif()

# simple test
add_executable(test_simple ${PROJECT_SOURCE_DIR}/test_simple.cpp)

# template
add_executable(test_template_old ${PROJECT_SOURCE_DIR}/test_template.cpp)
target_compile_definitions(test_template_old PRIVATE OLD_TEST)
add_executable(test_template ${PROJECT_SOURCE_DIR}/test_template.cpp)
