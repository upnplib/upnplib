# Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-12-01

cmake_minimum_required(VERSION 3.18)
include(../../../cmake/project-header.cmake)

project(GTESTS_COMPA_API VERSION 0001
                  DESCRIPTION "Tests for api module and compatible code"
                  HOMEPAGE_URL "https://github.com/upnplib")


# upnpdebug
#==========
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts. We must use static libraries.
add_executable(test_upnpdebug-pst
        ./test_upnpdebug.cpp
)
target_include_directories(test_upnpdebug-pst
    PRIVATE
        ${GTESTS_SOURCE_DIR}/include
        ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_compile_definitions(test_upnpdebug-pst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
)
target_compile_options(test_upnpdebug-pst
    PRIVATE
        # disable warning C4459: declaration of 'fp' hides global declaration
        $<$<CXX_COMPILER_ID:MSVC>:/wd4459>
        # disable warning C4211: nonstandard extension used: redefined extern to static
        $<$<CXX_COMPILER_ID:MSVC>:/wd4211>
        # disable warning C4100: 'result_listener': unreferenced formal parameter
        $<$<CXX_COMPILER_ID:MSVC>:/wd4100>
)
target_link_libraries(test_upnpdebug-pst
    PRIVATE
        pupnp_static
        upnplib_static
        upnplib_gtest_tools_static
)
add_test(NAME ctest_upnpdebug-pst COMMAND test_upnpdebug-pst
)


# upnptools
#==========
if(UPNPLIB_WITH_TOOLS)
add_executable(test_upnptools-psh
#--------------------------------
        ./test_upnptools.cpp
)
target_compile_definitions(test_upnptools-psh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
)
target_link_libraries(test_upnptools-psh
    PRIVATE
        pupnp_shared
)
add_test(NAME ctest_upnptools-psh COMMAND test_upnptools-psh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)
endif()


# UpnpString
#===========
add_executable(test_UpnpString-psh
#---------------------------------
        ./test_UpnpString.cpp
)
target_include_directories(test_UpnpString-psh
    PRIVATE
        ${GTESTS_SOURCE_DIR}/include
)
target_compile_definitions(test_UpnpString-psh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)
target_compile_options(test_UpnpString-psh
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W1>
)
target_link_libraries(test_UpnpString-psh
    PRIVATE
        pupnp_shared
        upnplib_shared
)
add_test(NAME ctest_UpnpString-psh COMMAND test_UpnpString-psh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

add_executable(test_UpnpString-csh
#---------------------------------
        ./test_UpnpString.cpp
)
target_include_directories(test_UpnpString-csh
    PRIVATE
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
        ${GTESTS_SOURCE_DIR}/include
        ${COMPA_SOURCE_DIR}/inc
        ${PUPNP_UPNP_SOURCE_DIR}/inc
)
target_compile_definitions(test_UpnpString-csh
    PRIVATE
        UPNPLIB_SHARED
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)
target_compile_options(test_UpnpString-csh
    PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W1>
)
target_link_libraries(test_UpnpString-csh
    PRIVATE
        compa_shared
        upnplib_shared
)
add_test(NAME ctest_UpnpString-csh COMMAND test_UpnpString-csh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# UpnpFileInfo
#=============
add_executable(test_UpnpFileInfo-psh
#-----------------------------------
    ./test_UpnpFileInfo.cpp
)
target_include_directories(test_UpnpFileInfo-psh
    PRIVATE
        ${GTESTS_SOURCE_DIR}/include
)
target_compile_definitions(test_UpnpFileInfo-psh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)
target_link_libraries(test_UpnpFileInfo-psh
    PRIVATE
        pupnp_shared
        upnplib_shared
)
add_test(NAME ctest_UpnpFileInfo-psh COMMAND test_UpnpFileInfo-psh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

add_executable(test_UpnpFileInfo-csh
#-----------------------------------
    ./test_UpnpFileInfo.cpp
)
target_include_directories(test_UpnpFileInfo-csh
    PRIVATE
        ${GTESTS_SOURCE_DIR}/include
        ${UPNPLIB_PROJECT_SOURCE_DIR}/include
        ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_IXML_SOURCE_DIR}/inc
        ${COMPA_SOURCE_DIR}/inc
)
target_compile_definitions(test_UpnpFileInfo-csh
    PRIVATE
        UPNPLIB_SHARED
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)
target_link_libraries(test_UpnpFileInfo-csh
    PRIVATE
        compa_shared
        upnplib_shared
)
add_test(NAME ctest_UpnpFileInfo-csh COMMAND test_UpnpFileInfo-csh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# upnpapi
#========
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts. We must use static libraries.
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
)
target_include_directories(test_upnpapi-pst
    PRIVATE
        ${GTESTS_SOURCE_DIR}/include
        ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_compile_definitions(test_upnpapi-pst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
        UPNP_HAVE_WEBSERVER
        UPNP_HAVE_CLIENT
        UPNP_HAVE_SSDP
        UPNP_HAVE_OPTSSDP
        UPNP_HAVE_GENA
)
target_compile_options(test_upnpapi-pst
    PRIVATE
        # disable warning C4065: switch statement contains 'default' but no 'case' labels
        $<$<CXX_COMPILER_ID:MSVC>:/wd4065>
        # disable warning C4100: 'result_listener': unreferenced formal parameter
        $<$<CXX_COMPILER_ID:MSVC>:/wd4100>
        # disable warning C4505: 'UpnpComputeIpv6PrefixLength': unreferenced local function has been removed
        $<$<CXX_COMPILER_ID:MSVC>:/wd4505>
)
target_link_libraries(test_upnpapi-pst
    PRIVATE
        pupnp_static
        upnplib_static
)
add_test(NAME ctest_upnpapi-pst COMMAND test_upnpapi-pst
)

add_executable(test_upnpapi-cst
#------------------------------
    ./test_upnpapi.cpp
)
target_include_directories(test_upnpapi-cst
    PRIVATE
        ${UPNPLIB_PROJECT_SOURCE_DIR}
        ${GTESTS_SOURCE_DIR}/include
)
target_compile_definitions(test_upnpapi-cst
    PRIVATE
        UPNP_HAVE_WEBSERVER
        UPNP_HAVE_CLIENT
        UPNP_HAVE_OPTSSDP
)
target_compile_options(test_upnpapi-cst
    PRIVATE
        # disable warning C4065: switch statement contains 'default' but no 'case' labels
        $<$<CXX_COMPILER_ID:MSVC>:/wd4065>
        # disable warning C4100: 'result_listener': unreferenced formal parameter
        $<$<CXX_COMPILER_ID:MSVC>:/wd4100>
        # disable warning C4505: 'UpnpComputeIpv6PrefixLength': unreferenced local function has been removed
        $<$<CXX_COMPILER_ID:MSVC>:/wd4505>
)
target_link_libraries(test_upnpapi-cst
    PRIVATE
        compa_static
        upnplib_static
)
add_test(NAME ctest_upnpapi-cst COMMAND test_upnpapi-cst
)

add_executable(test_upnpapi_platform-psh
#---------------------------------------
        $<$<PLATFORM_ID:Windows>:./test_upnpapi_win32.cpp>
        $<$<NOT:$<PLATFORM_ID:Windows>>:./test_upnpapi_unix.cpp>
)
target_include_directories(test_upnpapi_platform-psh
    PRIVATE
        ${GTESTS_SOURCE_DIR}/include
        ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_compile_definitions(test_upnpapi_platform-psh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)
target_compile_options(test_upnpapi_platform-psh
    PRIVATE
        # disable warning C4251: 'upnplib::CNetIf4::m_Description': class 'std::basic_string<wchar_t,std::char_traits<wchar_t>,std::allocator<wchar_t>>' needs to have dll-interface to be used by cli nts of class 'upnplib::CNetIf4'
        $<$<CXX_COMPILER_ID:MSVC>:/wd4251>
        # disable warning C4244: '=': conversion from 'const _Ty' to '_Ty', possible loss of data
        $<$<CXX_COMPILER_ID:MSVC>:/wd4244>
        # disable warning C4389: '==': signed/unsigned mismatch
        $<$<CXX_COMPILER_ID:MSVC>:/wd4389>
)
target_link_libraries(test_upnpapi_platform-psh
    PRIVATE
        upnplib_gtest_tools_shared
        pupnp_shared
        upnplib_shared
)
add_test(NAME ctest_upnpapi_platform-psh COMMAND test_upnpapi_platform-psh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)
