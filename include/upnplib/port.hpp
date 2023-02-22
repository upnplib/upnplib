#ifndef UPNPLIB_INCLUDE_PORT_HPP
#define UPNPLIB_INCLUDE_PORT_HPP
// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-02-03

// Header file for portable definitions
// ====================================
// This header should be includable into any source file to have portable
// definitions available.

// clang-format off

// Check Debug settings. Exlusive NDEBUG or DEBUG must be set.
#if defined(NDEBUG) && defined(DEBUG)
  #error "NDBUG and DEBUG are defined. Only one is possible."
#endif
#if !defined(NDEBUG) && !defined(DEBUG)
  #error "Neither NDBUG nor DEBUG is definded."
#endif

// Header file for portable <unistd.h>
// -----------------------------------
// On MS Windows <unistd.h> isn't available. We can use <io.h> instead for most
// functions but it's not 100% compatible.
#ifdef _MSC_VER
  #include <io.h>
  #define STDIN_FILENO 0
  #define STDOUT_FILENO 1
  #define STDERR_FILENO 2
#else
  #include <unistd.h>
#endif

// Make size_t and ssize_t portable
// --------------------------------
// no ssize_t defined for VC but SSIZE_T
#ifdef _MSC_VER
  #ifndef UPNPLIB_WITH_NATIVE_PUPNP
    // This conflicts with definition of ssize_t in pupnp/UpnpStdInt.hpp.
    #include <BaseTsd.h> // for SSIZE_T
    #define ssize_t SSIZE_T
  #endif
  // Needed for some uncompatible arguments on MS Windows.
  #define SIZEP_T int
  #define SSIZEP_T int
#else
  #define SIZEP_T size_t
  #define SSIZEP_T ssize_t
#endif

/*!
 * \brief Declares an inline function.
 *
 * Surprisingly, there are some compilers that do not understand the
 * inline keyword. This definition makes the use of this keyword
 * portable to these systems.
 */
#ifdef __STRICT_ANSI__
  #define UPNPLIB_INLINE __inline__
#else
  #define UPNPLIB_INLINE inline
#endif

// This compiles tracing into the source code. Once compiled in you can
// disable TRACE with
// std::clog.setstate(std::ios_base::failbit);
// and enable with
// std::clog.clear();
#ifdef UPNPLIB_WITH_TRACE
  #include <iostream>
  #define TRACE(s) std::clog<<"TRACE: "<<(s)
#else
  #define TRACE(s)
#endif

// clang-format on

#endif // UPNPLIB_INCLUDE_PORT_HPP
