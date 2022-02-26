# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-02-26

cmake_minimum_required(VERSION 3.18)
include(cmake/project-header.cmake)

set (UPNP_VERSION_STRING "1.14.0")
set (UPNP_VERSION_MAJOR 1)
project(UPnPlib VERSION ${UPNP_VERSION_STRING}
                DESCRIPTION "UPnP library"
                HOMEPAGE_URL "https://github.com/upnplib")


#################################
# Paths                         #
#################################
# Set default POSIX Threads Libraries. May be overwritten if pthreads4w is used.
# See below on installing pthreads4w.
set(UPNP_PTHREADS_SHARED_LIBRARY pthread CACHE INTERNAL "Name of the linkable shared library for POSIX threads")  # Implies FORCE
set(UPNP_PTHREADS_STATIC_LIBRARY libpthread.a CACHE INTERNAL "Name of the linkable static library for POSIX threads")  # Implies FORCE


#################################
# Options                       #
#################################
include(CMakeDependentOption)

option(PT4W_BUILD_TESTING "Build the time and space consuming test suite of pthreads4w" OFF)

option(UPNP_GOOGLETEST "Download and build Googletest" OFF)
# gtest_disable_pthreads is an option of the GoogleTest package set to OFF by
# default so it always compiles with PThreads. This does not compiles on MS
# Windows with pthreads4w. But we cannot disable pthreads by default because on
# Linux mocked functions segfault without it. So we must disable PThreads only
# on MS Windows. The user can set gtest_disable_pthreads as he like.
if(WIN32)
    cmake_dependent_option(gtest_disable_pthreads "Disable Googletest to use POSIX Threads" ON ${UPNP_GOOGLETEST} ON)
endif()

# TODO: Test this option to use the static pthreads4w
# PTW32_STATIC_LIB - for using the static pthreads4W library
# The option is contained in cmake/upnpconfig.h.cmake.
option(PTW32_STATIC_LIB "For using the static pthreads4w library" OFF)


#################################
# Compiling                     #
#################################
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

# Set the default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "MinSizeRel" CACHE STRING "Choose the type of build, options are: None, Debug, Release, RelWithDebInfo, or MinSizeRel." FORCE)
    message(STATUS "No build type specified, defaulting to MinSizeRel.")
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

# Suffix on libraries having built with Debug information
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(UPNP_DEBUG_SUFFIX "d")
endif()

# General define DEBUG if configuration is Debug, otherwise define NDEBUG.
# TODO: manage setting NDEBUG is done by cmake by default. Check against it.
add_compile_definitions(
    PRIVATE $<IF:$<CONFIG:Debug>,DEBUG,NDEBUG>
)

# Set general compile options
#----------------------------
add_compile_options(
        # all warnings as errors
        $<$<CXX_COMPILER_ID:MSVC>:/WX>
        # lots of warnings and all warnings as errors
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wextra>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wpedantic>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Werror>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-dangling-else>
        $<$<CXX_COMPILER_ID:GNU>:-Wno-format-truncation>

        #$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fanalyzer>
        #$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wfatal-errors>
        # Following is only valid for C++/ObjC++ but not for C
        #$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wsuggest-override>
)
if(DEFINED ENV{GITHUB_ACTIONS})
    add_compile_options(
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-unused-result>)
endif()

# Some compiler options for MSVC:
# Reports the compiler warning that is specified by nnnn only once.
#add_compile_options(/wo4273)
# suppress all warnings for development to see only errors
#add_compile_options(/W0)
# warning level 4 and all warnings as errors
# but suppress 'type cast': pointer truncation
#add_compile_options(/W4 /wd4311 /WX)


#################################
# Large-file-sensitivity        #
#################################
# TODO: Check large-file-sensitivity
# [Add LFS flags to pkg-config? #89](https://github.com/pupnp/pupnp/issues/89)
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


##########################################
# POSIX Threads for Windows (pthreads4w) #
##########################################
# This should be one of the first installations to have pthreads available.
if(WIN32)
    # This dependent project must use generator "NMake Makefiles" that may
    # differ from default generator "Visual Studio *". So we cannot use
    # add_directory() because this will also compile the dependent project
    # with "Visual Studio *". We have to configure and build it in separate
    # processes.

    include(FetchContent)

    message(CHECK_START "Download and configuring POSIX Threads for Windows")

    FetchContent_Declare(
        pthreads4w
        GIT_REPOSITORY  https://github.com/jwinarske/pthreads4w.git
        GIT_TAG         origin/cmake
        GIT_SHALLOW     ON)

    # Check if population has already been performed
    FetchContent_GetProperties(pthreads4w)
    if(NOT pthreads4w_POPULATED)
        # Fetch the content using previously declared details.
        # This caches pthreads4w_SOURCE_DIR, pthreads4w_BINARY_DIR and pthreads4w_POPULATED.
        FetchContent_Populate(pthreads4w)

        execute_process(COMMAND cmake -S ${pthreads4w_SOURCE_DIR} -B ${pthreads4w_BINARY_DIR} -G "NMake Makefiles" -D CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -D BUILD_TESTING=${PT4W_BUILD_TESTING}
                        WORKING_DIRECTORY ${pthreads4w_SOURCE_DIR}
                        RESULT_VARIABLE PT4W_RETURN_CODE
                        ERROR_VARIABLE PT4W_ERROR_MESSAGE
                        COMMAND_ECHO STDOUT)
        if(NOT ${PT4W_RETURN_CODE} EQUAL 0)
            # This will stop the installation
            message(FATAL_ERROR "${PT4W_RETURN_CODE}: ${PT4W_ERROR_MESSAGE}")
        endif()

        message(CHECK_PASS "done")
        message(CHECK_START "Building POSIX Threads for Windows libraries")

        if(PT4W_BUILD_TESTING)
            set(PT4W_COMMAND COMMAND_ECHO)
            set(PT4W_COMMAND_OUTPUT "STDOUT")
        else()
            set(PT4W_COMMAND OUTPUT_QUIET)
            set(PT4W_COMMAND_OUTPUT "")
        endif()
        execute_process(COMMAND cmake --build ${pthreads4w_BINARY_DIR} --config ${CMAKE_BUILD_TYPE}
                        WORKING_DIRECTORY ${pthreads4w_SOURCE_DIR}
                        RESULT_VARIABLE PT4W_RETURN_CODE
                        ERROR_VARIABLE PT4W_ERROR_MESSAGE
                        ${PT4W_COMMAND} ${PT4W_COMMAND_OUTPUT})
        if(NOT ${PT4W_RETURN_CODE} EQUAL 0)
            # This will stop the installation
            message(FATAL_ERROR "${PT4W_RETURN_CODE}: ${PT4W_ERROR_MESSAGE}")
        endif()

        set(UPNP_PTHREADS_SHARED_LIBRARY "${pthreads4w_BINARY_DIR}/pthreadVC3${UPNP_DEBUG_SUFFIX}.lib"
            CACHE INTERNAL "Name of the linkable shared library for POSIX threads")  # Implies FORCE
        set(UPNP_PTHREADS_STATIC_LIBRARY "${pthreads4w_BINARY_DIR}/libpthreadVC3${UPNP_DEBUG_SUFFIX}.lib"
            CACHE INTERNAL "Name of the linkable static library for POSIX threads")  # Implies FORCE

        message(CHECK_PASS "done")

    endif(NOT pthreads4w_POPULATED)

    # Output with cmake option --log-level=DEBUG
    message(DEBUG "  DEBUG: pthreads4w_SOURCE_DIR is: ${pthreads4w_SOURCE_DIR}")
    message(DEBUG "  DEBUG: pthreads4w_BINARY_DIR is: ${pthreads4w_BINARY_DIR}")

endif(WIN32)


#################################
# UPnP subdirectories           #
#################################
# This caches <subprojectname>_SOURCE_DIR and <subprojectname_BINARY_DIR
# e.g. UPNP_CORE_SOURCE_DIR and UPNP_CORE_BINARY_DIR

add_subdirectory(${PROJECT_SOURCE_DIR}/pupnp ${PROJECT_SOURCE_DIR}/pupnp/build)
add_subdirectory(${PROJECT_SOURCE_DIR}/core ${PROJECT_SOURCE_DIR}/core/build)
add_subdirectory(${PROJECT_SOURCE_DIR}/sample ${PROJECT_SOURCE_DIR}/sample/build)


#################################
# Googletest                    #
#################################
# Download, configure and build
if(UPNP_GOOGLETEST)

    # It may already included to install pthreads4w on WIN32
    if(NOT WIN32)
        include(FetchContent)
    endif()

    message(CHECK_START "Download and configuring GoogleTest")

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY    https://github.com/google/googletest.git
        #GIT_TAG           origin/main
        GIT_TAG           release-1.11.0
        GIT_SHALLOW       ON
    )
    if(WIN32)

        # Reference: build/_deps/googletest-src/googletest/README.md
        # Must be set before FetchContent_MakeAvailable().
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

        # This caches googletest_SOURCE_DIR, googletest_BINARY_DIR and googletest_POPULATED.
        FetchContent_MakeAvailable(googletest)

        # Cache the different GoogleTest library names for later use.
        # Due to issue https://github.com/google/googletest/issues/1325#issuecomment-903884914
        # it should only link with the gmock libraries. They contain also gtest.
        if(${CMAKE_GENERATOR} MATCHES "Visual Studio")
            set(UPNP_GTEST_LIBRARIES ${PROJECT_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}/gtest${UPNP_DEBUG_SUFFIX}.lib
                                     CACHE INTERNAL "Names of the GoogleTest/GoogleMock libraries")
            set(UPNP_GMOCK_LIBRARIES ${PROJECT_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}/gmock${UPNP_DEBUG_SUFFIX}.lib
                                     CACHE INTERNAL "Names of the GoogleTest/GoogleMock libraries")
            set(UPNP_GTEST_MAIN_LIBRARIES ${PROJECT_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}/gtest_main${UPNP_DEBUG_SUFFIX}.lib
                                     CACHE INTERNAL "Names of the GoogleTest/GoogleMock libraries")
            set(UPNP_GMOCK_MAIN_LIBRARIES ${PROJECT_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}/gmock_main${UPNP_DEBUG_SUFFIX}.lib
                                     CACHE INTERNAL "Names of the GoogleTest/GoogleMock libraries")
        else()
            set(UPNP_GTEST_LIBRARIES ${PROJECT_BINARY_DIR}/lib/gtest${UPNP_DEBUG_SUFFIX}.lib
                                     CACHE INTERNAL "Names of the GoogleTest/GoogleMock libraries")
            set(UPNP_GMOCK_LIBRARIES ${PROJECT_BINARY_DIR}/lib/gmock${UPNP_DEBUG_SUFFIX}.lib
                                     CACHE INTERNAL "Names of the GoogleTest/GoogleMock libraries")
            set(UPNP_GTEST_MAIN_LIBRARIES ${PROJECT_BINARY_DIR}/lib/gtest_main${UPNP_DEBUG_SUFFIX}.lib
                                     CACHE INTERNAL "Names of the GoogleTest/GoogleMock libraries")
            set(UPNP_GMOCK_MAIN_LIBRARIES ${PROJECT_BINARY_DIR}/lib/gmock_main${UPNP_DEBUG_SUFFIX}.lib
                                     CACHE INTERNAL "Names of the GoogleTest/GoogleMock libraries")
        endif()

        message(CHECK_PASS "done")


        ##################################
        # Build Unit Tests on MS Windows #
        ##################################

        message(CHECK_START "configuring Unit Tests")

        # This caches UPNP_GTESTS_SOURCE_DIR and UPNP_GTESTS_BINARY_DIR
        add_subdirectory(${PROJECT_SOURCE_DIR}/gtests ${PROJECT_SOURCE_DIR}/gtests/build)

        # Create symlinks for shared gtest .dlls so the Unit Tests can find them.
        if(BUILD_SHARED_LIBS)
            if(${CMAKE_GENERATOR} MATCHES "Visual Studio")
                # Make Directory is necessary because sub directory ${CMAKE_BUILD_TYPE} isn't available yet.
                file(MAKE_DIRECTORY ${UPNP_GTESTS_BINARY_DIR}/${CMAKE_BUILD_TYPE})
                file(CREATE_LINK ${PROJECT_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/gtest${UPNP_DEBUG_SUFFIX}.dll
                                 ${UPNP_GTESTS_BINARY_DIR}/${CMAKE_BUILD_TYPE}/gtest${UPNP_DEBUG_SUFFIX}.dll SYMBOLIC)
                file(CREATE_LINK ${PROJECT_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/gmock${UPNP_DEBUG_SUFFIX}.dll
                                 ${UPNP_GTESTS_BINARY_DIR}/${CMAKE_BUILD_TYPE}/gmock${UPNP_DEBUG_SUFFIX}.dll SYMBOLIC)
                file(CREATE_LINK ${PROJECT_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/gtest_main${UPNP_DEBUG_SUFFIX}.dll
                                 ${UPNP_GTESTS_BINARY_DIR}/${CMAKE_BUILD_TYPE}/gtest_main${UPNP_DEBUG_SUFFIX}.dll SYMBOLIC)
                file(CREATE_LINK ${PROJECT_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/gmock_main${UPNP_DEBUG_SUFFIX}.dll
                                 ${UPNP_GTESTS_BINARY_DIR}/${CMAKE_BUILD_TYPE}/gmock_main${UPNP_DEBUG_SUFFIX}.dll SYMBOLIC)
            else()
                file(CREATE_LINK ${PROJECT_BINARY_DIR}/bin/gtest${UPNP_DEBUG_SUFFIX}.dll
                                 ${UPNP_GTESTS_BINARY_DIR}/gtest${UPNP_DEBUG_SUFFIX}.dll SYMBOLIC)
                file(CREATE_LINK ${PROJECT_BINARY_DIR}/bin/gmock${UPNP_DEBUG_SUFFIX}.dll
                                 ${UPNP_GTESTS_BINARY_DIR}/gmock${UPNP_DEBUG_SUFFIX}.dll SYMBOLIC)
                file(CREATE_LINK ${PROJECT_BINARY_DIR}/bin/gtest_main${UPNP_DEBUG_SUFFIX}.dll
                                 ${UPNP_GTESTS_BINARY_DIR}/gtest_main${UPNP_DEBUG_SUFFIX}.dll SYMBOLIC)
                file(CREATE_LINK ${PROJECT_BINARY_DIR}/bin/gmock_main${UPNP_DEBUG_SUFFIX}.dll
                                 ${UPNP_GTESTS_BINARY_DIR}/gmock_main${UPNP_DEBUG_SUFFIX}.dll SYMBOLIC)
            endif()

        endif(BUILD_SHARED_LIBS)

    else(WIN32) # AND UPNP_GOOGLETEST

        # This caches googletest_SOURCE_DIR, googletest_BINARY_DIR and googletest_POPULATED.
        FetchContent_MakeAvailable(googletest)

        # Cache the GoogleTest library names for later use.
        # Due to issue https://github.com/google/googletest/issues/1325#issuecomment-903884914
        # it should only link with the gmock libraries. They contain also gtest.
        if(BUILD_SHARED_LIBS)
            if(APPLE)
                set(UPNP_GTEST_LIBRARIES ${PROJECT_BINARY_DIR}/lib/libgtest${UPNP_DEBUG_SUFFIX}.dylib
                                         CACHE INTERNAL "Names of the GoogleTest/GoogleMock shared libraries")
                set(UPNP_GMOCK_LIBRARIES ${PROJECT_BINARY_DIR}/lib/libgmock${UPNP_DEBUG_SUFFIX}.dylib
                                         CACHE INTERNAL "Names of the GoogleTest/GoogleMock shared libraries")
                set(UPNP_GTEST_MAIN_LIBRARIES ${PROJECT_BINARY_DIR}/lib/libgtest_main${UPNP_DEBUG_SUFFIX}.dylib
                                         CACHE INTERNAL "Names of the GoogleTest/GoogleMock shared libraries")
                set(UPNP_GMOCK_MAIN_LIBRARIES ${PROJECT_BINARY_DIR}/lib/libgmock_main${UPNP_DEBUG_SUFFIX}.dylib
                                         CACHE INTERNAL "Names of the GoogleTest/GoogleMock shared libraries")
            else()
                set(UPNP_GTEST_LIBRARIES ${PROJECT_BINARY_DIR}/lib/libgtest${UPNP_DEBUG_SUFFIX}.so
                                         CACHE INTERNAL "Names of the GoogleTest/GoogleMock shared libraries")
                set(UPNP_GMOCK_LIBRARIES ${PROJECT_BINARY_DIR}/lib/libgmock${UPNP_DEBUG_SUFFIX}.so
                                         CACHE INTERNAL "Names of the GoogleTest/GoogleMock shared libraries")
                set(UPNP_GTEST_MAIN_LIBRARIES ${PROJECT_BINARY_DIR}/lib/libgtest_main${UPNP_DEBUG_SUFFIX}.so
                                         CACHE INTERNAL "Names of the GoogleTest/GoogleMock shared libraries")
                set(UPNP_GMOCK_MAIN_LIBRARIES ${PROJECT_BINARY_DIR}/lib/libgmock_main${UPNP_DEBUG_SUFFIX}.so
                                         CACHE INTERNAL "Names of the GoogleTest/GoogleMock shared libraries")
            endif()
        else(BUILD_SHARED_LIBS)
            set(UPNP_GTEST_LIBRARIES ${PROJECT_BINARY_DIR}/lib/libgtest${UPNP_DEBUG_SUFFIX}.a
                                     CACHE INTERNAL "Names of the GoogleTest/GoogleMock static libraries")
            set(UPNP_GMOCK_LIBRARIES ${PROJECT_BINARY_DIR}/lib/libgmock${UPNP_DEBUG_SUFFIX}.a
                                     CACHE INTERNAL "Names of the GoogleTest/GoogleMock static libraries")
            set(UPNP_GTEST_MAIN_LIBRARIES ${PROJECT_BINARY_DIR}/lib/libgtest_main${UPNP_DEBUG_SUFFIX}.a
                                     CACHE INTERNAL "Names of the GoogleTest/GoogleMock static libraries")
            set(UPNP_GMOCK_MAIN_LIBRARIES ${PROJECT_BINARY_DIR}/lib/libgmock_main${UPNP_DEBUG_SUFFIX}.a
                                     CACHE INTERNAL "Names of the GoogleTest/GoogleMock static libraries")
        endif(BUILD_SHARED_LIBS)

        message(CHECK_PASS "done")


        ##################################
        # Build default Unit Tests       #
        ##################################

        message(CHECK_START "configuring Unit Tests")

        # This caches UPNP_GTESTS_SOURCE_DIR and UPNP_GTESTS_BINARY_DIR
        add_subdirectory(${PROJECT_SOURCE_DIR}/gtests ${PROJECT_SOURCE_DIR}/gtests/build)

        message(CHECK_PASS "done")

    endif(WIN32)

    # Output with cmake option --log-level=DEBUG
    message(DEBUG "  DEBUG: googletest_SOURCE_DIR is: ${googletest_SOURCE_DIR}")
    message(DEBUG "  DEBUG: googletest_BINARY_DIR is: ${googletest_BINARY_DIR}")

endif(UPNP_GOOGLETEST)

#################################
# Export cmake variables        #
#################################
# This should be the last statement
#----------------------------------
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

# RPATH settings to find shared libraries
# reference: https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling
# Output with cmake option --log-level=DEBUG
message(DEBUG "\n     DEBUG: RPATH settings\n     ---------------------")
message(DEBUG "  DEBUG: CMAKE_SKIP_RPATH is ................: ${CMAKE_SKIP_RPATH}")
message(DEBUG "  DEBUG: CMAKE_SKIP_INSTALL_RPATH is ........: ${CMAKE_SKIP_INSTALL_RPATH}")
message(DEBUG "  DEBUG: CMAKE_MACOSX_RPATH is ..............: ${CMAKE_MACOSX_RPATH}")
message(DEBUG "  DEBUG: CMAKE_INSTALL_NAME_DIR is ..........: ${CMAKE_INSTALL_NAME_DIR}")
message(DEBUG "  DEBUG: CMAKE_INSTALL_RPATH is .............: ${CMAKE_INSTALL_RPATH}")
message(DEBUG "  DEBUG: CMAKE_SKIP_BUILD_RPATH is ..........: ${CMAKE_SKIP_BUILD_RPATH}")
message(DEBUG "  DEBUG: CMAKE_BUILD_WITH_INSTALL_RPATH is ..: ${CMAKE_BUILD_WITH_INSTALL_RPATH}")
message(DEBUG "  DEBUG: CMAKE_INSTALL_RPATH_USE_LINK_PATH is: ${CMAKE_INSTALL_RPATH_USE_LINK_PATH}")

message(STATUS "Build type is ${CMAKE_BUILD_TYPE}")
