#ifndef UPNPLIB_INCLUDE_PORT_HPP
#define UPNPLIB_INCLUDE_PORT_HPP
// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-08-21

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
// functions but it's not 100% compatible. We also need <winsock2.h> for
// closesocket instead of <unistd.h> for close that also closes a socket on
// unix.
#ifdef _WIN32
  #include <fcntl.h>
  #include <winsock2.h>
  #include <io.h>
  #define STDIN_FILENO 0
  #define STDOUT_FILENO 1
  #define STDERR_FILENO 2
  #define stat _stat

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

// clang-format on

#endif // UPNPLIB_INCLUDE_PORT_HPP
