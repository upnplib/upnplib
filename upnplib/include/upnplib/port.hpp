#ifndef UPNPLIB_INCLUDE_PORT_HPP
#define UPNPLIB_INCLUDE_PORT_HPP
// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-11-17

// Header file for portable definitions
// ====================================
// This header should be includable into any source file to have portable
// definitions available. It should not build any inline code.

// clang-format off

// Check Debug settings. Exlusive NDEBUG or DEBUG must be set.
// -----------------------------------------------------------
#if defined(NDEBUG) && defined(DEBUG)
  #error "NDEBUG and DEBUG are defined. Only one is possible."
#endif
#if !defined(NDEBUG) && !defined(DEBUG)
  #error "Neither NDEBUG nor DEBUG is definded."
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
  #include <BaseTsd.h> // for SSIZE_T
   #define ssize_t SSIZE_T
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

#ifdef _MSC_VER
  // POSIX names for functions
  #define strcasecmp _stricmp
#endif


// Macros to disable and enable compiler warnings
// ----------------------------------------------
// Warning 4251: 'type' : class 'type1' needs to have dll-interface to be used
// by clients of class 'type2'.
// This can be ignored for classes from the C++ STL (best if it is private).
#ifdef _MSC_VER
  #define SUPPRESS_MSVC_WARN_4251_NEXT_LINE \
    _Pragma("warning(suppress: 4251)")
#else
  #define SUPPRESS_MSVC_WARN_4251_NEXT_LINE
#endif

#ifdef _MSC_VER
  #define DISABLE_MSVC_WARN_4251 \
    _Pragma("warning(push)") \
    _Pragma("warning(disable: 4251)")
#else
  #define DISABLE_MSVC_WARN_4251
#endif

#ifdef _MSC_VER
  #define ENABLE_MSVC_WARN \
    _Pragma("warning(pop)")
#else
  #define ENABLE_MSVC_WARN
#endif

// clang-format on

#endif // UPNPLIB_INCLUDE_PORT_HPP
