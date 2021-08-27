# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2021-08-27

cmake_minimum_required(VERSION 3.18)
include(cmake/project-header.cmake)

set (UPNP_VERSION_STRING "3.0.0")
set (UPNP_VERSION_MAJOR 1)
project(UPnPlib VERSION ${UPNP_VERSION_STRING}
                DESCRIPTION "UPnP library"
                HOMEPAGE_URL "https://github.com/upnplib")


# Define custom cmake options for commandline and cmake-gui
#------------------------------------------------------------------------
# If you add or modify an option you should also have attention to a possible
# corresponding flag in "${PROJECT_SOURCE_DIR}/cmake/upnpconfig.h.cmake". These
# variables are exported at the end. See below at section "Export cmake
# variables".
# Variablename and its value-placeholder must be the same, e.g.
# #cmakedefine VAR1 ${VAR1}
# #cmakedefine VAR1 ${VAR} does not work.
# #cmakedefine VAR2 1  : VAR2 must be exist before even you set a constant.

include(CMakeDependentOption)

# TODO: Test this option to link statically
# UPNP_STATIC_LIB - for creating a statically linkable UPnP-library
# The option is contained in cmake/upnpconfig.h.cmake.
option(UPNP_STATIC_LIB "Create a statically linkable UPnP-library" OFF)

# TODO: Test this option to use the static pthreads4w
# PTW32_STATIC_LIB - for using the static pthreads4W library
# The option is contained in cmake/upnpconfig.h.cmake.
option(PTW32_STATIC_LIB "For using the static pthreads4w library" OFF)


# Setup configure and build environment
#--------------------------------------
# Set default POSIX threads libraries. May be overwritten if pthreads4w is
# used. See below on installing pthreads4w.
set(UPNP_PTHREADS_SHARED_LIBRARY pthread CACHE INTERNAL  # Implies FORCE
    "Name of the linkable shared library for POSIX threads")
set(UPNP_PTHREADS_STATIC_LIBRARY libpthread.a CACHE INTERNAL  # Implies FORCE
    "Name of the linkable static library for POSIX threads")

# Check developement setting from cmake option (-D UPNP_DEVEL)
if(UPNP_DEVEL)
# Commandline variables are always cached. This one shouldn't?
    unset(UPNP_DEVEL CACHE)
    set(UPNP_DEVEL ON)
endif()

# Set the default build type
if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "Release" CACHE
        STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel ..." FORCE)
endif()

# Check available library functions. We need that for later configuration.
include(CheckCXXSymbolExists)
# The option is contained in cmake/upnpconfig.h.cmake.
check_cxx_symbol_exists(strnlen "cstring" HAVE_STRNLEN)
# The option is contained in cmake/upnpconfig.h.cmake.
check_cxx_symbol_exists(strndup "cstring" HAVE_STRNDUP)

# Set a compiler flag to respect specific features of the compiler.
# The option is contained in cmake/upnpconfig.h.cmake.
if(MSVC)
    set(UPNP_USE_MSVCPP 1)
endif()


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


# Installation of POSIX Threads for Windows (pthreads4w)
#-------------------------------------------------------
# This should be one of the first installations to have pthreads available.
if(WIN32)
    # This caches <subprojectname>_SOURCE_DIR and <subprojectname_BINARY_DIR
    # e.g. PTHREADS4W_SOURCE_DIR and PTHREADS4W_BINARY_DIR
    add_subdirectory(${PROJECT_SOURCE_DIR}/pthreads4w
                     ${PROJECT_SOURCE_DIR}/pthreads4w/build
                     EXCLUDE_FROM_ALL)
    set(UPNP_PTHREADS_SHARED_LIBRARY ${PTHREADS4W_BINARY_DIR}/lib/pthreadVC3.lib
            CACHE INTERNAL  # Implies FORCE
            "Name of the linkable shared library for POSIX threads")
    set(UPNP_PTHREADS_STATIC_LIBRARY ${PTHREADS4W_BINARY_DIR}/lib/libpthreadVC3.lib
            CACHE INTERNAL  # Implies FORCE
            "Name of the linkable static library for POSIX threads")
else()
    # If not on MS Windows we don't need the pthreads4w library
    if(NOT UPNP_DEVEL)
        file(REMOVE_RECURSE ${PROJECT_SOURCE_DIR}/pthreads4w)
    endif()
endif()


# Build UPnP core library
#------------------------
# This caches <subprojectname>_SOURCE_DIR and <subprojectname_BINARY_DIR
# e.g. UPNP_CORE_SOURCE_DIR and UPNP_CORE_BINARY_DIR

add_subdirectory(${PROJECT_SOURCE_DIR}/upnp
                 ${PROJECT_SOURCE_DIR}/upnp/build)
add_subdirectory(${PROJECT_SOURCE_DIR}/ixml
                 ${PROJECT_SOURCE_DIR}/ixml/build)


# Download, configure and build googletest
#-----------------------------------------
Find_Package(Git)
cmake_dependent_option(UPNP_GOOGLETEST "Download and build Googletest" OFF ${Git_FOUND} OFF)
cmake_dependent_option(UPNP_GOOGLETEST "Build Googletest shared libraries" OFF ${Git_FOUND} OFF)

if (UPNP_GOOGLETEST)
    add_subdirectory(${PROJECT_SOURCE_DIR}/gtests
                     ${PROJECT_SOURCE_DIR}/gtests/build)
endif()


# Export cmake variables; should be the last statement
#-----------------------------------------------------
# to compiler macros (#define ...) for conditions to the program source.
# Hope we have all needed variables defined so we can write upnpconfig.h now.
# To meet all variables this should be the last statement.
# Variablename and its value-placeholder must be the same, e.g.
# #cmakedefine VAR1 ${VAR1}
# #cmakedefine VAR1 ${VAR} does not work.
# #cmakedefine VAR2 1  : VAR2 must be exist before even you set a constant.
configure_file(${PROJECT_SOURCE_DIR}/cmake/upnpconfig.h.cmake
               ${PROJECT_BINARY_DIR}/upnp/inc/upnpconfig.h
)
# TODO: Remove symlink which is only for compatibility reasons
file(CREATE_LINK ${PROJECT_BINARY_DIR}/upnp/inc/upnpconfig.h
                 ${PROJECT_BINARY_DIR}/autoconfig.h
                 COPY_ON_ERROR SYMBOLIC
)

message("-- Build type is ${CMAKE_BUILD_TYPE}")
