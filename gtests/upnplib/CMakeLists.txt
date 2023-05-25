# Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2023-05-24

cmake_minimum_required(VERSION 3.18)
include(../../cmake/project-header.cmake)

project(UPNPLIB_GTEST5 VERSION 0003
      DESCRIPTION "Tests for upnplib programs"
      HOMEPAGE_URL "https://github.com/upnplib")


# upnptools
#==========
add_executable(test_upnptools-ush
#--------------------------------
        ./test_upnptools.cpp
)
target_compile_definitions(test_upnptools-ush
    PRIVATE
        UPNPLIB_SHARED
)
target_compile_options(test_upnptools-ush
    PRIVATE
        # disable warning C4251: 'testing::internal::RE::pattern_': class
        # 'std::basic_string<char,std::char_traits<char>,std::allocator<char>>'
        # needs to have dll-interface to be used by clients of class
        # 'testing::internal::RE'
        $<$<CXX_COMPILER_ID:MSVC>:/wd4251>
)
target_link_libraries(test_upnptools-ush
    PRIVATE
        upnplib_shared
)
add_test(NAME ctest_upnptools-ush COMMAND test_upnptools-ush --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

add_executable(test_upnptools-ust
#---------------------------------
        ./test_upnptools.cpp
)
target_compile_options(test_upnptools-ust
    PRIVATE
        # disable warning C4251: 'testing::internal::RE::pattern_': class
        # 'std::basic_string<char,std::char_traits<char>,std::allocator<char>>'
        # needs to have dll-interface to be used by clients of class
        # 'testing::internal::RE'
        $<$<CXX_COMPILER_ID:MSVC>:/wd4251>
)
target_link_libraries(test_upnptools-ust
    PRIVATE
        upnplib_static
)
add_test(NAME ctest_upnptools-ust COMMAND test_upnptools-ust --gtest_shuffle
)


# sockaddr
# ========
add_executable(test_sockaddr-ush
        ./test_sockaddr.cpp
)
target_compile_definitions(test_sockaddr-ush
    PRIVATE
        UPNPLIB_SHARED
        #$<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)
target_compile_options(test_sockaddr-ush
    PRIVATE
        # disable warning C4100: 'result_listener': unreferenced formal parameter
        # This is from Googlemock MATCHER_P. Don't know how to fix it. Seems to
        # be a side effect of the macro definition. --Ingo
        #$<$<CXX_COMPILER_ID:MSVC>:/wd4100>
        # disable warning C4251: 'testing::internal::RE::pattern_': class
        # 'std::basic_string<char,std::char_traits<char>,std::allocator<char>>'
        # needs to have dll-interface to be used by clients of class
        # 'testing::internal::RE'
        #$<$<CXX_COMPILER_ID:MSVC>:/wd4251>
)
target_link_libraries(test_sockaddr-ush
    PRIVATE
        upnplib_shared
)
add_test(NAME ctest_sockaddr-ush COMMAND test_sockaddr-ush --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# CAddrinfo
# =========
add_executable(test_addrinfo-ush
    ./test_addrinfo.cpp
)
target_compile_definitions(test_addrinfo-ush
    PRIVATE
        UPNPLIB_SHARED
)
target_link_libraries(test_addrinfo-ush
    PRIVATE
        upnplib_shared
)
add_test(NAME ctest_addrinfo-ush COMMAND test_addrinfo-ush --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# CSocket
# =======
add_executable(test_socket-ush
    ./test_socket.cpp
)
target_compile_definitions(test_socket-ush
    PRIVATE
        UPNPLIB_SHARED
)
target_link_libraries(test_socket-ush
    PRIVATE
        upnplib_shared
)
if(NOT DEFINED ENV{GITHUB_ACTIONS})
add_test(NAME ctest_socket-ush COMMAND test_socket-ush --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)
endif()


# webserver
# =========
add_executable(test_webserver-ush
    ./test_webserver.cpp
)
target_include_directories(test_webserver-ush
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_compile_definitions(test_webserver-ush
    PRIVATE
        UPNPLIB_SHARED
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)
target_compile_options(test_webserver-ush
    PRIVATE
        # disable warning C4251: 'testing::internal::RE::pattern_': class
        # 'std::basic_string<char,std::char_traits<char>,std::allocator<char>>'
        # needs to have dll-interface to be used by clients of class
        # 'testing::internal::RE'
        $<$<CXX_COMPILER_ID:MSVC>:/wd4251>
)
target_link_libraries(test_webserver-ush
    PRIVATE
        upnplib_shared
)
add_test(NAME ctest_webserver-ush COMMAND test_webserver-ush --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


add_subdirectory(./test_uri.d)
if(UPNPLIB_WITH_SAMPLES)
    add_subdirectory(./test_sample.d)
endif()
