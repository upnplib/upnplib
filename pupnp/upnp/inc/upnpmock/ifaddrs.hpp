// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-05-17

#ifndef _WIN32

#ifndef UPNPLIB_IFADDRSIF_HPP
#define UPNPLIB_IFADDRSIF_HPP

#include "UpnpGlobal.hpp" // for EXPORT_SPEC
#include <ifaddrs.h>

namespace upnplib {

class Bifaddrs {
    // Real class to call the system functions
  public:
    virtual ~Bifaddrs() {}

    virtual int getifaddrs(struct ifaddrs** ifap) { return ::getifaddrs(ifap); }
    virtual void freeifaddrs(struct ifaddrs* ifa) { return ::freeifaddrs(ifa); }
};

// Global pointer to the current object (real or mocked), will be modified by
// the constructor of the mock object.
EXPORT_SPEC extern Bifaddrs* ifaddrs_h;

// In the production code you just prefix the old system call with
// 'upnplib::ifaddrs_h->' so the new call looks like this:
//  upnplib::ifaddrs_h->getifaddrs(..)

/* clang-format off
 * The following class should be copied to the test source. You do not need to
 * copy all MOCK_METHOD, only that are acually used. It is not a good
 * idea to move it here to the header. It uses googletest macros and you always
 * have to compile the code with googletest even for production and not used.

class Mock_ifaddrs : public Bifaddrs {
    // Class to mock the free system functions.
    Bifaddrs* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_ifaddrs() { m_oldptr = ifaddrs_h; ifaddrs_h = this; }
    virtual ~Mock_ifaddrs() { ifaddrs_h = m_oldptr; }

    MOCK_METHOD(int, getifaddrs, (ifaddrs** ifapint errnum), (override));
};

 * In a gtest you will instantiate the Mock class, maybe as protected member
 * variable at the constructor of the testsuite:

    Mock_ifaddrs m_mocked_ifaddrs;

 *  and call it with: m_mocked_ifaddrs.strerror(..)
 * clang-format on
*/

} // namespace upnplib

#endif // UPNPLIB_IFADDRSIF_HPP
#endif // not _WIN32
