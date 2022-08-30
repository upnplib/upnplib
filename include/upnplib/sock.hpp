#ifndef UPNPLIB_NET_SOCK_HPP
#define UPNPLIB_NET_SOCK_HPP
// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-08-30

// Helpful link for ip address structures:
// https://stackoverflow.com/a/16010670/5014688

#include "upnplib/visibility.hpp" // for UPNPLIB_API
#include <string>
#include <stdexcept>

#ifdef _WIN32
#include <winsock2.h>
#include <iphlpapi.h> // must be after <winsock2.h>
#include <ws2tcpip.h> // for socklen_t etc.
// #include <ws2ipdef.h>

#else // _WIN32

#include <sys/socket.h>
#include <arpa/inet.h>

/*! This typedef makes the code slightly more WIN32 tolerant.
 * On WIN32 systems, SOCKET is unsigned and is not a file
 * descriptor. */
typedef int SOCKET;

/*! socket() returns INVALID_SOCKET on win32 and is unsigned. */
#define INVALID_SOCKET (-1)

#endif // _WIN32

namespace upnplib {

// Wrapper for mocking sys/socket.h standard library calls
// -------------------------------------------------------
class ISysSocket {
  public:
    virtual ~ISysSocket() = default;
    virtual int getsockname(int sockfd, struct sockaddr* addr,
                            socklen_t* addrlen) = 0;
};

class SysSocket : public ISysSocket {
  public:
    int getsockname(int sockfd, struct sockaddr* addr,
                    socklen_t* addrlen) override {
        return ::getsockname(sockfd, addr, addrlen);
    }
};

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
// In addition to the properties and methods of the base class SockAddr we can
// get the socket address from a socket file descriptor and store it in the
// structure. The system call to get the information can be mocked by injection
// of the mock object with the second constructor. With the default constructor
// we use the initialized object pointer to the real library function.
/* Typical calls:
    struct SocketAddr sock;             // For productive system call
    struct SocketAddr sock(&mockedObj); // For mocking injection
    std::string text_addr = sock.addr_get();       // Get stored ip address
    std::string text_addr = sock.addr_get(sockfd); // Get ip address from socket
*/
struct UPNPLIB_API SocketAddr : public SockAddr {
    SocketAddr() = default;
    SocketAddr(ISysSocket* a_sys_socketObj);
    std::string addr_get();
    std::string addr_get(SOCKET sockfd);

  private:
    class SysSocket sys_socketObj;
    class ISysSocket* m_sys_socketObj{&sys_socketObj};
};

} // namespace upnplib

#endif // UPNPLIB_NET_SOCK_HPP
