# Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2023-09-27

cmake_minimum_required(VERSION 3.18)
include(../../cmake/project-header.cmake)

project(GTESTS_SAMPLE VERSION 0006
        DESCRIPTION "Tests for upnplib sample programs"
        HOMEPAGE_URL "https://github.com/upnplib")

if(FALSE)
# Currently not used but having available for reference.
# We link against the upnplib library (see below).
set(UPNPLIB_TV_DEVICE_SOURCE_FILES
    ${PUPNP_IXML_SOURCE_DIR}/src/node.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/document.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/nodeList.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/element.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/ixml.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/attr.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/ixmlparser.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/namedNodeMap.cpp
    ${PUPNP_IXML_SOURCE_DIR}/src/ixmlmembuf.cpp
    $<$<CONFIG:Debug>:${PUPNP_IXML_SOURCE_DIR}/src/ixmldebug.cpp>

    ${PUPNP_UPNP_SOURCE_DIR}/src/api/upnptools.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpEventSubscribe.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpString.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpDiscovery.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpSubscriptionRequest.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpEvent.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpActionRequest.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpActionComplete.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpStateVarRequest.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpStateVarComplete.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpapi.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/UpnpFileInfo.cpp
    $<$<CONFIG:Debug>:${PUPNP_UPNP_SOURCE_DIR}/src/api/upnpdebug.cpp>

    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/sock.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/httpreadwrite.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/httpparser.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/statcodes.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/webserver.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/http/parsetools.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/net/uri/uri.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/miniserver/miniserver.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/service-table/service_table.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/client_table/GenlibClientSubscription.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/client_table/client_table.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/membuffer.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/list.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/upnp_timeout.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/strintmap.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/genlib/util/util.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/ssdp/ssdp_server.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/ssdp/ssdp_ctrlpt.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/ssdp/ssdp_device.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/ssdp/SSDPResultDataCallback.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/ssdp/SSDPResultData.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/gena/gena_ctrlpt.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/gena/gena_device.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/gena/gena_callback2.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/soap/soap_ctrlpt.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/soap/soap_device.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/soap/soap_common.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/urlconfig/urlconfig.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/uuid/uuid.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/uuid/sysdep.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/uuid/md5.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/LinkedList.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/TimerThread.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/ThreadPool.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/threadutil/FreeList.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/global.cpp
    ${GTESTS_SOURCE_DIR}/tools/mockObj.cpp
    $<$<BOOL:${WIN32}>:${GTESTS_SOURCE_DIR}/tools/mockObj_win32.cpp>
)
endif()


if(NOT WIN32)
# test tv_device build with pupnp shared library
#-----------------------------------------------
add_executable(test_tv_device-psh
    ${PUPNP_UPNP_SOURCE_DIR}/src/api/upnptools.cpp
    ${SAMPLE_SOURCE_DIR}/common/sample_util.cpp
    ./test_tv_device.cpp
)
target_include_directories(test_tv_device-psh
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_compile_options(test_tv_device-psh
    PRIVATE
        # disable warning C4706: assignment within conditional expression.
        # This is intended but bad practice. I won't modify pupnp code. --Ingo
        $<$<CXX_COMPILER_ID:MSVC>:/wd4706>
)
target_link_libraries(test_tv_device-psh
    PRIVATE
        pupnp_shared
        upnplib_shared
        utest_shared
)
add_test(NAME ctest_tv_device-psh COMMAND test_tv_device-psh --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)

# test tv_device build with compatible shared library
#----------------------------------------------------
add_executable(test_tv_device-csh
    ${COMPA_SOURCE_DIR}/src/api/upnptools.cpp
    ${SAMPLE_SOURCE_DIR}/common/sample_util.cpp
    ./test_tv_device.cpp
)
target_include_directories(test_tv_device-csh
    PRIVATE ${UPNPLIB_PROJECT_SOURCE_DIR}
)
target_link_libraries(test_tv_device-csh
    PRIVATE
        compa_shared
        upnplib_shared
        utest_shared
)
add_test(NAME ctest_tv_device-csh COMMAND test_tv_device-csh --gtest_shuffle
        WORKING_DIRECTORY ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}
)
endif()
