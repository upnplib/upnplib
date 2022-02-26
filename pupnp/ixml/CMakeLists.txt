# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-02-26

cmake_minimum_required(VERSION 3.18)
include(../../cmake/project-header.cmake)

project(PUPNP_IXML VERSION 0003
                   DESCRIPTION "IXML library"
                   HOMEPAGE_URL "https://github.com/upnplib")

#if (BUILD_TESTING)
    add_subdirectory (test)
#endif()

include_directories (
    inc
    src/inc
    ${PUPNP_SOURCE_DIR}/upnp/inc
    ${UPnPlib_BINARY_DIR}/upnp/inc
)

set (IXML_SOURCES
    src/attr.cpp
    src/document.cpp
    src/element.cpp
    src/ixml.cpp
    $<$<CONFIG:Debug>:src/ixmldebug.cpp>
    src/ixmlmembuf.cpp
    src/ixmlparser.cpp
    src/namedNodeMap.cpp
    src/node.cpp
    src/nodeList.cpp
)

add_library (ixml_shared SHARED
    ${IXML_SOURCES}
)

target_compile_definitions (ixml_shared PRIVATE
    $<$<BOOL:${script_support}>:IXML_HAVE_SCRIPTSUPPORT>
    $<$<CONFIG:Debug>:DEBUG>
    #$<IF:$<CONFIG:Debug>,DEBUG,NDEBUG>
    $<$<BOOL:${MSVC}>:UPNP_USE_MSVCPP>
    $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>
)

add_library (ixml_static STATIC
    ${IXML_SOURCES}
)
target_compile_definitions (ixml_static PRIVATE
    $<$<BOOL:${script_support}>:IXML_HAVE_SCRIPTSUPPORT>
    $<$<CONFIG:Debug>:DEBUG>
    #$<IF:$<CONFIG:Debug>,DEBUG,NDEBUG>
    $<$<BOOL:${MSVC}>:UPNP_USE_MSVCPP>
    UPNP_STATIC_LIB
)
