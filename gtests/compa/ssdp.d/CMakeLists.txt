# Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2023-05-24

cmake_minimum_required(VERSION 3.18)
include(../../../cmake/project-header.cmake)

project(GTESTS_COMPA_SSDP VERSION 0001
                  DESCRIPTION "Tests for the compa ssdp module"
                  HOMEPAGE_URL "https://github.com/upnplib")


# ssdp_server
#============
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts. We must use static libraries.

add_executable(test_ssdp_server-pst
#----------------------------------
    ./test_ssdp_server.cpp
)
target_include_directories(test_ssdp_server-pst
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_compile_definitions(test_ssdp_server-pst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
        UPNP_HAVE_CLIENT
        UPNP_HAVE_DEVICE
        UPNP_HAVE_SSDP
        $<$<BOOL:${UPNPLIB_WITH_IPV6}>:UPNP_ENABLE_IPV6>
)
target_compile_options(test_ssdp_server-pst
    PRIVATE
        # disable warning C4251: 'testing::internal::RE::pattern_': class
        # 'std::basic_string<char,std::char_traits<char>,std::allocator<char>>'
        # needs to have dll-interface to be used by clients of class
        # 'testing::internal::RE'
        $<$<CXX_COMPILER_ID:MSVC>:/wd4251>
)
target_link_libraries(test_ssdp_server-pst
    PRIVATE
        pupnp_static
        upnplib_static
)
add_test(NAME ctest_ssdp_server-pst COMMAND test_ssdp_server-pst --gtest_shuffle
)

add_executable(test_ssdp_server-cst
#----------------------------------
    ./test_ssdp_server.cpp
)
target_include_directories(test_ssdp_server-cst
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_compile_definitions(test_ssdp_server-cst
    PRIVATE
        UPNP_HAVE_CLIENT
        UPNP_HAVE_DEVICE
        UPNP_HAVE_SSDP
        $<$<BOOL:${UPNPLIB_WITH_IPV6}>:UPNP_ENABLE_IPV6>
)
target_compile_options(test_ssdp_server-cst
    PRIVATE
        # disable warning C4251: 'testing::internal::RE::pattern_': class
        # 'std::basic_string<char,std::char_traits<char>,std::allocator<char>>'
        # needs to have dll-interface to be used by clients of class
        # 'testing::internal::RE'
        $<$<CXX_COMPILER_ID:MSVC>:/wd4251>
)
target_link_libraries(test_ssdp_server-cst
    PRIVATE
        compa_static
        upnplib_static
)
add_test(NAME ctest_ssdp_server-cst COMMAND test_ssdp_server-cst --gtest_shuffle
)
