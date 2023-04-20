#ifndef UPNPLIB_NET_SOCK_HPP
#define UPNPLIB_NET_SOCK_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-04-26

// Helpful link for ip address structures:
// https://stackoverflow.com/a/16010670/5014688

#include <upnplib/port_sock.hpp>
#include <upnplib/visibility.hpp>
#include <string>
#include <stdexcept>

namespace upnplib {

// Free function to get the port number from a string
// --------------------------------------------------
UPNPLIB_API uint16_t to_port(const std::string& a_port_str);


// Specialized sockaddr_structure derived from system ::sockaddr_structure
// -----------------------------------------------------------------------
struct UPNPLIB_API sockaddr_storage : public ::sockaddr_storage {

    // Pointer to the inherited sockaddr_structure for simplified access.
    ::sockaddr_storage* ss = (::sockaddr_storage*)&ss_family;
    ::sockaddr_in6* sin6 = (::sockaddr_in6*)&ss_family;
    ::sockaddr_in* sin = (::sockaddr_in*)&ss_family;

    // Constructor
    sockaddr_storage();

    // Constructor with socket address initialization.
    sockaddr_storage(const std::string& a_addr_str, uint16_t a_port);

    // Destructor
    virtual ~sockaddr_storage();

    // Assignment operator to set socket address from string,
    // e.g.: sockaddr_storage ss; ss = "192.168.100.1";
    // or with port: ss = "[2001:db8::1]:50001".
    void operator=(const std::string& a_addr_str);

    // Getter for the assosiated ip address without port, e.g.
    // "[2001:db8::2]" or "192.168.254.253".
    std::string get_addr_str() const;

    // Getter for the numeric port.
    uint16_t get_port() const;

  private:
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif
    std::string m_addr_str; // input string without brackets
#ifdef _MSC_VER
#pragma warning(pop)
#endif
    uint16_t m_port;

    UPNPLIB_LOCAL void handle_ipv6();
    UPNPLIB_LOCAL void handle_ipv6_with_port();
    UPNPLIB_LOCAL void handle_ipv4();
    UPNPLIB_LOCAL void handle_ipv4_with_port();
    UPNPLIB_LOCAL void handle_port();
};


// Wrapper for a sockaddr structure
// --------------------------------
// This structure simplifies the handling and setting of the different sockaddr
// structures. It does not use any standard library system calls so we do not
/* have to mock anything. For example:
    SockAddr sock;
    sock.addr_set("192.168.1.2", 52345);
    net_order_addr = sock.addr_in->sin_addr.s_addr;
    some_net_func(soc);
*/
struct UPNPLIB_API SockAddr {
    struct ::sockaddr_storage addr_ss {};
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
