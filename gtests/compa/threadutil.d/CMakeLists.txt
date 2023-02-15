# Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2023-01-11

cmake_minimum_required(VERSION 3.18)
include(../../../cmake/project-header.cmake)

project(GTESTS_COMPA_THREADUTIL VERSION 0001
                  DESCRIPTION "Tests for the compa threadutil module"
                  HOMEPAGE_URL "https://github.com/upnplib")


# FreeList
#=========
# Additional FreeList tests can be found until
# git commit 1baf9b7cb29aadae5408a42461c013ae4638d92e

add_executable(test_FreeList-psh
#-------------------------------
        ./test_FreeList.cpp
)
target_compile_definitions(test_FreeList-psh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
)
target_compile_options(test_FreeList-psh
    PRIVATE
        # disable warning C4251: 'testing::internal::RE::pattern_': class
        # 'std::basic_string<char,std::char_traits<char>,std::allocator<char>>'
        # needs to have dll-interface to be used by clients of class
        # 'testing::internal::RE'
        $<$<CXX_COMPILER_ID:MSVC>:/wd4251>
)
target_link_libraries(test_FreeList-psh
    PRIVATE
        pupnp_shared
)
add_test(NAME ctest_FreeList-psh COMMAND test_FreeList-psh --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# LinkedList
#===========
add_executable(test_LinkedList-psh
        ./test_LinkedList.cpp
)
target_compile_definitions(test_LinkedList-psh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
)
target_compile_options(test_LinkedList-psh
    PRIVATE
        # disable warning C4251: 'testing::internal::RE::pattern_': class
        # 'std::basic_string<char,std::char_traits<char>,std::allocator<char>>'
        # needs to have dll-interface to be used by clients of class
        # 'testing::internal::RE'
        $<$<CXX_COMPILER_ID:MSVC>:/wd4251>
)
target_link_libraries(test_LinkedList-psh
    PRIVATE
        pupnp_shared
)
add_test(NAME ctest_LinkedList-psh COMMAND test_LinkedList-psh --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# ThreadPool
#===========
add_executable(test_ThreadPool-psh
        ./test_ThreadPool.cpp
)
target_include_directories(test_ThreadPool-psh
    PRIVATE ${COMPA_SOURCE_DIR}/include
)
target_compile_definitions(test_ThreadPool-psh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)
target_compile_options(test_ThreadPool-psh
    PRIVATE
        # disable warning C4251: 'testing::internal::RE::pattern_': class
        # 'std::basic_string<char,std::char_traits<char>,std::allocator<char>>'
        # needs to have dll-interface to be used by clients of class
        # 'testing::internal::RE'
        $<$<CXX_COMPILER_ID:MSVC>:/wd4251>
)
target_link_libraries(test_ThreadPool-psh
    PRIVATE
        pupnp_shared
)
add_test(NAME ctest_ThreadPool-psh COMMAND test_ThreadPool-psh --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# TimerThread
#============
add_executable(test_TimerThread-psh
        ./test_TimerThread.cpp
)
target_include_directories(test_TimerThread-psh
    PRIVATE ${COMPA_SOURCE_DIR}/include
)
target_compile_definitions(test_TimerThread-psh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)
target_compile_options(test_TimerThread-psh
    PRIVATE
        # disable warning C4251: 'testing::internal::RE::pattern_': class
        # 'std::basic_string<char,std::char_traits<char>,std::allocator<char>>'
        # needs to have dll-interface to be used by clients of class
        # 'testing::internal::RE'
        $<$<CXX_COMPILER_ID:MSVC>:/wd4251>
)
target_link_libraries(test_TimerThread-psh
    PRIVATE
        pupnp_shared
)
add_test(NAME ctest_TimerThread-psh COMMAND test_TimerThread-psh --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)
