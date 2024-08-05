#ifndef UPNPLIB_CMAKE_VARS_HPP
#define UPNPLIB_CMAKE_VARS_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-08-04
/*!
 * \file
 * \brief Defines symbols for the compiler that are provided by CMake.
 * \cond
 * It isn't documented so far.
 */

/***************************************************************************
 * CMake configuration settings
 ***************************************************************************/
#cmakedefine CMAKE_VERSION "${CMAKE_VERSION}"
#cmakedefine CMAKE_CXX_COMPILER "${CMAKE_CXX_COMPILER}"
#cmakedefine CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}"
#cmakedefine CMAKE_CXX_COMPILER_VERSION "${CMAKE_CXX_COMPILER_VERSION}"
#cmakedefine CMAKE_GENERATOR "${CMAKE_GENERATOR}"

/***************************************************************************
 * Needed paths of the project
 ***************************************************************************/
// Path to the project directory and its length
#cmakedefine UPNPLIB_PROJECT_SOURCE_DIR "${UPNPLIB_PROJECT_SOURCE_DIR}"
#cmakedefine UPNPLIB_PROJECT_PATH_LENGTH ${UPNPLIB_PROJECT_PATH_LENGTH}
// Path to the build directory of the project
#cmakedefine UPNPLIB_PROJECT_BINARY_DIR "${UPNPLIB_PROJECT_BINARY_DIR}"
// Path to sample source directory to access web subdirectory
#cmakedefine SAMPLE_SOURCE_DIR "${SAMPLE_SOURCE_DIR}"

/***************************************************************************
 * Library version
 ***************************************************************************/
/** The pUPnP library version the fork is based on, e.g. "1.14.19" */
#cmakedefine PUPNP_VERSION_STRING "${PUPNP_VERSION_STRING}"

/***************************************************************************
 * UPNPLIB_PROJECT configuration settings
 ***************************************************************************/
/* Large file support
 * whether the system defaults to 32bit off_t but can do 64bit when requested
 * warning libupnp requires largefile mode - use AC_SYS_LARGEFILE */
#cmakedefine UPNP_LARGEFILE_SENSITIVE
#if defined UPNP_LARGEFILE_SENSITIVE && _FILE_OFFSET_BITS + 0 != 64
#if defined __GNUC__
#warning libupnp requires largefile mode - use AC_SYS_LARGEFILE
#elif !defined _WIN32
#error libupnp requires largefile mode - use AC_SYS_LARGEFILE
#endif
#endif

/***************************************************************************
 * Other settings
 ***************************************************************************/
// Defined to ON if the library will use the static pthreads4W library
#cmakedefine PTW32_STATIC_LIB ${PTW32_STATIC_LIB}

/// \endcond
#endif // UPNPLIB_CMAKE_VARS_HPP
// vim: syntax=cpp
