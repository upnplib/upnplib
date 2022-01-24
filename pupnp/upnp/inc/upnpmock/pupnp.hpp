// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-01-19

#ifndef UPNPLIB_PUPNPIF_HPP
#define UPNPLIB_PUPNPIF_HPP

#include "sock.hpp"

static int Check_Connect_And_Wait_Connection(SOCKET sock, int connect_res);
static int private_connect(SOCKET sockfd, const struct sockaddr* serv_addr,
                           socklen_t addrlen);

namespace upnplib {

class Bpupnp {
    // Real class to call the system functions
  public:
    virtual ~Bpupnp() {}

    // virtual char* strerror(int errnum) { return ::strerror(errnum); }
    virtual int sock_make_no_blocking(SOCKET sock) {
        return ::sock_make_no_blocking(sock);
    }
    virtual int sock_make_blocking(SOCKET sock) {
        return ::sock_make_blocking(sock);
    }
    virtual int Check_Connect_And_Wait_Connection(SOCKET sock,
                                                  int connect_res) {
        return ::Check_Connect_And_Wait_Connection(sock, connect_res);
    }
    virtual int private_connect(SOCKET sockfd, const struct sockaddr* serv_addr,
                                socklen_t addrlen) {
        return ::private_connect(sockfd, serv_addr, addrlen);
    }
};

// Global pointer to the current object (real or mocked), will be modified by
// the constructor of the mock object.
static upnplib::Bpupnp pupnpObj{};
static upnplib::Bpupnp* pupnp = &pupnpObj;

// In the production code you just prefix the old system call with
// 'upnplib::pupnp->' so the new call looks like this:
//  upnplib::pupnp->sock_make_no_blocking(socket)

/* clang-format off
 * The following class should be copied to the test source. You do not need to
 * copy all MOCK_METHOD, only that are acually used. It is not a good
 * idea to move it here to the header. It uses googletest macros and you always
 * have to compile the code with googletest even for production and not used.

class Mock_pupnp : public Bpupnp {
    // Class to mock the free system functions.
    Bpupnp* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_pupnp() { m_oldptr = pupnp; pupnp = this; }
    virtual ~Mock_pupnp() override { pupnp = m_oldptr; }

    MOCK_METHOD(int, sock_make_no_blocking, (SOCKET sock), (override));
    MOCK_METHOD(int, sock_make_blocking, (SOCKET sock), (override));
    MOCK_METHOD(int, Check_Connect_And_Wait_Connection, (SOCKET sock, int connect_res), (override));
    MOCK_METHOD(int, private_connect, (SOCKET sockfd, const struct sockaddr* serv_addr, socklen_t addrlen), (override));
};

 * In a gtest you will instantiate the Mock class, maybe as protected member
 * variable at the constructor of the testsuite:

    Mock_pupnp m_mocked_pupnp;

 *  and call it with: m_mocked_pupnp.sock_make_no_blocking(..)
 * clang-format on
*/

} // namespace upnplib

#endif // UPNPLIB_PUPNPIF_HPP
