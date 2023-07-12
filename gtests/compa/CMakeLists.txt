# Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2023-07-12

cmake_minimum_required(VERSION 3.18)
include(../../cmake/project-header.cmake)

project(GTESTS_COMPA VERSION 0010
                  DESCRIPTION "Tests for pupnp and compatible code"
                  HOMEPAGE_URL "https://github.com/upnplib")


# sock
#=====
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
            upnplib_gtest_tools_shared
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
            upnplib_gtest_tools_shared
    )
    add_test(NAME ctest_sock_openssl-csh COMMAND test_sock_openssl-csh --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
    )


# openssl_threaded
#=================
    add_executable(test_openssl_threaded-psh
        ./test_openssl_threaded.cpp
    )
    target_include_directories(test_openssl_threaded-psh
        PRIVATE ${UPNPLIB_PROJECT_BINARY_DIR}/include
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
    target_include_directories(test_openssl_threaded-csh
        PRIVATE ${UPNPLIB_PROJECT_BINARY_DIR}/include
    )
    target_link_libraries(test_openssl_threaded-csh
        PRIVATE
            compa_shared
    )
    #add_test(NAME ctest_openssl_threaded-csh COMMAND test_openssl_threaded-csh --gtest_shuffle
    #    WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
    #)

endif()


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
    ./test_miniserver.cpp
    ${COMPA_SOURCE_DIR}/src/api/upnpdebug.cpp
)
target_include_directories(test_miniserver-pst
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
    PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/src/inc
    PRIVATE ${COMPA_SOURCE_DIR}/include
)
target_compile_definitions(test_miniserver-pst
    PRIVATE
        UPNP_HAVE_CLIENT
        UPNP_HAVE_DEVICE
        UPNP_HAVE_WEBSERVER
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)
target_compile_options(test_miniserver-pst
    PRIVATE
        # disable warning C4100: 'result_listener': unreferenced formal parameter
        # This is from Googlemock MATCHER_P. Don't know how to fix it. Seems to
        # be a side effect of the macro definition. --Ingo
        $<$<CXX_COMPILER_ID:MSVC>:/wd4100>
)
target_link_libraries(test_miniserver-pst
    PRIVATE
        pupnp_static
        upnplib_static
        upnplib_gtest_tools_static
)
add_test(NAME ctest_miniserver-pst COMMAND test_miniserver-pst --gtest_shuffle
)

add_executable(test_miniserver-cst
#---------------------------------
    ./test_miniserver.cpp
)
target_include_directories(test_miniserver-cst
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_compile_definitions(test_miniserver-cst
    PRIVATE
        UPNP_HAVE_CLIENT
        UPNP_HAVE_DEVICE
        UPNP_HAVE_WEBSERVER
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)
target_compile_options(test_miniserver-cst
    PRIVATE
        # disable warning C4100: 'result_listener': unreferenced formal parameter
        # This is from Googlemock MATCHER_P. Don't know how to fix it. Seems to
        # be a side effect of the macro definition. --Ingo
        $<$<CXX_COMPILER_ID:MSVC>:/wd4100>
)
target_link_libraries(test_miniserver-cst
    PRIVATE
        compa_static
        upnplib_static
        upnplib_gtest_tools_static
)
add_test(NAME ctest_miniserver-cst COMMAND test_miniserver-cst --gtest_shuffle
)


add_subdirectory(./api.d)
add_subdirectory(./http.d)
add_subdirectory(./ssdp.d)
add_subdirectory(./threadutil.d)
add_subdirectory(./uri.d)
add_subdirectory(./util.d)
