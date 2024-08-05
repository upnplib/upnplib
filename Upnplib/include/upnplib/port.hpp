#ifndef UPNPLIB_INCLUDE_PORT_HPP
#define UPNPLIB_INCLUDE_PORT_HPP
// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-03-04
/*!
 * \file
 * \brief Specifications to be portable between different platforms.
 * \cond
 * It isn't documented so far.
 */

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
  #define strncasecmp strnicmp
#endif

// Some different format specifications for printf() and friends
// -------------------------------------------------------------
#ifdef _MSC_VER
  #define PRIzu "lu"
  #define PRIzx "lx"
#else
  #define PRIzu "zu"
  #define PRIzx "zx"
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

// Warning 4273: 'function' : inconsistent DLL linkage.
// This is expected on propagate global variables with included header file
// (__declspec(dllimport)) in its source file (__declspec(dllexport)).
#ifdef _MSC_VER
  #define SUPPRESS_MSVC_WARN_4273_NEXT_LINE \
    _Pragma("warning(suppress: 4273)")
#else
  #define SUPPRESS_MSVC_WARN_4273_NEXT_LINE
#endif

#ifdef _MSC_VER
  #define DISABLE_MSVC_WARN_4273 \
    _Pragma("warning(push)") \
    _Pragma("warning(disable: 4273)")
#else
  #define DISABLE_MSVC_WARN_4273
#endif

#ifdef _MSC_VER
  #define ENABLE_MSVC_WARN \
    _Pragma("warning(pop)")
#else
  #define ENABLE_MSVC_WARN
#endif

// clang-format on


// This has been taken from removed Compa/src/inc/upnputil.hpp
/* C specific */
/* VC needs these in C++ mode too (do other compilers?) */
#if !defined(__cplusplus) || defined(UPNP_USE_MSVCPP)
#ifdef _WIN32
#ifndef S_ISREG
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif
#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif
#define sleep(a) Sleep((a) * 1000)
#define usleep(a) Sleep((a) / 1000)
#define strerror_r(a, b, c) (strerror_s((b), (c), (a)))
#endif /* _WIN32 */
#endif /* !defined(__cplusplus) || defined(UPNP_USE_MSVCPP) */

/// \endcond
#endif // UPNPLIB_INCLUDE_PORT_HPP
