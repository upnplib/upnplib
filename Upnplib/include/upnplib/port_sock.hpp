#ifndef UPNPLIB_INCLUDE_PORT_SOCK_HPP
#define UPNPLIB_INCLUDE_PORT_SOCK_HPP
// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-06-14
/*!
 * \file
 * \brief Specifications to be portable with sockets between different
 * platforms.
 * \cond
 * It isn't documented so far.
 */

// clang-format off

// Make sockets portable
// ---------------------
#ifdef _MSC_VER
  #include <fcntl.h>
  #include <winsock2.h>
  #include <iphlpapi.h> // must be after <winsock2.h>
  #include <ws2tcpip.h> // for getaddrinfo, socklen_t etc.
  #include <stdint.h> // for uint16_t etc.

  #define CLOSE_SOCKET_P closesocket

  // _MSC_VER has SOCKET defined but unsigned and not a file descriptor.
  typedef ADDRESS_FAMILY sa_family_t;
  // socklen_t describes the length of a socket address. This is an integer
  // type of at least 32 bits.
  typedef int socklen_t;
  typedef uint16_t in_port_t;
  typedef uint32_t in_addr_t;

  // socket() returns INVALID_SOCKET and is defined unsigned:
  // #define INVALID_SOCKET (0xffff)
  // some winsock functions return SOCKET_ERROR that is defined:
  // #define SOCKET_ERROR (-1)

  // For shutdown() send/receive on a socket there are other constant names.
  #define SHUT_RD SD_RECEIVE
  #define SHUT_WR SD_SEND
  #define SHUT_RDWR SD_BOTH

#else // not _MSC_VER

  #include <sys/socket.h>
  #include <sys/select.h>
  #include <arpa/inet.h>
  #include <unistd.h> // Also needed here to use 'close()' for a socket.
  #include <netdb.h>  // for getaddrinfo etc.

  #define CLOSE_SOCKET_P close

  typedef int SOCKET;
  // Posix has sa_family_t defined.
  // Posix has socklen_t defined.
  // Posix has in_port_t defined.

  // socket() returns INVALID_SOCKET on win32 and is unsigned.
  #define INVALID_SOCKET (-1)
  // some functions return SOCKET_ERROR on win32.
  #define SOCKET_ERROR (-1)
#endif // _MSC_VER


// This a bit flag to disable SIGPIPE on socket connections. With 0 it do
// nothing but we need it on platforms that do not support it, to compile
// without error missing this macro.
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

// On MS Windows there is 'int' used instead of 'sa_family_t' (unsigned short
// int) for some variable. To be portable I simply use
// 'static_cast<sa_family_t>(var)'. This is a compile time guard that the
// smaler boundaries of 'sa_family_t' are not violated.
#if (AF_UNSPEC < 0) || (AF_UNSPEC > 65535) || (AF_INET6 < 0) || (AF_INET6 > 65535) || (AF_INET < 0) || (AF_INET > 65535)
  #error "Constant AF_UNSPEC, or AF_INET6, or AF_INET is negative or bigger than 65535."
#endif

// clang-format on

/// \endcond
#endif // UPNPLIB_INCLUDE_PORT_SOCK_HPP
