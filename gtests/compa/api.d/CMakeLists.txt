# Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2023-08-16

cmake_minimum_required(VERSION 3.18)
include(../../../cmake/project-header.cmake)

project(GTESTS_COMPA_API VERSION 0002
                  DESCRIPTION "Tests for api module and compatible code"
                  HOMEPAGE_URL "https://github.com/upnplib")

# upnpdebug
#==========
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts on Microsoft Windows (warning C4273: inconsistent dll linkage). We
# must use static libraries.
add_executable(test_upnpdebug-pst
#--------------------------------
        ./test_upnpdebug.cpp
)
target_include_directories(test_upnpdebug-pst
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_compile_options(test_upnpdebug-pst
    PRIVATE
        # disable warning C4459: declaration of 'fp' hides global declaration
        # This is intended and don't fixed it because we won't modify pupnp code.
        $<$<CXX_COMPILER_ID:MSVC>:/wd4459>
        # disable warning C4100: 'result_listener': unreferenced formal parameter
        # This is from Googlemock MATCHER_P. Don't know how to fix it. Seems to
        # be a side effect of the macro definition. --Ingo
        $<$<CXX_COMPILER_ID:MSVC>:/wd4100>
)
target_link_libraries(test_upnpdebug-pst
    PRIVATE
        pupnp_static
        upnplib_static
        upnplib_gtest_tools_static
)
add_test(NAME ctest_upnpdebug-pst COMMAND test_upnpdebug-pst --gtest_shuffle
)

add_executable(test_upnpdebug-cst
#--------------------------------
        ./test_upnpdebug.cpp
)
target_include_directories(test_upnpdebug-cst
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_compile_options(test_upnpdebug-cst
    PRIVATE
        # disable warning C4100: 'result_listener': unreferenced formal parameter
        # This is from Googlemock MATCHER_P. Don't know how to fix it. Seems to
        # be a side effect of the macro definition. --Ingo
        $<$<CXX_COMPILER_ID:MSVC>:/wd4100>
)
target_link_libraries(test_upnpdebug-cst
    PRIVATE
        compa_static
        upnplib_static
        upnplib_gtest_tools_static
)
add_test(NAME ctest_upnpdebug-cst COMMAND test_upnpdebug-cst --gtest_shuffle
)


# upnptools
#==========
if(UPNPLIB_WITH_TOOLS)
add_executable(test_upnptools-psh
#--------------------------------
        ./test_upnptools.cpp
)
target_link_libraries(test_upnptools-psh
    PRIVATE pupnp_shared
)
add_test(NAME ctest_upnptools-psh COMMAND test_upnptools-psh --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)
endif()


# UpnpString
#===========
add_executable(test_UpnpString-psh
#---------------------------------
    ./test_UpnpString.cpp
)
target_link_libraries(test_UpnpString-psh
    PRIVATE pupnp_shared
)
add_test(NAME ctest_UpnpString-psh COMMAND test_UpnpString-psh --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

add_executable(test_UpnpString-csh
#---------------------------------
    ./test_UpnpString.cpp
)
target_link_libraries(test_UpnpString-csh
    PRIVATE compa_shared
)
add_test(NAME ctest_UpnpString-csh COMMAND test_UpnpString-csh --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# UpnpFileInfo
#=============
add_executable(test_UpnpFileInfo-psh
#-----------------------------------
    ./test_UpnpFileInfo.cpp
)
target_compile_definitions(test_UpnpFileInfo-psh
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)
target_link_libraries(test_UpnpFileInfo-psh
    PRIVATE
        pupnp_shared
        upnplib_shared
)
add_test(NAME ctest_UpnpFileInfo-psh COMMAND test_UpnpFileInfo-psh --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

add_executable(test_UpnpFileInfo-csh
#-----------------------------------
    ./test_UpnpFileInfo.cpp
)
target_compile_definitions(test_UpnpFileInfo-csh
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)
target_link_libraries(test_UpnpFileInfo-csh
    PRIVATE
        compa_shared
        upnplib_shared
)
add_test(NAME ctest_UpnpFileInfo-csh COMMAND test_UpnpFileInfo-csh --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# upnpapi
#========
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts on Microsoft Windows (warning C4273: inconsistent dll linkage). We
# must use static libraries.
if(FALSE)
# Direct linking with source files currently not used but having available for
# reference. We link against the internal upnplib libraries (see below).
set(UPNP_UPNPAPI_SOURCE_FILES
    ${PUPNP_IXML_SOURCE_DIR}/src/attr.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/document.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/element.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/ixml.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/ixmlmembuf.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/ixmlparser.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/namedNodeMap.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/node.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/nodeList.cpp
    $<$<CONFIG:Debug>:${PUPNP_IXML_SOURCE_DIR}/src/ixmldebug.cpp>

    ${PUPNP_UPNP_SOURCE_DIR}/src/global.cpp
    ./tools/mockObj.cpp
    $<$<BOOL:${WIN32}>:./tools/mockObj_win32.cpp>

    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/FreeList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/LinkedList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/ThreadPool.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/TimerThread.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/sock.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/uri/uri.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/httpparser.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/httpreadwrite.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/statcodes.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/list.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/membuffer.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/strintmap.cpp

    # ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpExtraHeaders.c
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpString.cpp
    $<$<CONFIG:Debug>:${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpdebug.cpp>

    ${UPNPLIB_SOURCE_DIR}/src/api/upnptools.cpp
)
endif()

add_executable(test_upnpapi-pst
#------------------------------
    ./test_upnpapi.cpp
    ${PUPNP_SOURCE_DIR}/src/api/upnpdebug.cpp
)
target_include_directories(test_upnpapi-pst
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_compile_definitions(test_upnpapi-pst
    PRIVATE
        UPNP_HAVE_WEBSERVER
        UPNP_HAVE_CLIENT
        UPNP_HAVE_OPTSSDP
)
target_compile_options(test_upnpapi-pst
    PRIVATE
        # disable warning C4100: 'result_listener': unreferenced formal
        # parameter This is from Googlemock MATCHER_P. Don't know how to fix
        # it. Seems to be a side effect of the macro definition. --Ingo
        $<$<CXX_COMPILER_ID:MSVC>:/wd4100>
        # disable warning C4505: 'UpnpComputeIpv6PrefixLength': unreferenced
        # local function has been removed This isn't used with WIN32 but on
        # other platforms. I won't modify pupnp. --Ingo
        $<$<CXX_COMPILER_ID:MSVC>:/wd4505>
        # disable warning C4065: switch statement contains 'default' but no
        # 'case' labels
        $<$<CXX_COMPILER_ID:MSVC>:/wd4065>
        # disable warning C4706: assignment within conditional expression
        $<$<CXX_COMPILER_ID:MSVC>:/wd4706>
        # disable warning C4701: potentially uninitialized local variable
        # '<variable>' used
        $<$<CXX_COMPILER_ID:MSVC>:/wd4701>
)
target_link_libraries(test_upnpapi-pst
    PRIVATE
        pupnp_static
        upnplib_static
)
add_test(NAME ctest_upnpapi-pst COMMAND test_upnpapi-pst --gtest_shuffle
)

add_executable(test_upnpapi-cst
#------------------------------
    ./test_upnpapi.cpp
    ${PUPNP_SOURCE_DIR}/src/api/upnpdebug.cpp
)
target_include_directories(test_upnpapi-cst
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_compile_definitions(test_upnpapi-cst
    PRIVATE
        UPNP_HAVE_WEBSERVER
        UPNP_HAVE_CLIENT
        UPNP_HAVE_OPTSSDP
)
target_compile_options(test_upnpapi-cst
    PRIVATE
        # disable warning C4100: 'result_listener': unreferenced formal parameter
        # This is from Googlemock MATCHER_P. Don't know how to fix it. Seems to
        # be a side effect of the macro definition. --Ingo
        $<$<CXX_COMPILER_ID:MSVC>:/wd4100>
)
target_link_libraries(test_upnpapi-cst
    PRIVATE
        compa_static
        upnplib_static
)
add_test(NAME ctest_upnpapi-cst COMMAND test_upnpapi-cst --gtest_shuffle
)

if(FALSE)
add_executable(test_upnpapi_platform-pst
#---------------------------------------
    $<$<PLATFORM_ID:Windows>:./test_upnpapi_win32.cpp>
    $<$<NOT:$<PLATFORM_ID:Windows>>:./test_upnpapi_unix.cpp>
    ${PUPNP_SOURCE_DIR}/src/api/upnpdebug.cpp
)
target_include_directories(test_upnpapi_platform-pst
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_compile_options(test_upnpapi_platform-pst
    PRIVATE
        # disable warning C4505: 'GetFreeHandle': unreferenced local function
        # has been removed This isn't used with WIN32 but on other platforms. I
        # won't modify pupnp code. --Ingo
        $<$<CXX_COMPILER_ID:MSVC>:/wd4505>
)
target_link_libraries(test_upnpapi_platform-pst
    PRIVATE
        pupnp_static
        upnplib_static
        upnplib_gtest_tools_static
)
add_test(NAME ctest_upnpapi_platform-pst COMMAND test_upnpapi_platform-pst --gtest_shuffle
)

add_executable(test_upnpapi_platform-cst
#---------------------------------------
    $<$<PLATFORM_ID:Windows>:./test_upnpapi_win32.cpp>
    $<$<NOT:$<PLATFORM_ID:Windows>>:./test_upnpapi_unix.cpp>
    ${PUPNP_SOURCE_DIR}/src/api/upnpdebug.cpp
)
target_include_directories(test_upnpapi_platform-cst
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_link_libraries(test_upnpapi_platform-cst
    PRIVATE
        compa_static
        upnplib_static
        upnplib_gtest_tools_static
)
add_test(NAME ctest_upnpapi_platform-cst COMMAND test_upnpapi_platform-cst --gtest_shuffle
)
endif()
