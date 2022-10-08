# Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-10-08

cmake_minimum_required(VERSION 3.18)
include(../../cmake/project-header.cmake)

project(GTESTS_COMPA VERSION 0007
                  DESCRIPTION "Tests for pupnp native and compatible code"
                  HOMEPAGE_URL "https://github.com/upnplib")


# UpnpString
#===========
add_executable(test_UpnpString-pnsh
#----------------------------------
        ./test_UpnpString.cpp
)
target_include_directories(test_UpnpString-pnsh
    PRIVATE
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
)
target_compile_definitions(test_UpnpString-pnsh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
)
target_link_libraries(test_UpnpString-pnsh
    PRIVATE
        pupnp_native_shared
)
add_test(NAME ctest_UpnpString-pnsh COMMAND test_UpnpString-pnsh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

if(NOT DEFINED ENV{GITHUB_ACTIONS})
add_executable(test_UpnpString-ucsh
#----------------------------------
        ./test_UpnpString.cpp
)
target_include_directories(test_UpnpString-ucsh
    PRIVATE
        ${UPNPLIB_GTESTS_SOURCE_DIR}/include
        ${UPnPlib_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${UPNPLIB_CORE_SOURCE_DIR}/compa/inc
)
target_compile_definitions(test_UpnpString-ucsh
    PRIVATE
        UPNPLIB_SHARED
)
target_link_libraries(test_UpnpString-ucsh
    PRIVATE
        upnplib_compa_shared
)
add_test(NAME ctest_UpnpString-ucsh COMMAND test_UpnpString-ucsh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)
endif()
