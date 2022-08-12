// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-08-17

#ifndef UPNP_SYS_SOCKETIF_HPP
#define UPNP_SYS_SOCKETIF_HPP

// Next should be defined with CMake target_compile_definitions().
// It is intended only to compile with gtests.
#ifdef UPNPLIB_DEBUG_MOCK
#include <iostream>
#endif

#include "UpnpGlobal.hpp" // for EXPORT_SPEC

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#endif

// Different return types for socket functions.

namespace upnplib {

// clang-format off

class Bsys_socket {
    // Real class to call the system functions
  public:
    virtual ~Bsys_socket() {}

    virtual int socket(int domain, int type, int protocol) {
#ifdef UPNPLIB_DEBUG_MOCK
        std::cerr << "DEBUG: call socket(" << domain << ", " << type << ", "
                  << protocol << ")\n";
#endif
        return ::socket(domain, type, protocol);
    }

    virtual int bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
#ifdef UPNPLIB_DEBUG_MOCK
        std::cerr << "DEBUG: call bind(" << sockfd << ", " << addr << ", " << addrlen << ")\n";
#endif
        return ::bind(sockfd, addr, addrlen);
    }

    virtual int listen(int sockfd, int backlog) {
#ifdef UPNPLIB_DEBUG_MOCK
        std::cerr << "DEBUG: call listen(" << sockfd << ", " << backlog << ")\n";
#endif
        return ::listen(sockfd, backlog);
    }

    virtual int accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen) {
#ifdef UPNPLIB_DEBUG_MOCK
        std::cerr << "DEBUG: call accept(" << sockfd << ", " << addr << ", " << addrlen << ")\n";
#endif
        return ::accept(sockfd, addr, addrlen);
    }

    virtual size_t recv(int sockfd, char* buf, size_t len, int flags) {
#ifdef UPNPLIB_DEBUG_MOCK
        std::cerr << "DEBUG: call recv(" << sockfd << ", " << buf << ", " << len << ", " << flags << ")\n";
#endif
        return ::recv(sockfd, buf, len, flags);
    }

    virtual size_t recvfrom(int sockfd, char* buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen) {
#ifdef UPNPLIB_DEBUG_MOCK
        std::cerr << "DEBUG: call recvfrom(" << sockfd << ", " << buf << ", " << len << ", " << flags << ", " << src_addr << ", " << addrlen << ")\n";
#endif
        return ::recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
    }

    virtual size_t send(int sockfd, const char* buf, size_t len, int flags) {
#ifdef UPNPLIB_DEBUG_MOCK
        std::cerr << "DEBUG: call send(" << sockfd << ", " << buf << ", " << len << ", " << flags << ")\n";
#endif
        return ::send(sockfd, buf, len, flags);
    }

    virtual int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
#ifdef UPNPLIB_DEBUG_MOCK
        std::cerr << "DEBUG: call recv(" << sockfd << ", " << addr << ", " << addrlen << ")\n";
#endif
        return ::connect(sockfd, addr, addrlen);
    }

    virtual int getsockopt(int sockfd, int level, int optname, void* optval, socklen_t* optlen) {
#ifdef UPNPLIB_DEBUG_MOCK
        std::cerr << "DEBUG: call getsockopt(" << sockfd << ", " << level << ", " << optname << ", " << optval << ", " << optlen << ")\n";
#endif
#ifdef _WIN32
        return ::getsockopt(sockfd, level, optname, (char*)optval, optlen);
#else
        return ::getsockopt(sockfd, level, optname, optval, optlen);
#endif
    }

    virtual int setsockopt(int sockfd, int level, int optname, const char* optval, socklen_t optlen) {
#ifdef UPNPLIB_DEBUG_MOCK
        std::cerr << "DEBUG: call setsockopt(" << sockfd << ", " << level << ", " << optname << ", " << optval << ", " << optlen << ")\n";
#endif
        return ::setsockopt(sockfd, level, optname, optval, optlen);
    }

    virtual int getsockname(int sockfd, struct sockaddr* addr, socklen_t* addrlen) {
#ifdef UPNPLIB_DEBUG_MOCK
        std::cerr << "DEBUG: call getsockname(" << sockfd << ", " << addr << ", " << addrlen << ")\n";
#endif
        return ::getsockname(sockfd, addr, addrlen);
    }

    virtual int shutdown(int sockfd, int how) {
#ifdef UPNPLIB_DEBUG_MOCK
        std::cerr << "DEBUG: call shutdown(" << sockfd << ", " << how << ")\n";
#endif
        return ::shutdown(sockfd, how);
    }
};

// Global pointer to the current object (real or mocked), will be modified by
// the constructor of the mock object.
EXPORT_SPEC extern Bsys_socket* sys_socket_h;

// In the production code you just prefix the old system call with
// 'upnplib::sys_socket_h->' so the new call looks like this:
//  upnplib::sys_socket_h->bind(..)

/* The following class should be copied to the test source. You do not need to
 * copy all MOCK_METHOD, only that are acually used. It is not a good
 * idea to move it here to the header. It uses googletest macros and you always
 * have to compile the code with googletest even for production and not used.

class Mock_sys_socket : public Bsys_socket {
    // Class to mock the free system functions.
    Bsys_socket* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_sys_socket() { m_oldptr = sys_socket_h; sys_socket_h = this; }
    virtual ~Mock_sys_socket() override { sys_socket_h = m_oldptr; }

    MOCK_METHOD(int, socket,
                (int domain, int type, int protocol),
                (override));
    MOCK_METHOD(int, bind,
                (int sockfd, const struct sockaddr* addr, socklen_t addrlen),
                (override));
    MOCK_METHOD(int, listen, (int sockfd, int backlog),
                (override));
    MOCK_METHOD(int, accept,
                (int sockfd, struct sockaddr* addr, socklen_t* addrlen),
                (override));
    MOCK_METHOD(size_t, recv,
                (int sockfd, char* buf, size_t len, int flags),
                (override));
    MOCK_METHOD(size_t, recvfrom,
                (int sockfd, char* buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen),
                (override));
    MOCK_METHOD(size_t, send,
                (int sockfd, const char* buf, size_t len, int flags),
                (override));
    MOCK_METHOD(int, connect,
                (int sockfd, const struct sockaddr* addr, socklen_t addrlen),
                (override));
    MOCK_METHOD(int, getsockopt,
                (int sockfd, int level, int optname, void* optval, socklen_t* optlen),
                (override));
    MOCK_METHOD(int, setsockopt,
                (int sockfd, int level, int optname, const char* optval, socklen_t optlen),
                (override));
    MOCK_METHOD(int, getsockname,
                (int sockfd, struct sockaddr* addr, socklen_t* addrlen),
                (override));
    MOCK_METHOD(int, shutdown, (int sockfd, int how),
                (override));
};

 * In a gtest you will instantiate the Mock class, prefered as protected member
 * variable at the constructor of the testsuite:

    Mock_sys_socket m_mocked_sys_socket;

 *  and call it with: m_mocked_sys_socket.bind(..)
*/
// clang-format on

} // namespace upnplib

#endif // UPNP_SYS_SOCKETIF_HPP

// vim: nowrap
