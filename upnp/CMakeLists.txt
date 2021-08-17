# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2021-08-18

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(UPNP_CORE DESCRIPTION "Binary build of the UPnP library core")

# This contains all settings to compile the library dynamically and static.
#==========================================================================

# Compiler options
#-----------------

if (MSVC)
    # TODO: Increase error levels on compilers
    # Reports the compiler warning that is specified by nnnn only once.
    #add_compile_options(/wo4273)
    # suppress all warnings for development to see only errors
    #add_compile_options(/W0)
    # warning level 4 and all warnings as errors
    #add_compile_options(/W4 /WX)
else()
    # lots of warnings and all warnings as errors
    #add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()


# Source files and compile settings
#----------------------------------
# TODO: Remove references to '${CMAKE_SOURCE_DIR}/ixml' if possible

include_directories(
    ${PROJECT_SOURCE_DIR}/inc
    ${PROJECT_SOURCE_DIR}/src/inc
    ${PROJECT_SOURCE_DIR}/src/threadutil
    ${CMAKE_SOURCE_DIR}/ixml/inc
    ${CMAKE_SOURCE_DIR}/ixml/src/inc
    ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/upnp/inc
    ${PTHREADS4W_BINARY_DIR}/include
)

set(UPNP_SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/ixml/src/attr.c
    ${CMAKE_SOURCE_DIR}/ixml/src/document.c
    ${CMAKE_SOURCE_DIR}/ixml/src/element.c
    ${CMAKE_SOURCE_DIR}/ixml/src/ixml.c
    ${CMAKE_SOURCE_DIR}/ixml/src/ixmlmembuf.c
    ${CMAKE_SOURCE_DIR}/ixml/src/ixmlparser.c
    ${CMAKE_SOURCE_DIR}/ixml/src/namedNodeMap.c
    ${CMAKE_SOURCE_DIR}/ixml/src/node.c
    ${CMAKE_SOURCE_DIR}/ixml/src/nodeList.c

    ${PROJECT_SOURCE_DIR}/src/threadutil/FreeList.cpp
    ${PROJECT_SOURCE_DIR}/src/threadutil/LinkedList.cpp
    ${PROJECT_SOURCE_DIR}/src/threadutil/ThreadPool.cpp
    ${PROJECT_SOURCE_DIR}/src/threadutil/TimerThread.cpp

    ${PROJECT_SOURCE_DIR}/src/genlib/net/sock.c

    ${PROJECT_SOURCE_DIR}/src/genlib/net/uri/uri.c

    ${PROJECT_SOURCE_DIR}/src/genlib/net/http/httpparser.c
    ${PROJECT_SOURCE_DIR}/src/genlib/net/http/httpreadwrite.c
    ${PROJECT_SOURCE_DIR}/src/genlib/net/http/statcodes.c

    ${PROJECT_SOURCE_DIR}/src/genlib/util/list.c
    ${PROJECT_SOURCE_DIR}/src/genlib/util/membuffer.c
    ${PROJECT_SOURCE_DIR}/src/genlib/util/strintmap.c

    ${PROJECT_SOURCE_DIR}/src/api/UpnpExtraHeaders.c
    ${PROJECT_SOURCE_DIR}/src/api/UpnpString.c
    ${PROJECT_SOURCE_DIR}/src/api/upnpapi.cpp
    ${PROJECT_SOURCE_DIR}/src/api/upnpdebug.cpp

    ${PROJECT_SOURCE_DIR}/src/upnplib.cpp
)

# Create test executable for information and create libraries
#------------------------------------------------------------
# All builds should have the same target_compile_definitions

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS YES CACHE BOOL "Export all symbols")

# Executable for information
add_executable(upnplib ${UPNP_SOURCE_FILES}
)
target_compile_definitions (upnplib
        #PRIVATE $<$<BOOL:${script_support}>:IXML_HAVE_SCRIPTSUPPORT>
        #PUBLIC $<IF:$<CONFIG:Debug>,DEBUG,NDEBUG>
        PUBLIC $<$<BOOL:${MSVC}>:UPNP_USE_MSVCPP>
        PUBLIC $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>
)

# Shared library
add_library(upnplibShared SHARED ${UPNP_SOURCE_FILES}
)
target_compile_definitions (upnplibShared
        PUBLIC $<$<BOOL:${MSVC}>:UPNP_USE_MSVCPP>
        PUBLIC $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>
)
set_target_properties(upnplibShared PROPERTIES
        OUTPUT_NAME upnplib
        LIBRARY_OUTPUT_DIRECTORY lib     # used if not WIN32
#        RUNTIME_OUTPUT_DIRECTORY lib     # used if WIN32
)

# Static library
add_library(upnplibStatic STATIC ${UPNP_SOURCE_FILES}
)
target_compile_definitions (upnplibStatic
        PUBLIC $<$<BOOL:${MSVC}>:UPNP_USE_MSVCPP>
        PUBLIC $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>
)

if(NOT WIN32)
    # If not on MS Windows we have another output directory for the library
    set_target_properties(upnplibStatic PROPERTIES
            OUTPUT_NAME upnplib
            ARCHIVE_OUTPUT_DIRECTORY lib
    )
    # In order to compile pthreads with C++ we need its library,
    # but not on MS Windows
    target_link_libraries(upnplib ${UPNP_PTHREADS_SHARED_LIBRARY})
    # TODO: Check why libraries does not need pthread
#    target_link_libraries(upnplibShared pthread)
#    target_link_libraries(upnplibStatic pthread)

else()

    # Link POSIX Threads for Windows (pthreads4w)
    #--------------------------------------------
    # Pthreads are not supported by MS Windows. So we have installed the
    # third party library pthreads4w (see subproject pthreads4w if installed).

    set_target_properties(upnplibStatic PROPERTIES
            OUTPUT_NAME upnplib-a
    )
    # In addition to pthreads we need some more system libraries
    # ws2_32: winsock to support sockets
    # iphlpapi: ip helper interface to get network management tools
    # I got warning LNK4098: defaultlib 'LIBCMT' conflicts with use of other
    # libs; use /NODEFAULTLIB:library
    set(UPNP_LINK_OPTIONS ws2_32 iphlpapi -NODEFAULTLIB:LIBCMT
                          ${PTHREADS4W_BINARY_DIR}/lib/libpthreadVC3.lib)
    target_link_libraries(upnplib ${UPNP_LINK_OPTIONS})
    target_link_libraries(upnplibShared ${UPNP_LINK_OPTIONS})
    target_link_libraries(upnplibStatic ${UPNP_LINK_OPTIONS})
endif()


# Installation will be done with packaging
#-----------------------------------------
#set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/shared)
#install(
#    TARGETS upnplib
#    LIBRARY DESTINATION lib
#    ARCHIVE DESTINATION lib
#    RUNTIME DESTINATION bin
#    PUBLIC_HEADER DESTINATION ${PROJECT_BINARY_DIR}/include
#)
