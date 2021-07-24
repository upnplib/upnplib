# Copyright 2021 GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Last modified: 2021-07-23

CMAKE_MINIMUM_REQUIRED (VERSION 3.18)
CMAKE_POLICY (VERSION 3.18)
if("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    message ("In-source builds are not supported.")
    message ("Use a separate folder for building, for example:\n")
    message ("    cmake -S . -B build/")
    message ("    cmake --build build/\n")
    message ("Before that, remove the files already created:\n")
    message ("    rm -rf CMakeCache.txt CMakeFiles\n")
    message (FATAL_ERROR "")
endif()

# set the project name and version
set (UPNP_VERSION_STRING "1.0.0")
set (UPNP_VERSION_MAJOR 1)
project(UPnPlib VERSION ${UPNP_VERSION_STRING}
                DESCRIPTION "UPnP library"
                HOMEPAGE_URL "https://github.com/upnplib")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Set upnplib options
#------------------------------------------------------------------------
# If you add or modify an option you MUST also do it in the corresponding
# "${PROJECT_SOURCE_DIR}/upnp/cmake/upnpconfig.h.cmake".
# Variablename and its value-placeholder must be the same.
# #cmakedefine VAR1 ${VAR} does not work.
# #cmakedefine VAR2 1  : VAR2 must be exist before even you set a constant.

    option(BUILDTESTS "Enable time- and storage-consuming extended tests on the installation" ON)

    # TODO: Test this option to link statically
    # UPNP_STATIC_LIB - for creating a statically linkable UPnP-library
    option(UPNP_STATIC_LIB "Create a statically linkable UPnP-library" OFF)

    # TODO: Test this option to use the static pthreads4W
    # PTW32_STATIC_LIB - for using the static pthreads4W library
    option(PTW32_STATIC_LIB "For using the static pthreads4W library" OFF)

    # Check available library functions. We need that for later configuration.
    include(CheckCXXSymbolExists)
    check_cxx_symbol_exists(strnlen "cstring" HAVE_STRNLEN)
    check_cxx_symbol_exists(strndup "cstring" HAVE_STRNDUP)


# Configure and compile settings
#-------------------------------
set(PTHREADS4W_DIR ${PROJECT_BINARY_DIR}/pthreads4w)

if (MSVC)
    # TODO: Fix warnings from the MSVC compiler
    # Reports the compiler warning that is specified by nnnn only once.
    #add_compile_options(/wo4273)
    # suppress all warnings for development to see only errors
    #add_compile_options(/W0)
else()
    # lots of warnings and all warnings as errors
    #add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()


include_directories(
#    "${PROJECT_SOURCE_DIR}/../build/_deps/googletest-src/googletest/include"
#    "${PROJECT_SOURCE_DIR}/../build/_deps/googletest-src/googlemock/include"
    "${PROJECT_SOURCE_DIR}/upnp/inc"
    "${PROJECT_SOURCE_DIR}/upnp/src/inc"
    "${PROJECT_SOURCE_DIR}/upnp/src/threadutil"
    "${PROJECT_SOURCE_DIR}/ixml/inc"
    "${PROJECT_SOURCE_DIR}/ixml/src/inc"
    "${PTHREADS4W_DIR}/include"
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_BINARY_DIR}/upnp/inc"
)

add_executable(upnplib
    ${PROJECT_SOURCE_DIR}/ixml/src/attr.c
    ${PROJECT_SOURCE_DIR}/ixml/src/document.c
    ${PROJECT_SOURCE_DIR}/ixml/src/element.c
    ${PROJECT_SOURCE_DIR}/ixml/src/ixml.c
    ${PROJECT_SOURCE_DIR}/ixml/src/ixmlmembuf.c
    ${PROJECT_SOURCE_DIR}/ixml/src/ixmlparser.c
    ${PROJECT_SOURCE_DIR}/ixml/src/namedNodeMap.c
    ${PROJECT_SOURCE_DIR}/ixml/src/node.c
    ${PROJECT_SOURCE_DIR}/ixml/src/nodeList.c

    ${PROJECT_SOURCE_DIR}/upnp/src/threadutil/FreeList.cpp
    ${PROJECT_SOURCE_DIR}/upnp/src/threadutil/LinkedList.cpp
    ${PROJECT_SOURCE_DIR}/upnp/src/threadutil/ThreadPool.cpp
    ${PROJECT_SOURCE_DIR}/upnp/src/threadutil/TimerThread.cpp

    ${PROJECT_SOURCE_DIR}/upnp/src/genlib/net/sock.c

    ${PROJECT_SOURCE_DIR}/upnp/src/genlib/net/uri/uri.c

    ${PROJECT_SOURCE_DIR}/upnp/src/genlib/net/http/httpparser.c
    ${PROJECT_SOURCE_DIR}/upnp/src/genlib/net/http/httpreadwrite.c
    ${PROJECT_SOURCE_DIR}/upnp/src/genlib/net/http/statcodes.c

    ${PROJECT_SOURCE_DIR}/upnp/src/genlib/util/list.c
    ${PROJECT_SOURCE_DIR}/upnp/src/genlib/util/membuffer.c
    ${PROJECT_SOURCE_DIR}/upnp/src/genlib/util/strintmap.c

    ${PROJECT_SOURCE_DIR}/upnp/src/api/UpnpExtraHeaders.c
    ${PROJECT_SOURCE_DIR}/upnp/src/api/UpnpString.c
    ${PROJECT_SOURCE_DIR}/upnp/src/api/upnpapi.cpp
    ${PROJECT_SOURCE_DIR}/upnp/src/api/upnpdebug.cpp

    ${PROJECT_SOURCE_DIR}/upnp/src/upnplib.cpp
)

target_compile_definitions (upnplib
    #PRIVATE $<$<BOOL:${script_support}>:IXML_HAVE_SCRIPTSUPPORT>
    #PUBLIC $<IF:$<CONFIG:Debug>,DEBUG,NDEBUG>
    PUBLIC $<$<BOOL:${MSVC}>:UPNP_USE_MSVCPP>
    PUBLIC $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>
)


# Checks for large-file-sensitivity
#----------------------------------------------------
# TODO: Check large-file-sensitivity
# Check for what CMAKE_REQUIRED_DEFINITIONS is needed
# In the changelog I found this:
#
#2011-01-16 Marcelo Roberto Jimenez <mroberto(at)users.sourceforge.net>
#
#    Define _FILE_OFFSET_BITS, _LARGEFILE_SOURCE and _LARGE_FILE_SOURCE in
#    upnpconfig.h.
#
#    Make these definitions available to programs using the library.
#    Thanks to Chandra Penke for pointing the problem.
#
#    Summary: Problem with large file support in pupnp build - ID: 3158969
#    Submitted: Chandra ( inactiveneurons ) - 2011-01-15 16:17:02 BRST
#    Details:
#    First off, I apologize in advance for the length of this comment, it's
#    the only way I could describe the problem accurately.
#
#    Secondly, a brief thanks (again). The company I'm working for has been
#    using pupnp for a massively cross-platform project which involves iphone,
#    osx, windows, linux x86, arm, and mips hosts. It's amazing how well it
#    works, so kudos to the maintainers!
#
#    We came across a problem when compiling with the following tool-chain:
#    http://www.codesourcery.com/sgpp/lite/mips/portal/release824. The
#    problem is the following:
#
#    In configure.ac the following lines exist to enable large file support:
#
#    AC_DEFINE([_LARGE_FILE_SOURCE], [], [Large files support])
#    AC_DEFINE([_FILE_OFFSET_BITS], [64], [File Offset size])
#
#    Which in turn result in the following #defines in autoconfig.h:
#
#    #define _LARGE_FILE_SOURCE /**/
#    #define _FILE_OFFSET_BITS 64
#
#    However, this file is not exported as part of the upnp build. Therefore,
#    while the entire library gets built with large file support, it's
#    possible that dependent libraries which only rely on the include files
#    may not use large file support.
#
#    In the particular case of the above tool-chain, the 'off_t' type is 8
#    bytes when large file support is enabled, but only 4 bytes when it's
#    not. As a result part our stack built on top of pupnp, which did not
#    have large file support (because it did not use the above autoconf
#    directives), was relying on 'a off_t' that was 4 bytes.
#
#    This caused, among many things, for the UpnpFileInfo struct to break.
#    Since the struct is completely invisible outside of pupnp (because of
#    some template macro magic), pupnp thought that 'FileLength' field was
#    8 bytes, but the header setter/getter methods being used by dependent
#    libraries thought that it was 4, which caused some erratic behavior
#    when going through pupnp's webserver and HTTP client API.
#
#    We put in a temporary work around by adding the following preprocessor
#    flags: -D_LARGE_FILE_SOURCE, -D_FILE_OFFSET_BITS=64 as part of our
#    build process. However, it's a hack, and I was wondering if I'm missing
#    something and there's a better way to approach this.
#
#    Thanks,
#    Chandra

include(CheckTypeSize)
if (NOT OFF_T_SIZE)
    check_type_size (off_t OFF_T_SIZE)
    set (UPNP_LARGEFILE_SENSITIVE FALSE CACHE BOOL "whether the system defaults to 32bit off_t but can do 64bit when requested" FORCE)

    if (OFF_T_SIZE EQUAL 8)
        message (STATUS "System uses 64 bit, no need to consider LFS (Large File Storage)")
    else()
        unset (OFF_T_SIZE CACHE)
        set (CMAKE_REQUIRED_DEFINITIONS _FILE_OFFSET_BITS=64)
        check_type_size (off_t OFF_T_SIZE)

        if (OFF_T_SIZE EQUAL 8)
            message (STATUS "_FILE_OFFSET_BITS=64 needed to support LFS (Large File Storage)")
            set (UPNP_LARGEFILE_SENSITIVE TRUE CACHE BOOL "whether the system defaults to 32bit off_t but can do 64bit when requested" FORCE)
            set (_FILE_OFFSET_BITS 64 CACHE BOOL "Number of bits in a file offset, on hosts where this is settable" FORCE)
        else()
            unset (OFF_T_SIZE CACHE)
            set (CMAKE_REQUIRED_DEFINITIONS _LARGE_FILES)
            check_type_size (off_t OFF_T_SIZE)

            if (OFF_T_SIZE EQUAL 8)
                message (STATUS "_LARGE_FILES needed to support LFS (Large File Storage)")
                set (_LARGE_FILES TRUE CACHE BOOL "Define for large files, on AIX-style hosts." FORCE)
                set (UPNP_LARGEFILE_SENSITIVE TRUE CACHE BOOL "whether the system defaults to 32bit off_t but can do 64bit when requested" FORCE)
            endif()
        endif()
    endif()
endif()

# All common work is done.
# Process Microsoft Windows specific configuriation
include(${PROJECT_SOURCE_DIR}/cmake/ms-windows.cmake)


# Hope we have all needed variables defined so we can write upnpconfig.h now
configure_file(${PROJECT_SOURCE_DIR}/cmake/upnpconfig.h.cmake
               ${PROJECT_BINARY_DIR}/upnp/inc/upnpconfig.h)
# TODO: Remove symlink which is only for compatibility reasons
file(CREATE_LINK ${PROJECT_BINARY_DIR}/upnp/inc/upnpconfig.h
                 ${PROJECT_BINARY_DIR}/autoconfig.h
                 COPY_ON_ERROR SYMBOLIC)
