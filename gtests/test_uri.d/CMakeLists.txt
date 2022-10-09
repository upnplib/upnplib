# Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-09-09

cmake_minimum_required(VERSION 3.18)
include(../../cmake/project-header.cmake)

project(GTESTS_URI VERSION 0002
                  DESCRIPTION "Tests for the uri module and url class"
                  HOMEPAGE_URL "https://github.com/upnplib")


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
        ${UPNPLIB_GTESTS_SOURCE_DIR}/tools/mockObj.cpp
        ./test_uri.cpp
)
endif()

add_executable(test_uri-pnst
        ./test_uri.cpp
)
target_include_directories(test_uri-pnst
    PRIVATE
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
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
        upnplib_native_static
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
        ${UPNPLIB_GTESTS_SOURCE_DIR}/tools/mockObj.cpp
        ${UPNPLIB_CORE_SOURCE_DIR}/src/api/upnptools.cpp
        ./test_uri_parse.cpp
)
endif()

add_executable(test_uri_parse-pnst
        ./test_uri_parse.cpp
)
target_include_directories(test_uri_parse-pnst
    PRIVATE
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
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
    ./test_urlparser.cpp
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
    ./test_url_class.cpp
)
target_include_directories(test_url_class
    PRIVATE
        ${UPnPlib_SOURCE_DIR}
        ${UPnPlib_SOURCE_DIR}/include
)
add_test(NAME ctest_url_class COMMAND test_url_class
)
endif()
