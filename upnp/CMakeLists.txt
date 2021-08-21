# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2021-08-20

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(UPNP_CORE VERSION 0001
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

    # Test executable linked with the static library
    # Tried to do it but got warnings and link errors. Only linking with
    # UPNP_PTHREADS_SHARED_LIBRARY helps but it doesn't give a static program.
    # It is the same as the shared linked program.
#    add_executable(upnplib_static_run ${PROJECT_SOURCE_DIR}/src/upnplib.cpp)
#    set_target_properties(upnplib_static_run PROPERTIES
#            OUTPUT_NAME upnplib.a
#    )
#    target_link_libraries(upnplib_static_run
#            upnplib_static
#            #libc.a
#            # linking with _STATIC_... gives errors, don't know why :-(
#            ${UPNP_PTHREADS_STATIC_LIBRARY}
#    )

else() # WIN32

    # Can be general initialized with CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS
    # but will have it under control individual
    set(WINDOWS_EXPORT_ALL_SYMBOLS YES)

    # Link with POSIX Threads for Windows (pthreads4w)
    #=================================================
    # Pthreads are not supported by MS Windows. So we have installed the
    # third party library pthreads4w (see subproject pthreads4w if installed).

    # Shared library
    #---------------
    add_library(upnplib_shared_win SHARED ${UPNP_SOURCE_FILES})
    set_target_properties(upnplib_shared_win PROPERTIES
            OUTPUT_NAME upnplib
    )
    target_compile_definitions(upnplib_shared_win
            #PRIVATE $<$<BOOL:${script_support}>:IXML_HAVE_SCRIPTSUPPORT>
            #PUBLIC $<IF:$<CONFIG:Debug>,DEBUG,NDEBUG>
            PUBLIC $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>
    )
    # In addition to pthreads we need some more system libraries here
    # ws2_32: winsock to support sockets
    # iphlpapi: ip helper interface to get network management tools
    target_link_libraries(upnplib_shared_win ws2_32 iphlpapi
            ${UPNP_PTHREADS_SHARED_LIBRARY}
    )
    # Test executable linked with the shared library
    add_executable(upnplib_shared_win_exe ${PROJECT_SOURCE_DIR}/src/upnplib.cpp)
    set_target_properties(upnplib_shared_win_exe PROPERTIES
            OUTPUT_NAME upnplib
    )
    target_link_libraries(upnplib_shared_win_exe
            ${UPNP_PTHREADS_SHARED_LIBRARY}
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
            #PUBLIC $<IF:$<CONFIG:Debug>,DEBUG,NDEBUG>
            PUBLIC $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>
    )
    # Test executable linked with the static library
    add_executable(upnplib_static_win_exe ${PROJECT_SOURCE_DIR}/src/upnplib.cpp)
    set_target_properties(upnplib_static_win_exe PROPERTIES
            OUTPUT_NAME upnplibStatic
    )
    # I got warning LNK4098: defaultlib 'LIBCMT' conflicts with use of other
    # libs; use /NODEFAULTLIB:library
    target_link_libraries(upnplib_static_win_exe
            ${UPNP_PTHREADS_STATIC_LIBRARY}
            upnplib_static_win
            -NODEFAULTLIB:LIBCMT
    )

endif() # WIN32
