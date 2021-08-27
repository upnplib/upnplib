# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2021-08-27

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

# set the project name and version
project(UPNP_GTESTS VERSION 0008
              DESCRIPTION "Unit Tests using googletest"
              HOMEPAGE_URL "https://github.com/upnplib")


# Install GoogleTest libraries
#-----------------------------
# Download and install Googletest. The problem is that we need the libraries
# already build to get its paths in this configure stage. We do it with an
# isolated project (no add_subdirectory()). So we can configure and build them
# direct in two external processes and have them available immediately.

set(UPNP_GOOGLETEST_SOURCE_DIR ${PROJECT_SOURCE_DIR}/googletest)
set(UPNP_GOOGLETEST_BINARY_DIR ${PROJECT_SOURCE_DIR}/googletest/build)

file(REMOVE_RECURSE ${UPNP_GOOGLETEST_BINARY_DIR}/lib/
                    ${UPNP_GOOGLETEST_BINARY_DIR}/bin/)

execute_process(COMMAND cmake -S . -B build
                              -D BUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
                              -D CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                WORKING_DIRECTORY ${UPNP_GOOGLETEST_SOURCE_DIR}
                RESULT_VARIABLE RETURN_CODE
                ERROR_VARIABLE UPNP_ERROR_MESSAGE
                COMMAND_ECHO STDOUT)
if(NOT ${RETURN_CODE} EQUAL 0)
    # This will stop the installation
    message(FATAL_ERROR "${RETURN_CODE}: ${UPNP_ERROR_MESSAGE}")
endif()

execute_process(COMMAND cmake --build build --config ${CMAKE_BUILD_TYPE}
                WORKING_DIRECTORY ${UPNP_GOOGLETEST_SOURCE_DIR}
                RESULT_VARIABLE RETURN_CODE
                ERROR_VARIABLE UPNP_ERROR_MESSAGE
                COMMAND_ECHO STDOUT)
if(NOT ${RETURN_CODE} EQUAL 0)
    # This will stop the installation
    message(FATAL_ERROR "${RETURN_CODE}: ${UPNP_ERROR_MESSAGE}")
endif()

# Because we have different prefixes and suffixes depending on installation type
# and operating system we must use find_library to get the current gtest library
# names. The names are cached for the whole project so you can get them global.
unset (UPNP_GTEST_LIBRARY CACHE)
find_library(UPNP_GTEST_LIBRARY gtest gtestd
             DOC "Path to the Googletest library"
             PATHS ${UPNP_GOOGLETEST_BINARY_DIR}/lib
             PATH_SUFFIXES Debug Release MinSizeRel RelWithDebInfo
             NO_DEFAULT_PATH)
unset (UPNP_GMOCK_LIBRARY CACHE)
find_library(UPNP_GMOCK_LIBRARY gmock gmockd
             DOC "Path to the Googlemock library"
             PATHS ${UPNP_GOOGLETEST_BINARY_DIR}/lib
             PATH_SUFFIXES Debug Release MinSizeRel RelWithDebInfo
             NO_DEFAULT_PATH)

unset (UPNP_GTEST_MAIN_LIBRARY CACHE)
find_library(UPNP_GTEST_MAIN_LIBRARY gtest_main gtest_maind
             DOC "Path to the Googletest library with main entry point"
             PATHS ${UPNP_GOOGLETEST_BINARY_DIR}/lib
             PATH_SUFFIXES Debug Release MinSizeRel RelWithDebInfo
             NO_DEFAULT_PATH)
unset (UPNP_GMOCK_MAIN_LIBRARY CACHE)
find_library(UPNP_GMOCK_MAIN_LIBRARY gmock_main gmock_maind
             DOC "Path to the Googlemock library with main entry point"
             PATHS ${UPNP_GOOGLETEST_BINARY_DIR}/lib
             PATH_SUFFIXES Debug Release MinSizeRel RelWithDebInfo
             NO_DEFAULT_PATH)


enable_testing()

# Create the Unit Tests
#----------------------
include_directories(
    ${UPNP_GOOGLETEST_BINARY_DIR}/_deps/googletest-src/googletest/include
    ${UPNP_GOOGLETEST_BINARY_DIR}/_deps/googletest-src/googlemock/include
    ${PROJECT_SOURCE_DIR}/include
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
    ${PTHREADS4W_BINARY_DIR}/include
)

link_libraries(${UPNP_GTEST_LIBRARY}
               ${UPNP_GMOCK_LIBRARY}
               ${UPNP_GTEST_MAIN_LIBRARY}
               ${UPNP_GMOCK_MAIN_LIBRARY}
               ${UPNP_PTHREADS_SHARED_LIBRARY}
               # It isn't possible to link with static lib due to link errors
               # ${UPNP_PTHREADS_STATIC_LIBRARY}
)

add_executable(test_template ${PROJECT_SOURCE_DIR}/test_template.cpp)
add_test(NAME ctest_template COMMAND test_template)

add_executable(test_threadutil ${PROJECT_SOURCE_DIR}/test_threadutil.cpp)
add_test(NAME ctest_threadutil COMMAND test_threadutil)
#target_compile_definitions(test_threadutil
#        PUBLIC $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>
#)

if(NOT WIN32)
    # TODO: Make tests running on MS Windows
    # Not usable on MS Windows at time because <ifaddrs.h> isn't available
    # there. We need it for struct ifaddrs and functions getifaddrs(),
    # freeifaddrs().
    add_executable(test_upnpapi
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
	${UPNP_CORE_SOURCE_DIR}/src/api/UpnpString.c
	# ${UPNP_CORE_SOURCE_DIR}/src/api/upnpdebug.cpp

	${PROJECT_SOURCE_DIR}/test_upnpapi.cpp
	${PROJECT_SOURCE_DIR}/tools/tools.cpp)
    add_test(NAME ctest_upnpapi COMMAND test_upnpapi)

    # Example link with upnplib library instead with sources
    #add_executable(test_upnpapi
    #    ${PROJECT_SOURCE_DIR}/tools/tools.cpp
    #    ${PROJECT_SOURCE_DIR}/test_upnpapi.cpp
    #)
    #target_link_libraries(test_upnpapi
    #    ${UPNP_CORE_BINARY_DIR}/lib/libupnplib.a
    #)

    # On MS Windows: test_upnpdebug_nomock.cpp(17,77): error C2440: 'type cast':
    # cannot convert from 'std ::filesystem::path' to 'std::string'
    add_executable(test_upnpdebug_nomock
        ${PROJECT_SOURCE_DIR}/test_upnpdebug_nomock.cpp)
    add_test(NAME ctest_upnpdebug_nomock COMMAND test_upnpdebug_nomock)

    # On MS Windows: inc\tools.h(17,20): error C2079: 'CIfaddr4::ifaddr' uses
    # undefined struct 'ifaddrs'
    add_executable(test_upnpdebug
        ${PROJECT_SOURCE_DIR}/tools/tools.cpp
        ${PROJECT_SOURCE_DIR}/test_upnpdebug.cpp)
    add_test(NAME ctest_upnpdebug COMMAND test_upnpdebug)
endif()
