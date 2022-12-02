# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-12-03

cmake_minimum_required(VERSION 3.18)
include(../../cmake/project-header.cmake)

project(PUPNP_IXML VERSION 0008
                   DESCRIPTION "IXML library"
                   HOMEPAGE_URL "https://github.com/upnplib")

set (IXML_SOURCES
    ./src/attr.cpp
    ./src/document.cpp
    ./src/element.cpp
    ./src/ixml.cpp
    $<$<CONFIG:Debug>:./src/ixmldebug.cpp>
    ./src/ixmlmembuf.cpp
    ./src/ixmlparser.cpp
    ./src/namedNodeMap.cpp
    ./src/node.cpp
    ./src/nodeList.cpp
)

set (IXML_INCLUDE_DIRECTORIES
    PRIVATE
        ./inc
        ./src/inc
        # Next variable is needed for one step build. Don't use
        # ${PUPNP_UPNP_SOURCE_DIR}/inc
        ${PUPNP_SOURCE_DIR}/upnp/inc
)

# ixml shared library
#--------------------
add_library (ixml_shared SHARED
        ${IXML_SOURCES}
)
target_include_directories(ixml_shared
    ${IXML_INCLUDE_DIRECTORIES}
)
target_compile_definitions(ixml_shared
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_EXPORTS
        $<$<BOOL:${script_support}>:IXML_HAVE_SCRIPTSUPPORT>
        $<$<CONFIG:Debug>:DEBUG>
)

# ixml static library
#--------------------
add_library (ixml_static STATIC
        ${IXML_SOURCES}
)
target_include_directories(ixml_static
    ${IXML_INCLUDE_DIRECTORIES}
)
target_compile_definitions(ixml_static
    PRIVATE
        $<$<BOOL:${script_support}>:IXML_HAVE_SCRIPTSUPPORT>
        $<$<CONFIG:Debug>:DEBUG>
)

#if (BUILD_TESTING)
#    add_subdirectory(./test)
#endif()
