// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-01-16

// winsock2.h is a Microsoft Windows library.
#ifdef _WIN32

#ifndef UPNPLIB_WINSOCK2IF_HPP
#define UPNPLIB_WINSOCK2IF_HPP

#include <winsock2.h>

namespace upnplib {

class Bwinsock2 {
    // Real class to call the system functions
  public:
    virtual ~Bwinsock2() {}

    virtual int WSAGetLastError() { return ::WSAGetLastError(); }
};

// Global pointer to the current object (real or mocked), will be modified by
// the constructor of the mock object.
extern Bwinsock2* winsock2_h;

// In the production code you just prefix the old system call with
// 'upnplib::winsock2_h->' so the new call looks like this:
//  upnplib::winsock2_h->WSAGetLastError()

/* clang-format off
 * The following class should be copied to the test source. You do not need to
 * copy all MOCK_METHOD, only that are acually used. It is not a good
 * idea to move it here to the header. It uses googletest macros and you always
 * have to compile the code with googletest even for production and not used.

class Mock_winsock2 : public Bwinsock2 {
    // Class to mock the free system functions.
    Bwinsock2* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_winsock2() { m_oldptr = winsock2_h; winsock2_h = this; }
    virtual ~Mock_winsock2() override { winsock2_h = m_oldptr; }

    MOCK_METHOD(int, WSAGetLastError, (), (override));
};

 * In a gtest you will instantiate the Mock class, maybe as protected member
 * variable at the constructor of the testsuite:

    Mock_winsock2 m_mocked_winsock2;

 *  and call it with: m_mocked_winsock2.WSAGetLastError()
 * clang-format on
*/

} // namespace upnplib

#endif // UPNPLIB_WINSOCK2IF_HPP
#endif // _WIN32
