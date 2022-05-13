# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-05-13

cmake_minimum_required(VERSION 3.18)
include(../../cmake/project-header.cmake)

project(PUPNP_IXML VERSION 0004
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

# ixml shared library
#--------------------
add_library (ixml_shared SHARED
        ${IXML_SOURCES}
)
set_target_properties(ixml_shared PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/bin"
)
target_compile_definitions(ixml_shared
        PRIVATE UPNPLIB_SHARED
        PRIVATE UPNPLIB_EXPORTS
        PRIVATE $<$<BOOL:${script_support}>:IXML_HAVE_SCRIPTSUPPORT>
        PRIVATE $<$<CONFIG:Debug>:DEBUG>
        PRIVATE $<$<BOOL:${MSVC}>:UPNP_USE_MSVCPP>
)

# ixml static library
#--------------------
add_library (ixml_static STATIC
    ${IXML_SOURCES}
)
set_target_properties(ixml_static PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${UPnPlib_BINARY_DIR}/bin"
)
target_compile_definitions(ixml_static
        PRIVATE $<$<BOOL:${script_support}>:IXML_HAVE_SCRIPTSUPPORT>
        PRIVATE $<$<CONFIG:Debug>:DEBUG>
        PRIVATE $<$<BOOL:${MSVC}>:UPNP_USE_MSVCPP>
)
