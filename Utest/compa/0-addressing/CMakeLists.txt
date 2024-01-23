# Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2023-12-22

cmake_minimum_required(VERSION 3.18)
include(../../../cmake/project-header.cmake)

project(UTEST_ADDRESSING VERSION 0011
        DESCRIPTION "Tests for UPnP Addressing pupnp and compatible code"
        HOMEPAGE_URL "https://github.com/upnplib")


# sock
#=====
add_executable(test_sock-psh
        test_sock.cpp
)
target_link_libraries(test_sock-psh
    PRIVATE
        pupnp_shared
        upnplib_shared
        utest_shared
)
add_test(NAME ctest_sock-psh COMMAND test_sock-psh --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

add_executable(test_sock-csh
        test_sock.cpp
)
target_link_libraries(test_sock-csh
    PRIVATE
        compa_shared
        upnplib_shared
        utest_shared
)
add_test(NAME ctest_sock-csh COMMAND test_sock-csh --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# sock_ssl
#=========
if(UPNPLIB_WITH_OPENSSL)
    add_executable(test_sock_ssl-psh
        test_sock_ssl.cpp
    )
    target_link_libraries(test_sock_ssl-psh
        PRIVATE
            pupnp_shared
            upnplib_shared
            utest_shared
    )
    add_test(NAME ctest_sock_ssl-psh COMMAND test_sock_ssl-psh --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
    )

    add_executable(test_sock_ssl-csh
        test_sock_ssl.cpp
    )
    target_link_libraries(test_sock_ssl-csh
        PRIVATE
            compa_shared
            upnplib_shared
            utest_shared
    )
    add_test(NAME ctest_sock_ssl-csh COMMAND test_sock_ssl-csh --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
    )


# openssl_threaded
#=================
if(FALSE)
# TODO: Tests need to be improved not to need a delay to be usable as Unit
# test. They also fail on WIN32 with Exception: Exit code 0xc0000409 means
# STATUS_STACK_BUFFER_OVERRUN.
    add_executable(test_openssl_threaded-psh
        test_openssl_threaded.cpp
    )
    target_link_libraries(test_openssl_threaded-psh
        PRIVATE pupnp_shared
        PRIVATE upnplib_shared
    )
    add_test(NAME ctest_openssl_threaded-psh COMMAND test_openssl_threaded-psh --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
    )

    add_executable(test_openssl_threaded-csh
        test_openssl_threaded.cpp
    )
    target_link_libraries(test_openssl_threaded-csh
        PRIVATE compa_shared
        PRIVATE upnplib_shared
    )
    add_test(NAME ctest_openssl_threaded-csh COMMAND test_openssl_threaded-csh --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
    )
endif()
endif()


# miniserver
#===========
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts. We must use static libraries.
#
# Direct linking with source files can be last found in
# commit b9c8fd4ad5f721a893de2181f5638b72b3014f0e.

add_executable(test_miniserver-pst
#---------------------------------
    test_miniserver.cpp
)
target_include_directories(test_miniserver-pst
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_link_libraries(test_miniserver-pst
    PRIVATE
        pupnp_static
        upnplib_static
        utest_static
)
add_test(NAME ctest_miniserver-pst COMMAND test_miniserver-pst --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

add_executable(test_miniserver-cst
#---------------------------------
    test_miniserver.cpp
)
target_include_directories(test_miniserver-cst
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_link_libraries(test_miniserver-cst
    PRIVATE
        compa_static
        upnplib_static
        utest_static
)
add_test(NAME ctest_miniserver-cst COMMAND test_miniserver-cst --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# miniserver_run
#===============
# disable warning C4273: inconsistent dll linkage.
# This warning on Microsoft Windows (MSVC) is because the source code is
# included into the test code to be able to test static functions. The compiled
# function in the link library is exported and it is also decorated to export
# with the included code. That's the warning. Because I know what I'm doing, I
# suppress it. It's not productive code. --Ingo
add_executable(test_miniserver_run-psh
#-------------------------------------
    test_miniserver_run.cpp
)
target_include_directories(test_miniserver_run-psh
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_compile_options(test_miniserver_run-psh
    # disable warning C4273: inconsistent dll linkage.
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/wd4273>
)
target_link_libraries(test_miniserver_run-psh
    PRIVATE
        pupnp_shared
        upnplib_shared
        utest_shared
)
add_test(NAME ctest_miniserver_run-psh COMMAND test_miniserver_run-psh --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

add_executable(test_miniserver_run-csh
#-------------------------------------
    test_miniserver_run.cpp
)
target_include_directories(test_miniserver_run-csh
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_compile_options(test_miniserver_run-csh
    # disable warning C4273: inconsistent dll linkage.
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/wd4273>
)
target_link_libraries(test_miniserver_run-csh
    PRIVATE
        compa_shared
        upnplib_shared
        utest_shared
)
add_test(NAME ctest_miniserver_run-csh COMMAND test_miniserver_run-csh --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)
