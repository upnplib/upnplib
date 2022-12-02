# Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-12-03

cmake_minimum_required(VERSION 3.18)
include(../../cmake/project-header.cmake)

project(GTESTS_COMPA VERSION 0009
                  DESCRIPTION "Tests for pupnp and compatible code"
                  HOMEPAGE_URL "https://github.com/upnplib")


# sock
#=====
add_executable(test_sock-psh
        ./test_sock.cpp
)
target_include_directories(test_sock-psh
    PRIVATE
        ${GTESTS_SOURCE_DIR}/include
)
target_compile_definitions(test_sock-psh
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_WITH_NATIVE_PUPNP
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)
target_compile_options(test_sock-psh
    PRIVATE
        # disable warning C4211: nonstandard extension used: redefined extern to static
        $<$<CXX_COMPILER_ID:MSVC>:/wd4211>
)
target_link_libraries(test_sock-psh
    PRIVATE
        pupnp_shared
        upnplib_shared
)
add_test(NAME ctest_sock-psh COMMAND test_sock-psh
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)


# miniserver
#===========
# Because we want to include the source file into the test to also test static
# functions, we cannot use shared libraries due to symbol import/export
# conflicts. We must use static libraries.
if(FALSE)
# Direct linking with source files currently not used but having available for
# reference. We link against the internal upnplib libraries (see below).
set(MINISERVER_SOURCE_FILES
#--------------------------
    ./test_miniserver.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/global.cpp
    ./tools/mockObj.cpp
    $<$<PLATFORM_ID:Windows>:./tools/mockObj_win32.cpp>

    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/FreeList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/LinkedList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/ThreadPool.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/TimerThread.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/sock.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/uri/uri.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/httpparser.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/httpreadwrite.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/statcodes.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/parsetools.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/webserver.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/list.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/membuffer.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/strintmap.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/ssdp/ssdp_server.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/ssdp/ssdp_ctrlpt.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/ssdp/SSDPResultData.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/ssdp/SSDPResultDataCallback.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/soap/soap_common.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/soap/soap_ctrlpt.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/uuid/uuid.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/uuid/sysdep.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/uuid/md5.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpExtraHeaders.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpString.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpFileInfo.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpDiscovery.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpActionComplete.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpStateVarComplete.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpapi.cpp
#    ${UPNPLIB_SOURCE_DIR}/src/api/upnptools.cpp
    $<$<CONFIG:Debug>:${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpdebug.cpp>
)
endif()

add_executable(test_miniserver-pst
#---------------------------------
# Needed to also compile additional sources instead of using that from the
# internal library. Otherwise the compiler complains 'multiple definition' of
# several symbols. This is because the compiler find symbols two times, one
# time with the included source file and one time from the library. If we use the
# source file here, the compiler doesn't look at the library.
    ./test_miniserver.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/ssdp/ssdp_server.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpapi.cpp
    $<$<BOOL:${UPNPLIB_WITH_IPV6}>:${PUPNP_UPNP_SOURCE_DIR}/src/ssdp/ssdp_ctrlpt.cpp>
)
target_include_directories(test_miniserver-pst
    PRIVATE
        ${UPNPLIB_PROJECT_SOURCE_DIR}
        ${GTESTS_SOURCE_DIR}/include
)
target_compile_definitions(test_miniserver-pst
    PRIVATE
        UPNPLIB_WITH_NATIVE_PUPNP
        UPNP_HAVE_CLIENT
        UPNP_HAVE_WEBSERVER
        UPNP_HAVE_SSDP
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)
target_compile_options(test_miniserver-pst
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W1>
        # disable warning C4065: switch statement contains 'default' but no 'case' labels
        $<$<CXX_COMPILER_ID:MSVC>:/wd4065>
        # disable warning C4505: 'UpnpComputeIpv6PrefixLength': unreferenced local function has been removed
        $<$<CXX_COMPILER_ID:MSVC>:/wd4505>
        # disable warning C4005: '_CRT_SECURE_NO_WARNINGS': macro redefinition
        $<$<CXX_COMPILER_ID:MSVC>:/wd4005>
        # disable warning C4244: 'argument': conversion from 'unsigned __int64' to 'To', possible loss of data
        $<$<CXX_COMPILER_ID:MSVC>:/wd4244>
        # disable warning C4267: 'argument': conversion from 'size_t' to 'const int', possible loss of data
        $<$<CXX_COMPILER_ID:MSVC>:/wd4267>
        # disable warning C4100: 'result_listener': unreferenced formal parameter
        $<$<CXX_COMPILER_ID:MSVC>:/wd4100>
)
target_link_libraries(test_miniserver-pst
    PRIVATE
        pupnp_static
        upnplib_static
        upnplib_gtest_tools_static
)
add_test(NAME ctest_miniserver-pst COMMAND test_miniserver-pst
)

add_executable(test_miniserver-cst
#---------------------------------
# Needed to also compile additional sources instead of using that from the
# internal library. Otherwise the compiler complains 'multiple definition' of
# several symbols. This is because the compiler find symbols two times, one
# time with the included source file and one time from the library. If we use the
# source file here, the compiler doesn't look at the library.
    ./test_miniserver.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/ssdp/ssdp_server.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpapi.cpp
    $<$<BOOL:${UPNPLIB_WITH_IPV6}>:${PUPNP_UPNP_SOURCE_DIR}/src/ssdp/ssdp_ctrlpt.cpp>
)
target_include_directories(test_miniserver-cst
    PRIVATE
        ${UPNPLIB_PROJECT_SOURCE_DIR}
        ${GTESTS_SOURCE_DIR}/include
)
target_compile_definitions(test_miniserver-cst
    PRIVATE
        UPNP_HAVE_CLIENT
        UPNP_HAVE_WEBSERVER
        UPNP_HAVE_SSDP
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)
target_compile_options(test_miniserver-cst
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W1>
        # disable warning C4065: switch statement contains 'default' but no 'case' labels
        $<$<CXX_COMPILER_ID:MSVC>:/wd4065>
        # disable warning C4505: 'UpnpComputeIpv6PrefixLength': unreferenced local function has been removed
        $<$<CXX_COMPILER_ID:MSVC>:/wd4505>
        # disable warning C4005: '_CRT_SECURE_NO_WARNINGS': macro redefinition
        $<$<CXX_COMPILER_ID:MSVC>:/wd4005>
        # disable warning C4244: 'argument': conversion from 'unsigned __int64' to 'To', possible loss of data
        $<$<CXX_COMPILER_ID:MSVC>:/wd4244>
        # disable warning C4267: 'argument': conversion from 'size_t' to 'const int', possible loss of data
        $<$<CXX_COMPILER_ID:MSVC>:/wd4267>
        # disable warning C4100: 'result_listener': unreferenced formal parameter
        $<$<CXX_COMPILER_ID:MSVC>:/wd4100>
)
target_link_libraries(test_miniserver-cst
    PRIVATE
        compa_static
        upnplib_static
        upnplib_gtest_tools_static
)
add_test(NAME ctest_miniserver-cst COMMAND test_miniserver-cst
)


add_subdirectory(./test_api.d)
add_subdirectory(./test_http.d)
add_subdirectory(./test_threadutil.d)
add_subdirectory(./test_uri.d)
add_subdirectory(./test_util.d)
