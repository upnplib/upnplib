# Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2024-08-15

cmake_minimum_required(VERSION 3.23) # for FILE_SET
include(../../cmake/project-header.cmake)

project(PUPNP_IXML VERSION 0008
                   DESCRIPTION "IXML library"
                   HOMEPAGE_URL "https://github.com/upnplib")

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

set(IXML_PUBLIC_HEADERS
    inc/ixml.h
    inc/ixml.hpp
    inc/posix_overwrites.h
    inc/posix_overwrites.hpp
)

set (IXML_INCLUDE_DIRECTORIES
    PUBLIC inc
    PUBLIC src/inc
    # Next variable is needed for one step configure. Don't use
    # PRIVATE ${PUPNP_UPNP_SOURCE_DIR}/inc
    PRIVATE ${PUPNP_SOURCE_DIR}/upnp/inc
)

# ixml shared library
#--------------------
add_library (ixml_shared SHARED
        ${IXML_SOURCES}
)
set_target_properties(ixml_shared PROPERTIES
        OUTPUT_NAME ixml
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
target_link_libraries(ixml_shared
    PRIVATE umock_shared
)
target_sources(ixml_shared
    INTERFACE FILE_SET ixml_public_headers TYPE HEADERS
              BASE_DIRS inc
              FILES ${IXML_PUBLIC_HEADERS}
)
install(TARGETS ixml_shared
    FILE_SET ixml_public_headers
    DESTINATION include/upnp
)

# ixml static library
#--------------------
add_library (ixml_static STATIC
        ${IXML_SOURCES}
)
set_target_properties(ixml_static PROPERTIES
        OUTPUT_NAME $<IF:$<PLATFORM_ID:Windows>,ixml-static,ixml>
)
target_include_directories(ixml_static
    ${IXML_INCLUDE_DIRECTORIES}
)
target_compile_definitions(ixml_static
    PRIVATE
        $<$<BOOL:${script_support}>:IXML_HAVE_SCRIPTSUPPORT>
        $<$<CONFIG:Debug>:DEBUG>
)
target_link_libraries(ixml_static
    PRIVATE umock_static
)

install(TARGETS ixml_shared ixml_static)

#if (BUILD_TESTING)
#    add_subdirectory(./test)
#endif()
