// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-01-19

// Header file for portable definitions
// ====================================
// This header should be includable into any source file to have portable
// definitions available.

#ifndef UPNPLIB_INCLUDE_PORT_HPP
#define UPNPLIB_INCLUDE_PORT_HPP

// clang-format off

// Check Debug settings. Exlusive NDEBUG or DEBUG must be set.
#if defined(NDEBUG) && defined(DEBUG)
  #error "NDBUG and DEBUG are defined. Only one is possible."
#endif
#if !defined(NDEBUG) && !defined(DEBUG)
  #error "Neither NDBUG nor DEBUG is definded."
#endif

//
// C++ visibility support
//-----------------------
// Reference: https://gcc.gnu.org/wiki/Visibility
// Generic helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
  #define UPNP_HELPER_DLL_IMPORT __declspec(dllimport)
  #define UPNP_HELPER_DLL_EXPORT __declspec(dllexport)
  #define UPNP_HELPER_DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define UPNP_HELPER_DLL_IMPORT __attribute__ ((visibility ("default")))
    #define UPNP_HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
    #define UPNP_HELPER_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define UPNP_HELPER_DLL_IMPORT
    #define UPNP_HELPER_DLL_EXPORT
    #define UPNP_HELPER_DLL_LOCAL
  #endif
#endif

// Now we use the generic helper definitions above to define UPNP_API and
// UPNP_LOCAL. UPNP_API is used for the public API symbols. It either DLL imports
// or DLL exports (or does nothing for static build) UPNP_LOCAL is used for
// non-api symbols.

#ifdef UPNP_DLL // defined if UPNP is compiled as a DLL
  #ifdef UPNP_DLL_EXPORTS // defined if we are building the UPNP DLL (instead of using it)
    #define UPNP_API UPNP_HELPER_DLL_EXPORT
  #else
    #define UPNP_API UPNP_HELPER_DLL_IMPORT
  #endif // UPNP_DLL_EXPORTS
  #define UPNP_LOCAL UPNP_HELPER_DLL_LOCAL
#else // UPNP_DLL is not defined: this means UPNP is a static lib.
  #define UPNP_API
  #define UPNP_LOCAL
#endif // UPNP_DLL

// clang-format on

// Header file for portable <unistd.h>
// -----------------------------------
// On MS Windows <unistd.h> isn't availabe. We can use <io.h> instead for most
// functions but it's not 100% compatible.

#if _WIN32
#include <fcntl.h>
#include <io.h>
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#else // WIN32
#include <unistd.h>
#endif // WIN32

#ifdef _MSC_VER
// no ssize_t defined for VC
#include <stdint.h>
#ifdef _WIN64
typedef int64_t ssize_t;
#else
typedef int32_t ssize_t;
#endif
#endif

#endif // UPNPLIB_INCLUDE_PORT_HPP
