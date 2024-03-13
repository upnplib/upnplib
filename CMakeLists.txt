# Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2024-03-12

cmake_minimum_required(VERSION 3.18)
include(cmake/project-header.cmake)

set (UPNP_VERSION_STRING "1.14.18")
set (UPNP_VERSION_MAJOR 1)
project(UPNPLIB_PROJECT VERSION ${UPNP_VERSION_STRING}
                DESCRIPTION "UPnP library"
                HOMEPAGE_URL "https://github.com/upnplib")

enable_testing()

#################################
# Options                       #
#################################
include(CMakeDependentOption)

option(UPNPLIB_WITH_NATIVE_PUPNP "Build the library with old native PUPNP code." OFF)
option(UPNPLIB_WITH_OPENSSL "SSL/TLS and Crypto support should always be ON if not restricted by law." ON)
option(UPNPLIB_WITH_MINISERVER "Provide the miniserver for low level network communication." OFF)
option(UPNPLIB_WITH_WEBSERVER "Provide a webserver." OFF)
option(UPNPLIB_WITH_CTRLPT_SSDP "Providing control point functionality to manage a remote UPnP device." OFF)
option(UPNPLIB_WITH_DEVICE_SSDP "Providing UPnP device functionality so remote control points can connect to it." OFF)
option(UPNPLIB_WITH_OPTION_SSDP "Enable optional SSDP headers." OFF)
option(UPNPLIB_WITH_CTRLPT_DESCRIPTION "Enable requesting from a control point for a devices UPnP description." OFF)
option(UPNPLIB_WITH_DEVICE_DESCRIPTION "Enable responding from a device to a control points UPnP description request." OFF)
option(UPNPLIB_WITH_CTRLPT_SOAP "Enable SOAP, the Simple Object Access Protocol." OFF)
option(UPNPLIB_WITH_DEVICE_SOAP "Enable SOAP, the Simple Object Access Protocol." OFF)
option(UPNPLIB_WITH_CTRLPT_GENA "Enable GENA, the General Event Notification Architecture." OFF)
option(UPNPLIB_WITH_DEVICE_GENA "Enable GENA, the General Event Notification Architecture." OFF)
option(UPNPLIB_WITH_IXML "Enable IXML script support." OFF)
option(UPNPLIB_WITH_TOOLS "Make some not essential program features available like text for error numbers etc." OFF)
option(UPNPLIB_WITH_SAMPLES "Provide sample programs device and control point." OFF)
option(UPNPLIB_WITH_GOOGLETEST "Download and build Googletest." OFF)
option(UPNPLIB_WITH_TRACE "Compile trace messages into the libraries for analizing program execution." OFF)
option(GTESTS_WITH_SHARED_LIBS "Build GOOGLETEST libraries shared. You should not disable this except you know what you doing." ON)
    # Because we link utests with shared and static libraries we need gtest/gmock
    # libraries that link together with both versions of other libraries. These are
    # the shared gtest/gmock libraries. Next option is only available for testing
    # other combinations. It should be set to ON by default otherwise you may get
    # compiling errors if you do not know what you are doing.
# option(UPNPLIB_WITH_UNSPECIFIED_SERVER "Set information in UPnP SERVER http request header to \"Unspecified\"". OFF)
    # This is an option to set to "Unspecified" the OS name, OS version, product
    # name and product version normally contained in the SERVER header of a
    # UPnP http request as this could be used by an attacker.
# option(UPNPLIB_WITH_BLOCKTCP "Enable the TCP protocol stack to wait (blocking) until receiving an answer or timed out." ON)
    # This is not an option anymore. Blocking until data received is always
    # enabled.

if(UPNPLIB_WITH_GOOGLETEST OR UPNPLIB_WITH_SAMPLES)
    # It is important to have the old library available with the tests because
    # the new code is compared with it.
    set (UPNPLIB_WITH_NATIVE_PUPNP ON CACHE BOOL
        "Build the library with old native PUPNP code." FORCE)
endif()

if(GTESTS_WITH_SHARED_LIBS)
    set (BUILD_SHARED_LIBS ON CACHE INTERNAL
        "This option belongs only to GOOGLETEST and build its libraries shared.")  # Implies FORCE?
endif()

option(PT4W_BUILD_TESTING "Build the time and space consuming test suite of pthreads4w" OFF)
# gtest_disable_pthreads is an option of the GoogleTest package set to OFF by
# default so it always compiles with PThreads. This does not compiles on MS
# Windows with pthreads4w. But we cannot disable pthreads by default because on
# Linux mocked functions segfault without it. So we must disable PThreads only
# on MS Windows. The user can set gtest_disable_pthreads as he like.
if(WIN32)
    CMAKE_DEPENDENT_OPTION(gtest_disable_pthreads "Disable Googletest to use POSIX Threads" ON ${UPNPLIB_WITH_GOOGLETEST} ON)
endif()

# TODO: Test this option to use the static pthreads4w
# PTW32_STATIC_LIB - for using the static pthreads4W library
# The option is contained in cmake/upnpconfig.hpp.cmake.
option(PTW32_STATIC_LIB "For using the static pthreads4w library" OFF)

# General settings given to the program
add_compile_definitions(
    $<$<BOOL:${UPNPLIB_WITH_TOOLS}>:UPNP_HAVE_TOOLS>
)


#################################
# Paths                         #
#################################
# Set default POSIX Threads Libraries. May be overwritten if pthreads4w is used.
# See below on installing pthreads4w.
set(UPNP_PTHREADS_SHARED_LIBRARY pthread CACHE INTERNAL "Name of the linkable shared library for POSIX threads")  # Implies FORCE
set(UPNP_PTHREADS_STATIC_LIBRARY libpthread.a CACHE INTERNAL "Name of the linkable static library for POSIX threads")  # Implies FORCE


#################################
# Compiling                     #
#################################
# Define custom cmake options for commandline and cmake-gui
#------------------------------------------------------------------------
# If you add or modify an option you should also have attention to a possible
# corresponding flag in "${PROJECT_SOURCE_DIR}/cmake/upnpconfig.hpp.cmake". These
# variables are exported at the end. See below at section "Export cmake
# variables".
# Variablename and its value-placeholder must be the same, e.g.
# #cmakedefine VAR1 ${VAR1}
# #cmakedefine VAR1 ${VAR} does not work.
# #cmakedefine VAR2 1  : VAR2 must be exist before even you set a constant.

# Set the default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: None, Debug, Release, RelWithDebInfo, or MinSizeRel." FORCE)
    message(STATUS "No build type specified, defaulting to Release.")
endif()

# Check available library functions. We need that for later configuration.
include(CheckCXXSymbolExists)

# The option is contained in cmake/upnpconfig.hpp.cmake.
check_cxx_symbol_exists(strnlen "cstring" UPNPLIB_HAVE_STRNLEN)

# The option is contained in cmake/upnpconfig.hpp.cmake.
check_cxx_symbol_exists(strndup "cstring" UPNPLIB_HAVE_STRNDUP)

# Suffix on libraries having built with Debug information
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(UPNP_DEBUG_SUFFIX "d")
endif()

# We have visibility support as described at https://gcc.gnu.org/wiki/Visibility
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# Provide length of the path of the projects base directory. We need this
# mainly for the compile time short file name of Debug and TRACE messages.
string(LENGTH "${UPNPLIB_PROJECT_SOURCE_DIR}/" UPNPLIB_PROJECT_PATH_LENGTH)

# Set general compile definitions and options
#--------------------------------------------
add_compile_definitions(
        # Having STRNLEN and STRNDUP are validated above. Give it to the program.
        $<$<BOOL:${UPNPLIB_HAVE_STRNLEN}>:HAVE_STRNLEN>
        $<$<BOOL:${UPNPLIB_HAVE_STRNDUP}>:HAVE_STRNDUP>
        # General define DEBUG if build type is "Debug". Manage setting NDEBUG
        # is done by cmake by default.
        $<$<CONFIG:Debug>:DEBUG>
        # There are traditional min/max macros on MS Windows which conflict
        # with the C++ std::min|max functions. Following switch disable the
        # macros.
        $<$<CXX_COMPILER_ID:MSVC>:NOMINMAX>

        $<$<CXX_COMPILER_ID:MSVC>:UPNP_USE_MSVCPP>
        $<$<CXX_COMPILER_ID:MSVC>:FD_SETSIZE=1024>
        $<$<BOOL:${UPNPLIB_WITH_TRACE}>:UPNPLIB_WITH_TRACE>
)
add_compile_options(
        # Most compiler warnings enabled
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        # all compiler warnings as errors
        $<$<CXX_COMPILER_ID:MSVC>:/WX>
        # disable warning C4996: 'strdup'(or others): The POSIX name for this
        # item is deprecated. Instead, use the ISO C and C++ conformant name:
        # _strdup. See online help for details. (This is Microsoft specific, no
        # need to fix).
        $<$<CXX_COMPILER_ID:MSVC>:/wd4996>
        # lots of warnings and all warnings as errors
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wextra>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wpedantic>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Werror>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wuninitialized>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-dangling-else>
        $<$<CXX_COMPILER_ID:GNU>:-Wno-format-truncation>
        $<$<CXX_COMPILER_ID:GNU>:-Wsuggest-override> # Only valid for C++/ObjC++ but not for C
        $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU>>:-fstack-protector-all>
        $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU>>:-fstack-protector-strong>
        # Next is only to suppress a warning for the TRACE macro.
        #$<$<AND:$<CXX_COMPILER_ID:AppleClang>,$<BOOL:${UPNPLIB_WITH_TRACE}>>:-Wno-string-plus-int>

        #$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-fanalyzer>
        #$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wfatal-errors>
)
if(DEFINED ENV{GITHUB_ACTIONS})
    add_compile_options(
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-unused-result>)
endif()

add_link_options(
        $<$<CXX_COMPILER_ID:MSVC>:/WX>   # all linker warnings as errors
)


#################################
# Targets                       #
#################################
# Set target output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${UPNPLIB_PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${UPNPLIB_PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${UPNPLIB_PROJECT_BINARY_DIR}/bin)
if(WIN32)
    if(CMAKE_GENERATOR STREQUAL "NMake Makefiles")
        set(UPNPLIB_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    else()
        set(UPNPLIB_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
    endif()
else()
    set(UPNPLIB_RUNTIME_OUTPUT_DIRECTORY "")
endif()
# Output with cmake option --log-level=DEBUG
message(DEBUG "  DEBUG: UPNPLIB_RUNTIME_OUTPUT_DIRECTORY is: ${UPNPLIB_RUNTIME_OUTPUT_DIRECTORY}")


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
#    upnpconfig.hpp.
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
#    Which in turn result in the following #defines in autoconfig.h
#    (now upnpconfig.hpp):
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
# Googletest                    #
#################################
# Download, configure and build
if(UPNPLIB_WITH_GOOGLETEST)

    # It may already included to install pthreads4w on WIN32
    if(NOT WIN32)
        include(FetchContent)
    endif()

    message(CHECK_START "Download and configuring GoogleTest")

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY    https://github.com/google/googletest.git
        GIT_TAG           origin/main
        #GIT_TAG           release-1.11.0
        GIT_SHALLOW       ON # comment if using GIT_TAG hash below
        #GIT_TAG           2057566e4e16c88f1fea4d6c96b2e2bfb87507a6
    )
    # We have googletest embedded so we turn installation off.
    set (INSTALL_GTEST OFF CACHE BOOL
    "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)" FORCE)

    if(WIN32)

        # Reference: build/_deps/googletest-src/googletest/README.md
        # Must be set before FetchContent_MakeAvailable().
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

        # This caches FETCHCONTENT_BASE_DIR (e.g. ${PROJECT_BINARY_DIR}/_deps)),
        # googletest_SOURCE_DIR, googletest_BINARY_DIR and googletest_POPULATED.
        FetchContent_MakeAvailable(googletest)

        # Due to issue https://github.com/google/googletest/issues/1325#issuecomment-903884914
        # it should only link with the gmock libraries. They contain also gtest.

        message(CHECK_PASS "done")


        ##################################
        # Build Unit Tests on MS Windows #
        ##################################

        message(CHECK_START "configuring Unit Tests")

        # Create symlinks for shared gtest .dlls so the Unit Tests can find them.
        if(BUILD_SHARED_LIBS)
            if(${CMAKE_GENERATOR} MATCHES "Visual Studio")
                # Make Directory is necessary because sub directory ${CMAKE_BUILD_TYPE} isn't available yet.
                file(MAKE_DIRECTORY ${GTESTS_BINARY_DIR}/${CMAKE_BUILD_TYPE})
                file(CREATE_LINK ${PROJECT_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/gtest.dll
                                 $ENV{SystemRoot}/System32/gtest.dll SYMBOLIC)
                file(CREATE_LINK ${PROJECT_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/gmock.dll
                                 $ENV{SystemRoot}/System32/gmock.dll SYMBOLIC)
                file(CREATE_LINK ${PROJECT_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/gtest_main.dll
                                 $ENV{SystemRoot}/System32/gtest_main.dll SYMBOLIC)
                file(CREATE_LINK ${PROJECT_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/gmock_main.dll
                                 $ENV{SystemRoot}/System32/gmock_main.dll SYMBOLIC)
            else()
                file(CREATE_LINK ${PROJECT_BINARY_DIR}/bin/gtest.dll
                                 $ENV{SystemRoot}/System32/gtest.dll SYMBOLIC)
                file(CREATE_LINK ${PROJECT_BINARY_DIR}/bin/gmock.dll
                                 $ENV{SystemRoot}/System32/gmock.dll SYMBOLIC)
                file(CREATE_LINK ${PROJECT_BINARY_DIR}/bin/gtest_main.dll
                                 $ENV{SystemRoot}/System32/gtest_main.dll SYMBOLIC)
                file(CREATE_LINK ${PROJECT_BINARY_DIR}/bin/gmock_main.dll
                                 $ENV{SystemRoot}/System32/gmock_main.dll SYMBOLIC)
            endif()
        endif(BUILD_SHARED_LIBS)

    else(WIN32) # AND UPNPLIB_WITH_GOOGLETEST

        # This caches FETCHCONTENT_BASE_DIR (e.g. ${PROJECT_BINARY_DIR}/_deps)),
        # googletest_SOURCE_DIR, googletest_BINARY_DIR and googletest_POPULATED.
        FetchContent_MakeAvailable(googletest)

        # Due to issue https://github.com/google/googletest/issues/1325#issuecomment-903884914
        # it should only link with the gmock libraries. They contain also gtest.

        message(CHECK_PASS "done")


        ##################################
        # Build default Unit Tests       #
        ##################################

        message(CHECK_START "configuring Unit Tests")
        message(CHECK_PASS "done")

    endif(WIN32)

    # Output with cmake option --log-level=DEBUG
    message(DEBUG "  DEBUG: googletest_SOURCE_DIR is: ${googletest_SOURCE_DIR}")
    message(DEBUG "  DEBUG: googletest_BINARY_DIR is: ${googletest_BINARY_DIR}")

endif(UPNPLIB_WITH_GOOGLETEST)


#################################
# UPnP subdirectories           #
#################################
# This caches <subprojectname>_SOURCE_DIR and <subprojectname_BINARY_DIR
# e.g. PUPNP_UPNP_SOURCE_DIR and PUPNP_UPNP_BINARY_DIR
# For a successful one step configure the order is important.

if(UPNPLIB_WITH_OPENSSL)
    add_subdirectory(${PROJECT_SOURCE_DIR}/openssl)
endif()
add_subdirectory(${PROJECT_SOURCE_DIR}/Umock)
add_subdirectory(${PROJECT_SOURCE_DIR}/Upnplib)
add_subdirectory(${PROJECT_SOURCE_DIR}/Pupnp)
add_subdirectory(${PROJECT_SOURCE_DIR}/Ixml)
add_subdirectory(${PROJECT_SOURCE_DIR}/Compa)
add_subdirectory(${PROJECT_SOURCE_DIR}/sample)
#add_subdirectory(${PROJECT_SOURCE_DIR}/apps)
if(googletest_POPULATED)
    add_subdirectory(${PROJECT_SOURCE_DIR}/Utest)
endif()


#################################
# Export cmake variables        #
#################################
# This should be the last statement
#----------------------------------
# to compiler macros (#define ...) for conditions to the program source.
# Hope we have all needed variables defined so we can write cmake_vars.hpp now.
# To meet all variables this should be the last statement.
# Variablename and its value-placeholder must be the same, e.g.
# #cmakedefine VAR1 ${VAR1}
# #cmakedefine VAR1 ${VAR} does not work.
# #cmakedefine VAR2 1  : VAR2 must be exist before even you set a constant.
configure_file(${PROJECT_SOURCE_DIR}/cmake/cmake_vars.hpp.cmake
               ${PROJECT_BINARY_DIR}/include/upnplib/cmake_vars.hpp
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
message(DEBUG "  DEBUG: --")
message(DEBUG "  DEBUG: UPNPLIB_WITH_OPENSSL = ${UPNPLIB_WITH_OPENSSL}")
message(STATUS "Build type is ${CMAKE_BUILD_TYPE}")


#################################
# Configure and build tests     #
#################################
# To test builds with CTest using different options you can find an example at
# git commit 533548e97765d6eafeafd4a478a48e8ec097f713. These tests have been
# moved to github workflow.
