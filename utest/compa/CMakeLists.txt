# Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2023-11-20

cmake_minimum_required(VERSION 3.18)
include(../../cmake/project-header.cmake)

project(GTESTS_COMPA VERSION 0010
                  DESCRIPTION "Tests for pupnp and compatible code"
                  HOMEPAGE_URL "https://github.com/upnplib")


# sock
#=====
if(NOT DEFINED ENV{GITHUB_ACTIONS})
    add_executable(test_sock-psh
            ./test_sock.cpp
    )
    target_compile_definitions(test_sock-psh
        PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
    )
    target_link_libraries(test_sock-psh
        PRIVATE
            pupnp_shared
            upnplib_shared
    )
    add_test(NAME ctest_sock-psh COMMAND test_sock-psh --gtest_shuffle
            WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
    )

    add_executable(test_sock-csh
            ./test_sock.cpp
    )
    target_link_libraries(test_sock-csh
        PRIVATE
            compa_shared
            upnplib_shared
    )
    add_test(NAME ctest_sock-csh COMMAND test_sock-csh --gtest_shuffle
            WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
    )
endif()


# sock_openssl
#=============
if(UPNPLIB_WITH_OPENSSL)

    add_executable(test_sock_openssl-psh
        ./test_sock_openssl.cpp
    )
    target_link_libraries(test_sock_openssl-psh
        PRIVATE
            pupnp_shared
            upnplib_shared
            utest_shared
    )
    add_test(NAME ctest_sock_openssl-psh COMMAND test_sock_openssl-psh --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
    )

    add_executable(test_sock_openssl-csh
        ./test_sock_openssl.cpp
    )
    target_link_libraries(test_sock_openssl-csh
        PRIVATE
            compa_shared
            upnplib_shared
            utest_shared
    )
    add_test(NAME ctest_sock_openssl-csh COMMAND test_sock_openssl-csh --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
    )


# openssl_threaded
#=================
    add_executable(test_openssl_threaded-psh
        ./test_openssl_threaded.cpp
    )
    target_link_libraries(test_openssl_threaded-psh
        PRIVATE pupnp_shared
    )
    #add_test(NAME ctest_openssl_threaded-psh COMMAND test_openssl_threaded-psh --gtest_shuffle
    #    WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
    #)

    add_executable(test_openssl_threaded-csh
        ./test_openssl_threaded.cpp
    )
    target_link_libraries(test_openssl_threaded-csh
        PRIVATE
            compa_shared
    )
    #add_test(NAME ctest_openssl_threaded-csh COMMAND test_openssl_threaded-csh --gtest_shuffle
    #    WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
    #)

endif()


add_subdirectory(./0-addressing)
add_subdirectory(./api.d)
add_subdirectory(./http.d)
add_subdirectory(./ssdp.d)
add_subdirectory(./threadutil.d)
add_subdirectory(./uri.d)
add_subdirectory(./util.d)
