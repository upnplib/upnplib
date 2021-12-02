// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-12-02

#ifndef UPNP_SYS_SOCKETIF_HPP
#define UPNP_SYS_SOCKETIF_HPP

#include <sys/socket.h>

namespace upnp {

class Bsys_socket {
    // Real class to call the system functions
  public:
    virtual ~Bsys_socket() {}

    virtual int bind(int sockfd, const struct sockaddr* addr,
                     socklen_t addrlen) {
        return ::bind(sockfd, addr, addrlen);
    }

    virtual int listen(int sockfd, int backlog) {
        return ::listen(sockfd, backlog);
    }

    virtual int accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen) {
        return ::accept(sockfd, addr, addrlen);
    }

    virtual int setsockopt(int sockfd, int level, int optname,
                           const void* optval, socklen_t optlen) {
        return ::setsockopt(sockfd, level, optname, optval, optlen);
    }
};

// Global pointer to the current object (real or mocked), will be modified by
// the constructor of the mock object.
extern Bsys_socket* sys_socket_h;

// In the production code you just prefix the old system call with
// 'upnp::sys_socket_h->' so the new call looks like this:
//  upnp::sys_socket_h->bind(..)

/* clang-format off
 * The following class should be copied to the test source. You do not need to
 * copy all MOCK_METHOD, only that are acually used. It is not a good
 * idea to move it here to the header. It uses googletest macros and you always
 * have to compile the code with googletest even for production and not used.

class Mock_sys_socket : public Bsys_socket {
    // Class to mock the free system functions.
    Bsys_socket* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_sys_socket() { m_oldptr = sys_socket_h; sys_socket_h = this; }
    virtual ~Mock_sys_socket() { sys_socket_h = m_oldptr; }

    MOCK_METHOD(int, bind,
                (int sockfd, const struct sockaddr* addr, socklen_t addrlen),
                (override));
    MOCK_METHOD(int, listen, (int sockfd, int backlog), (override));
    MOCK_METHOD(int, accept,
                (int sockfd, struct sockaddr* addr, socklen_t* addrlen),
                (override));
    MOCK_METHOD(int, setsockopt,
                (int sockfd, int level, int optname, const void* optval,
                 socklen_t optlen),
                (override));
};

 * In a gtest you will instantiate the Mock class, prefered as protected member
 * variable at the constructor of the testsuite:

    Mock_sys_socket m_mocked_sys_socket;

 *  and call it with: m_mocked_sys_socket.bind(..)
 * clang-format on
*/

} // namespace upnp

#endif // UPNP_SYS_SOCKETIF_HPP
