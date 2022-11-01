# Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-10-31

cmake_minimum_required(VERSION 3.18)
include(../../../cmake/project-header.cmake)

project(GTESTS_COMPA_HTTP VERSION 0001
                  DESCRIPTION "Tests for the compa http module"
                  HOMEPAGE_URL "https://github.com/upnplib")


# webserver
#==========
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts. We must use static libraries.

add_executable(test_webserver-pst
#--------------------------------
    ./test_webserver.cpp
)
target_include_directories(test_webserver-pst
    PRIVATE
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
        ${UPNPLIB_PROJECT_SOURCE_DIR}
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        $<$<NOT:$<PLATFORM_ID:Windows>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${PUPNP_IXML_SOURCE_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_webserver-pst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
        UPNP_HAVE_WEBSERVER
        UPNP_HAVE_DEVICE
        UPNP_HAVE_SOAP
        UPNP_HAVE_SSDP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_webserver-pst
        PRIVATE
            pupnp_static
)
add_test(NAME ctest_webserver-pst COMMAND test_webserver-pst
)

add_executable(test_webserver-cst
#--------------------------------
    ./test_webserver.cpp
)
target_include_directories(test_webserver-cst
    PRIVATE
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
        ${UPNPLIB_PROJECT_SOURCE_DIR}
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        $<$<NOT:$<PLATFORM_ID:Windows>>:${PUPNP_UPNP_SOURCE_DIR}/inc/not_win32>
        ${PUPNP_UPNP_SOURCE_DIR}/src/inc
        ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil
        ${PUPNP_IXML_SOURCE_DIR}/inc
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        ${pthreads4w_SOURCE_DIR}   # needed for pthread.h
)
target_compile_definitions(test_webserver-cst
    PRIVATE
        UPNP_HAVE_WEBSERVER
        UPNP_HAVE_DEVICE
        UPNP_HAVE_SOAP
        UPNP_HAVE_SSDP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_webserver-cst
        PRIVATE
            compa_static
)
add_test(NAME ctest_webserver-cst COMMAND test_webserver-cst
)
