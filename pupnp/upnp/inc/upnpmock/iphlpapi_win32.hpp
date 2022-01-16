// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-01-17

// iphlpapi.h is a Microsoft Windows library.
#ifdef _WIN32

#ifndef UPNPLIB_IPHLPAPI_WIN32IF_HPP
#define UPNPLIB_IPHLPAPI_WIN32IF_HPP

#include <winsock2.h>
#include <iphlpapi.h>

namespace upnplib {

class Biphlpapi {
    // Real class to call the system functions
  public:
    virtual ~Biphlpapi() {}

    virtual ULONG GetAdaptersAddresses(ULONG Family, ULONG Flags,
                                       PVOID Reserved,
                                       PIP_ADAPTER_ADDRESSES AdapterAddresses,
                                       PULONG SizePointer) {
        return ::GetAdaptersAddresses(Family, Flags, Reserved, AdapterAddresses,
                                      SizePointer);
    }
};

// Global pointer to the current object (real or mocked), will be modified by
// the constructor of the mock object.
extern Biphlpapi* iphlpapi_h;

// In the production code you just prefix the old system call with
// 'upnplib::iphlpapi_h->' so the new call looks like this:
//  upnplib::iphlpapi_h->GetAdaptersAddresses(..)

/* clang-format off
 * The following class should be copied to the test source. You do not need to
 * copy all MOCK_METHOD, only that are acually used. It is not a good
 * idea to move it here to the header. It uses googletest macros and you always
 * have to compile the code with googletest even for production and not used.

class Mock_iphlpapi : public Biphlpapi {
    // Class to mock the free system functions.
    Biphlpapi* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_iphlpapi() { m_oldptr = iphlpapi_h; iphlpapi_h = this; }
    virtual ~Mock_iphlpapi() { iphlpapi_h = m_oldptr; }

    MOCK_METHOD(ULONG, GetAdaptersAddresses, (ULONG Family, ULONG Flags, PVOID Reserved, PIP_ADAPTER_ADDRESSES AdapterAddresses, PULONG SizePointer), (override));
};

 * In a gtest you will instantiate the Mock class, prefered as protected member
 * variable at the constructor of the testsuite:

    Mock_iphlpapi m_mocked_iphlpapi;

 *  and call it with: m_mocked_iphlpapi.GetAdaptersAddresses(..)
 * clang-format on
*/

} // namespace upnplib

#endif // UPNPLIB_IPHLPAPI_WIN32IF_HPP
#endif // _WIN32
