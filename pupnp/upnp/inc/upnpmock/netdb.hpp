// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-01-18

#ifndef UPNPLIB_NETDBIF_HPP
#define UPNPLIB_NETDBIF_HPP

#ifdef _WIN32
#include <ws2tcpip.h>
#else
#include <netdb.h>
#endif

namespace upnplib {

class Bnetdb {
    // Real class to call the system functions
  public:
    virtual ~Bnetdb() {}

    virtual int getaddrinfo(const char* node, const char* service,
                            const struct addrinfo* hints,
                            struct addrinfo** res) {
        return ::getaddrinfo(node, service, hints, res);
    }
    virtual void freeaddrinfo(struct addrinfo* res) {
        return ::freeaddrinfo(res);
    }
};

// Global pointer to the current object (real or mocked), will be modified by
// the constructor of the mock object.
static Bnetdb netdbObj{};
static Bnetdb* netdb_h = &netdbObj;

// In the production code you just prefix the old system call with
// 'upnplib::netdb_h->' so the new call looks like this:
//  upnplib::netdb_h->getaddrinfo(..)

/* clang-format off
 * The following class should be copied to the test source. You do not need to
 * copy all MOCK_METHOD, only that are acually used. It is not a good
 * idea to move it here to the header. It uses googletest macros and you always
 * have to compile the code with googletest even for production and not used.

class Mock_netdb : public Bnetdb {
    // Class to mock the free system functions.
    Bnetdb* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_netdb() { m_oldptr = netdb_h; netdb_h = this; }
    virtual ~Mock_netdb() { netdb_h = m_oldptr; }

    MOCK_METHOD(int, getaddrinfo, (const char* node, const char* service,
                const struct addrinfo* hints, struct addrinfo** res), (override));
};

 * In a gtest you will instantiate the Mock class, maybe as protected member
 * variable at the constructor of the testsuite:

    Mock_netdb m_mocked_netdb;

 *  and call it with: m_mocked_netdb.getaddrinfo(..)
 * clang-format on
*/

} // namespace upnplib

#endif // UPNPLIB_NETDBIF_HPP
