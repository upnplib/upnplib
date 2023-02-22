#ifndef UPNPLIB_NET_SOCK_HPP
#define UPNPLIB_NET_SOCK_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-02-22

// Helpful link for ip address structures:
// https://stackoverflow.com/a/16010670/5014688

#include "upnplib/port_sock.hpp"
#include "upnplib/visibility.hpp"
#include <string>
#include <stdexcept>

namespace upnplib {

// Wrapper for a sockaddr structure
// --------------------------------
// This structure simplifies the handling and setting of the different sockaddr
// structures. It does not use any standard library system calls so we do not
/* have to mock anything. For example:
    SockAddr sock;
    sock.addr_set("192.168.1.2", 52345);
    net_order_addr = sock.addr_in->sin_addr.s_addr;
    some_net_func(sock.addr);
*/
struct UPNPLIB_API SockAddr {
    struct sockaddr_storage addr_ss {};
    union { // the pointers to the sockaddr_storage in the union are initialized
            // by the constructor.
        struct sockaddr* addr{};
        struct sockaddr_in* addr_in;
        // struct sockaddr_in6* addr_in6;
    };
    socklen_t addr_len{sizeof addr_ss};

    SockAddr();
    void addr_set(const std::string& a_text_addr, unsigned short a_port = 0);
    std::string addr_get();
    unsigned short addr_get_port();
};

// Derived socketaddr structure
// ----------------------------
// In addition to the properties and methods of the base structure SockAddr we
// can get the socket address from a socket file descriptor and store it in the
// structure. The system call to get the information can be mocked. If you do
// not need information from the socket then it's better to use the base
// structure SockAddr because it does not need to compile in mocking that isn't
// used.
/* Typical calls:
    struct SocketAddr sock;             // For productive system call
    std::string text_addr = sock.addr_get();       // Get stored ip address
    std::string text_addr = sock.addr_get(sockfd); // Get ip address from socket
*/
struct UPNPLIB_API SocketAddr : public SockAddr {
    SocketAddr() = default;
    std::string addr_get();
    std::string addr_get(SOCKET sockfd);
};

} // namespace upnplib

#endif // UPNPLIB_NET_SOCK_HPP
