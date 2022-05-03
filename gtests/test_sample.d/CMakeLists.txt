# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-05-05

cmake_minimum_required(VERSION 3.18)
include(../../cmake/project-header.cmake)

project(GTESTS_SAMPLE VERSION 0001
                  DESCRIPTION "Tests for upnplib sample programs"
                  HOMEPAGE_URL "https://github.com/upnplib")


add_executable(test_tv_device
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

    ${UPNPLIB_SAMPLE_SOURCE_DIR}/common/sample_util.cpp

    ${PUPNP_UPNP_SOURCE_DIR}/src/global.cpp
    ${PUPNP_UPNP_SOURCE_DIR}/src/mockObj.cpp
    $<$<BOOL:${WIN32}>:${PUPNP_UPNP_SOURCE_DIR}/src/mockObj_win32.cpp>
    $<$<NOT:$<BOOL:${WIN32}>>:${PUPNP_UPNP_SOURCE_DIR}/src/mockObj_unix.cpp>

    ./test_tv_device.cpp
)

target_compile_definitions (test_tv_device PRIVATE
    UPNP_HAVE_TOOLS
    UPNP_HAVE_CLIENT
    UPNP_HAVE_WEBSERVER
    UPNP_HAVE_SSDP
    UPNP_HAVE_SOAP
    UPNP_HAVE_GENA
    INCLUDE_DEVICE_APIS
    $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
    # There are traditional min/max macros on MS Windows which conflict
    # with the C++ std::min|max functions. Following switch disable the
    # macros.
    $<$<CXX_COMPILER_ID:MSVC>:NOMINMAX>
)

add_test(NAME ctest_tv_device COMMAND test_tv_device)
#set_tests_properties( ctest_tv_device PROPERTIES
#    ENVIRONMENT "PATH=$ENV{PATH};${UPnPlib_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}" )
