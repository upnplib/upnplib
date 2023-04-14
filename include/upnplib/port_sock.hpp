#ifndef UPNPLIB_INCLUDE_PORT_SOCK_HPP
#define UPNPLIB_INCLUDE_PORT_SOCK_HPP
// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-03-15

// clang-format off

// Make sockets portable
// ---------------------
#ifdef _MSC_VER
  #include <fcntl.h>
  #include <winsock2.h>
  #include <iphlpapi.h> // must be after <winsock2.h>
  #include <ws2tcpip.h> // for getaddrinfo, socklen_t etc.

  // _MSC_VER has SOCKET defined but unsigned and not a file descriptor.
  #define sa_family_t ADDRESS_FAMILY
  #define CLOSE_SOCKET_P closesocket

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

  // This typedef makes the code slightly more WIN32 tolerant. On WIN32 systems,
  // SOCKET is unsigned and is not a file descriptor.
  typedef int SOCKET;
  // Posix has sa_family_t defined.
  #define CLOSE_SOCKET_P close

  // socket() returns INVALID_SOCKET on win32 and is unsigned.
  #define INVALID_SOCKET (-1)
  // some functions return SOCKET_ERROR on win32.
  #define SOCKET_ERROR (-1)
#endif

// clang-format on

#endif // UPNPLIB_INCLUDE_PORT_SOCK_HPP
