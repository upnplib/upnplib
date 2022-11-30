# Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-12-01

cmake_minimum_required(VERSION 3.18)
include(../../../cmake/project-header.cmake)

project(GTESTS_COMPA_URI VERSION 0003
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
add_executable(test_uri-pst
        ${PUPNP_UPNP_SOURCE_DIR}/src/global.cpp
        ${GTESTS_SOURCE_DIR}/tools/mockObj.cpp
        ./test_uri.cpp
)
endif()

add_executable(test_uri-pst
        ./test_uri.cpp
)
target_include_directories(test_uri-pst
    PRIVATE
        ${UPNPLIB_PROJECT_SOURCE_DIR}
        ${GTESTS_SOURCE_DIR}/include
)
target_compile_definitions(test_uri-pst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
)
target_compile_options(test_uri-pst
    PRIVATE
        # disable warning C4244: 'initializing': conversion from 'const int' to '_Ty', possible loss of data
        $<$<CXX_COMPILER_ID:MSVC>:/wd4244>
)
target_link_libraries(test_uri-pst
    PRIVATE
        pupnp_static
)
add_test(NAME ctest_uri-pst COMMAND test_uri-pst
)


# uri: uri_parse
#===============
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts. We must use static libraries.
if(FALSE)
# Direct linking with source files currently not used but having available for
# reference. We link against the internal upnplib libraries (see below).
add_executable(test_uri_parse-pst
        ${PUPNP_UPNP_SOURCE_DIR}/src/global.cpp
        ${GTESTS_SOURCE_DIR}/tools/mockObj.cpp
        ${UPNPLIB_SOURCE_DIR}/src/api/upnptools.cpp
        ./test_uri_parse.cpp
)
endif()

add_executable(test_uri_parse-pst
        ./test_uri_parse.cpp
)
target_include_directories(test_uri_parse-pst
    PRIVATE
        ${UPNPLIB_PROJECT_SOURCE_DIR}
        ${GTESTS_SOURCE_DIR}/include
)
target_compile_definitions(test_uri_parse-pst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
)
target_link_libraries(test_uri_parse-pst
    PRIVATE
        pupnp_static
        upnplib_static
)
add_test(NAME ctest_uri_parse-pst COMMAND test_uri_parse-pst
)
