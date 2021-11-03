# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2021-11-03

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(UPNP_CORE VERSION 0006
        DESCRIPTION "Binary build of the UPnP library core"
        HOMEPAGE_URL "https://github.com/upnplib")

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
    # if pthreads4w isn't installed this path is empty. The linker
    # doesn't find "pthread.h" and falls back to look at <pthread.h>.
    ${pthreads4w_SOURCE_DIR}/
)

set(UPNP_SOURCE_FILES
    ${UPNP_IXML_SOURCE_DIR}/src/attr.c
    ${UPNP_IXML_SOURCE_DIR}/src/document.c
    ${UPNP_IXML_SOURCE_DIR}/src/element.c
    ${UPNP_IXML_SOURCE_DIR}/src/ixml.c
    ${UPNP_IXML_SOURCE_DIR}/src/ixmlmembuf.c
    ${UPNP_IXML_SOURCE_DIR}/src/ixmlparser.c
    ${UPNP_IXML_SOURCE_DIR}/src/namedNodeMap.c
    ${UPNP_IXML_SOURCE_DIR}/src/node.c
    ${UPNP_IXML_SOURCE_DIR}/src/nodeList.c
    $<$<CONFIG:Debug>:${UPNP_IXML_SOURCE_DIR}/src/ixmldebug.c>

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
    ${PROJECT_SOURCE_DIR}/src/api/UpnpString.cpp
    ${PROJECT_SOURCE_DIR}/src/api/upnpapi.cpp
    ${PROJECT_SOURCE_DIR}/src/api/upnpdebug.cpp
    ${PROJECT_SOURCE_DIR}/src/global.cpp
)


# Create libraries and create test executable for information
#============================================================
if(NOT WIN32)

    # Shared library
    #---------------
    add_library(upnplib_shared SHARED ${UPNP_SOURCE_FILES})
    set_target_properties(upnplib_shared PROPERTIES
            OUTPUT_NAME upnplib  # lib*.so will be added
            LIBRARY_OUTPUT_DIRECTORY lib  # used if not WIN32
    #       RUNTIME_OUTPUT_DIRECTORY lib  # used if WIN32
    )

    # Test executable linked with the shared library
    add_executable(upnplib_shared_run ${PROJECT_SOURCE_DIR}/src/upnplib.cpp)
    set_target_properties(upnplib_shared_run PROPERTIES
            OUTPUT_NAME upnplib
    )
    target_link_libraries(upnplib_shared_run
            upnplib_shared
            ${UPNP_PTHREADS_SHARED_LIBRARY}
    )

    # Static library
    #---------------
    add_library(upnplib_static STATIC ${UPNP_SOURCE_FILES})
    set_target_properties(upnplib_static PROPERTIES
            OUTPUT_NAME upnplib  # lib*.a will be added
            ARCHIVE_OUTPUT_DIRECTORY lib
    )

else() # WIN32

    # Can be general initialized with CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS
    # but will have it under control individual
    set(WINDOWS_EXPORT_ALL_SYMBOLS YES)

    # Link with POSIX Threads for Windows (pthreads4w)
    #=================================================
    # Pthreads are not supported by MS Windows. So we have installed the
    # third party library pthreads4w.

    # Shared library
    #---------------
    add_library(upnplib_shared_win SHARED ${UPNP_SOURCE_FILES})
    set_target_properties(upnplib_shared_win PROPERTIES
            OUTPUT_NAME upnplib
    )
    target_compile_definitions(upnplib_shared_win
            #PRIVATE $<$<BOOL:${script_support}>:IXML_HAVE_SCRIPTSUPPORT>
            PUBLIC $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>
    )
    # In addition to pthreads we need some more system libraries here
    # ws2_32: winsock to support sockets
    # iphlpapi: ip helper interface to get network management tools
    target_link_libraries(upnplib_shared_win ws2_32 iphlpapi
            ${UPNP_PTHREADS_STATIC_LIBRARY}
            # If you link with the shared pthreads4w library then its .dll file
            # must also be found.
            #${UPNP_PTHREADS_SHARED_LIBRARY}
    )

    # Test executable linked with the shared library
    add_executable(upnplib_shared_win_exe ${PROJECT_SOURCE_DIR}/src/upnplib.cpp)
    set_target_properties(upnplib_shared_win_exe PROPERTIES
            OUTPUT_NAME upnplib
    )
    target_link_libraries(upnplib_shared_win_exe
            upnplib_shared_win
    )

    # Static library
    #---------------
    add_library(upnplib_static_win STATIC ${UPNP_SOURCE_FILES})
    set_target_properties(upnplib_static_win PROPERTIES
            OUTPUT_NAME upnplibStatic
    )
    target_compile_definitions(upnplib_static_win
            #PRIVATE $<$<BOOL:${script_support}>:IXML_HAVE_SCRIPTSUPPORT>
            PUBLIC $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>
    )
    target_link_libraries(upnplib_static_win
            ${UPNP_PTHREADS_STATIC_LIBRARY}
    )

    # Test executable linked with the static library
    add_executable(upnplib_static_win_exe ${PROJECT_SOURCE_DIR}/src/upnplib.cpp)
    set_target_properties(upnplib_static_win_exe PROPERTIES
            OUTPUT_NAME upnplibStatic
    )
    target_link_libraries(upnplib_static_win_exe
            upnplib_static_win
    )

endif() # WIN32
