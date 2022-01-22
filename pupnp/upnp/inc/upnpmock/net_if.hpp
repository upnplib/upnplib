// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-01-22

#ifndef _WIN32

#ifndef UPNPLIB_NETIFIF_HPP
#define UPNPLIB_NETIFIF_HPP

#include <net/if.h>

namespace upnplib {

class Bnet_if {
    // Real class to call the system functions
  public:
    virtual ~Bnet_if() {}

    // With the constructor initialize the pointer to the interface that may be
    // overwritten to point to a mock object instead.
    // Bnet_if() { net_if_h = this; }

    virtual unsigned int if_nametoindex(const char* ifname) {
        return ::if_nametoindex(ifname);
    }
};

// Global pointer to the current object (real or mocked), will be modified by
// the constructor of the mock object.
extern Bnet_if* net_if_h;

// In the production code you just prefix the old system call with
// 'upnplib::net_if_h->' so the new call looks like this:
//  upnplib::net_if_h->if_nametoindex("eth0")

/* clang-format off
 * The following class should be copied to the test source. You do not need to
 * copy all MOCK_METHOD, only that are acually used. It is not a good
 * idea to move it here to the header. It uses googletest macros and you always
 * have to compile the code with googletest even for production and not used.

class Mock_net_if : public Bnet_if {
    // Class to mock the free system functions.
    Bnet_if* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_net_if() { m_oldptr = net_if_h; net_if_h = this; }
    virtual ~Mock_net_if() { net_if_h = m_oldptr; }

    MOCK_METHOD(unsigned int, if_nametoindex, (const char* ifname), (override));
};

 * In a gtest you will instantiate the Mock class, maybe as protected member
 * variable at the constructor of the testsuite:

    Mock_net_if m_mocked_net_if;

 * and call it with: m_mocked_net_if.if_nametoindex(..)
 * clang-format on
*/

} // namespace upnplib

#endif // UPNPLIB_NETIFIF_HPP
#endif // not _WIN32
