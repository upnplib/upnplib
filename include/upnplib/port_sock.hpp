#ifndef UPNPLIB_INCLUDE_PORT_SOCK_HPP
#define UPNPLIB_INCLUDE_PORT_SOCK_HPP
// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-06-21

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

  // For shutdown() send/receive on a socket there are other constant names.
  #define SHUT_RD SD_RECEIVE
  #define SHUT_WR SD_SEND
  #define SHUT_RDWR SD_BOTH

#else

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
#endif

// clang-format on

#endif // UPNPLIB_INCLUDE_PORT_SOCK_HPP
