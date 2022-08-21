#ifndef UPNPLIB_NET_SOCKET_HPP
#define UPNPLIB_NET_SOCKET_HPP
// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-08-21

#include "upnplib/port.hpp" // for UPNPLIB_API
#include <string>
#include <stdexcept>

#ifdef _WIN32
#include <winsock2.h>
#include <iphlpapi.h> // must be after <winsock2.h>
#include <ws2tcpip.h> // for socklen_t etc.
// #include <ws2ipdef.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

namespace upnplib {

// Wrapper for a sockaddr structure
// --------------------------------
// This structure simplifies the handling and setting of the different sockaddr
// structures. For example:
// SockAddr sock;
// sock.addr_set("192.168.1.2", 52345)
// net_order_addr = sock.addr_in->sin_addr.s_addr
// some_net_func(sock.addr);

struct UPNPLIB_API SockAddr {
    struct sockaddr_storage addr_ss {};
    union { // the union is initialized by the constructor
        struct sockaddr* addr{};
        struct sockaddr_in* addr_in;
        // struct sockaddr_in6* addr_in6;
    };

    SockAddr();
    void addr_set(const std::string& a_text_addr, unsigned short a_port = 0);
    std::string addr_get();
    unsigned short addr_get_port();

  private:
    char buf_ntop[INET6_ADDRSTRLEN]{};
};

} // namespace upnplib

#endif // UPNPLIB_NET_SOCKET_HPP
