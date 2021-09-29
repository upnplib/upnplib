# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2021-09-25

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

# set the project name and version
project(UPNP_GTESTS VERSION 0014
                    DESCRIPTION "Unit Tests using googletest"
                    HOMEPAGE_URL "https://github.com/upnplib")


# Create the Unit Tests
#----------------------
set(WINDOWS_EXPORT_ALL_SYMBOLS YES)
enable_testing()

include_directories(
    ${googletest_SOURCE_DIR}/googletest/include/
    ${googletest_SOURCE_DIR}/googlemock/include/
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
    ${pthreads4w_SOURCE_DIR}/
)

# If linking with shared build gtest libs we need to tell it the compiler.
# I have found a vague hint about the flag GTEST_LINKED_AS_SHARED_LIBRARY at
# build/_deps/googletest-src/googletest/README.md
add_compile_definitions(
    PRIVATE $<$<AND:$<BOOL:${UPNP_GOOGLETEST}>,$<BOOL:${BUILD_SHARED_LIBS}>>:GTEST_LINKED_AS_SHARED_LIBRARY>
)

link_libraries(${UPNP_GTEST_LIBRARIES}
               # On Linux we cannot link with the static pthreads library
               # because then we have also link with a static libc that's not
               # available by default.
               $<$<NOT:$<BOOL:${WIN32}>>:${UPNP_PTHREADS_SHARED_LIBRARY}>
               # On MS Windows we link with the static pthreads4w library to
               # avoid error prone managing access to its .dll file.
               $<$<BOOL:${WIN32}>:${UPNP_PTHREADS_STATIC_LIBRARY}>
)
# For macOS we must tell the linker where it finds the shared libraries
add_link_options($<$<BOOL:${APPLE}>:LINKER:-rpath,${UPnPlib_BINARY_DIR}/lib>)


# The tests are build in reverse order. test_template.cpp will be build first.
if(NOT WIN32)
    # TODO: Make tests running on MS Windows

    # UpnpString
    # On MS Windows: error LNK2005: free already defined in test_UpnpString.cpp.obj
    # It does not occur if linking with shared libs
    add_executable(test_UpnpString ${PROJECT_SOURCE_DIR}/test_UpnpString.cpp)
    add_test(NAME ctest_UpnpString COMMAND test_UpnpString)
    add_executable(test_UpnpString_old ${PROJECT_SOURCE_DIR}/test_UpnpString.cpp)
    target_compile_definitions(test_UpnpString_old PRIVATE OLD_TEST)
    add_test(NAME ctest_UpnpString_old COMMAND test_UpnpString_old)
    add_executable(test_UpnpString_mock_old ${PROJECT_SOURCE_DIR}/test_UpnpString_mock.cpp)
    target_compile_definitions(test_UpnpString_mock_old PRIVATE OLD_TEST)
    add_test(NAME ctest_UpnpString_mock_old COMMAND test_UpnpString_mock_old)


    # upnpdebug
    # On MS Windows: test_upnpdebug.cpp(17,77): error C2440: 'type cast':
    # cannot convert from 'std ::filesystem::path' to 'std::string'
    add_executable(test_upnpdebug_old ${PROJECT_SOURCE_DIR}/test_upnpdebug.cpp)
    target_compile_definitions(test_upnpdebug_old PRIVATE OLD_TEST)
    add_test(NAME ctest_upnpdebug_old COMMAND test_upnpdebug_old)


    # upnpapi
    # Not usable on MS Windows at time because <ifaddrs.h> isn't available
    # there. We need it for struct ifaddrs and functions getifaddrs(),
    # freeifaddrs().
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

	${PROJECT_SOURCE_DIR}/test_upnpapi.cpp
	${PROJECT_SOURCE_DIR}/tools/tools.cpp
    )
    add_executable(test_upnpapi ${UPNP_UPNPAPI_SOURCE_FILES})
    add_executable(test_upnpapi_old ${UPNP_UPNPAPI_SOURCE_FILES})
    target_compile_definitions(test_upnpapi_old PRIVATE OLD_TEST)
    add_test(NAME ctest_upnpapi COMMAND test_upnpapi)
    add_test(NAME ctest_upnpapi_old COMMAND test_upnpapi_old)

    # Example link with upnplib library instead with sources
#    add_executable(test_upnpapi
#        ${PROJECT_SOURCE_DIR}/tools/tools.cpp
#        ${PROJECT_SOURCE_DIR}/test_upnpapi.cpp
#    )
#    target_link_libraries(test_upnpapi
#        ${UPNP_CORE_BINARY_DIR}/lib/libupnplib.a
#    )


    # upnpdebug_mock
    # On MS Windows: inc\tools.h(17,20): error C2079: 'CIfaddr4::ifaddr' uses
    # undefined struct 'ifaddrs'
    add_executable(test_upnpdebug_mock
        ${PROJECT_SOURCE_DIR}/tools/tools.cpp
        ${PROJECT_SOURCE_DIR}/test_upnpdebug_mock.cpp)
    add_test(NAME ctest_upnpdebug_mock COMMAND test_upnpdebug_mock)
    add_executable(test_upnpdebug_mock_old
        ${PROJECT_SOURCE_DIR}/tools/tools.cpp
        ${PROJECT_SOURCE_DIR}/test_upnpdebug_mock.cpp)
    target_compile_definitions(test_upnpdebug_mock_old PRIVATE OLD_TEST)
    add_test(NAME ctest_upnpdebug_mock_old COMMAND test_upnpdebug_mock_old)
endif()

# threadutil
add_executable(test_threadutil_old ${PROJECT_SOURCE_DIR}/test_threadutil.cpp)
target_compile_definitions(test_threadutil_old PRIVATE OLD_TEST)
add_test(NAME ctest_threadutil_old COMMAND test_threadutil_old)
add_executable(test_threadutil ${PROJECT_SOURCE_DIR}/test_threadutil.cpp)
add_test(NAME ctest_threadutil COMMAND test_threadutil)

# simple test
add_executable(test_simple ${PROJECT_SOURCE_DIR}/test_simple.cpp)
add_test(NAME ctest_simple COMMAND test_simple)

# template
add_executable(test_template_old ${PROJECT_SOURCE_DIR}/test_template.cpp)
target_compile_definitions(test_template_old PRIVATE OLD_TEST)
add_test(NAME ctest_template_old COMMAND test_template_old)
add_executable(test_template ${PROJECT_SOURCE_DIR}/test_template.cpp)
add_test(NAME ctest_template COMMAND test_template)
