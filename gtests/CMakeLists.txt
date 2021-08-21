# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2021-08-20

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

# set the project name and version
project(GTEST VERSION 0005
              DESCRIPTION "Unit Tests using googletest"
              HOMEPAGE_URL "https://github.com/upnplib")


# Install GoogleTest libraries
#-----------------------------
include(FetchContent)

# TODO: manage build option Release or Debug
FetchContent_Declare(
    googletest
    GIT_REPOSITORY    https://github.com/google/googletest.git
    GIT_TAG           origin/master
    GIT_SHALLOW       ON
    GIT_CONFIG        advice.detachedHead=false
)
FetchContent_MakeAvailable(googletest)

# Because we have different prefixes and suffixes depending on installation type
# and operating system we must use find_library to get the current gtest library
# names.
find_library(UPNP_GTEST_LIBRARY gtest gtestd
             PATHS ${CMAKE_BINARY_DIR}/lib
             PATH_SUFFIXES Debug Release MinSizeRel RelWithDebInfo
             NO_DEFAULT_PATH)
find_library(UPNP_GMOCK_LIBRARY gmock gmockd
             PATHS ${CMAKE_BINARY_DIR}/lib
             PATH_SUFFIXES Debug Release MinSizeRel RelWithDebInfo
             NO_DEFAULT_PATH)

find_library(UPNP_GTEST_MAIN_LIBRARY gtest_main gtest_maind
             PATHS ${CMAKE_BINARY_DIR}/lib
             PATH_SUFFIXES Debug Release MinSizeRel RelWithDebInfo
             NO_DEFAULT_PATH)
find_library(UPNP_GMOCK_MAIN_LIBRARY gmock_main gmock_maind
             PATHS ${CMAKE_BINARY_DIR}/lib
             PATH_SUFFIXES Debug Release MinSizeRel RelWithDebInfo
             NO_DEFAULT_PATH
)

# TODO: Use gtest to build and execute the following Unit Tests
return()

# Create the Unit Tests
#----------------------
include_directories(
    ${gtest_SOURCE_DIR}/include
    ${gmock_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include
    ${UPNP_CORE_SOURCE_DIR}/inc
    ${UPNP_CORE_SOURCE_DIR}/src
    ${UPNP_CORE_SOURCE_DIR}/src/inc
    ${UPNP_CORE_SOURCE_DIR}/src/threadutil
    ${UPNP_CORE_BINARY_DIR}/inc
    ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/upnp/inc
    ${CMAKE_SOURCE_DIR}/ixml/inc
    ${CMAKE_SOURCE_DIR}/ixml/src/inc
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

add_executable(test_threadutil ${PROJECT_SOURCE_DIR}/test_threadutil.cpp)
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
	${PROJECT_SOURCE_DIR}/tools/tools.cpp
    )

    # On MS Windows: test_upnpdebug_nomock.cpp(17,77): error C2440: 'type cast':
    # cannot convert from 'std ::filesystem::path' to 'std::string'
    add_executable(test_upnpdebug_nomock
        ${PROJECT_SOURCE_DIR}/test_upnpdebug_nomock.cpp
    )

    # On MS Windows: inc\tools.h(17,20): error C2079: 'CIfaddr4::ifaddr' uses
    # undefined struct 'ifaddrs'
    add_executable(test_upnpdebug
        ${PROJECT_SOURCE_DIR}/tools/tools.cpp
        ${PROJECT_SOURCE_DIR}/test_upnpdebug.cpp
    )
endif()

#add_executable(test_upnpapi
#    ${PROJECT_SOURCE_DIR}/tools/tools.cpp
#    ${PROJECT_SOURCE_DIR}/test_upnpapi.cpp
#)
#target_link_libraries(test_upnpapi
#    ${UPNP_CORE_BINARY_DIR}/lib/libupnplib.a
#)
