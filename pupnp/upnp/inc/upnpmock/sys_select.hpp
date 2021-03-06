// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-05-17

#ifndef UPNPLIB_SYS_SELECTIF_HPP
#define UPNPLIB_SYS_SELECTIF_HPP

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/select.h>
#endif

namespace upnplib {

class Bsys_select {
    // Real class to call the system functions
  public:
    virtual ~Bsys_select() {}

    virtual int select(int nfds, fd_set* readfds, fd_set* writefds,
                       fd_set* exceptfds, struct timeval* timeout) {
        return ::select(nfds, readfds, writefds, exceptfds, timeout);
    }
};

// Global pointer to the current object (real or mocked), will be modified by
// the constructor of the mock object.
EXPORT_SPEC extern Bsys_select* sys_select_h;

// In the production code you just prefix the old system call with
// 'upnplib::sys_select_h->' so the new call looks like this:
//  upnplib::sys_select_h->select(..)

/* clang-format off
 * The following class should be copied to the test source. You do not need to
 * copy all MOCK_METHOD, only that are acually used. It is not a good
 * idea to move it here to the header. It uses googletest macros and you always
 * have to compile the code with googletest even for production and not used.

class Mock_sys_select : public Bsys_select {
    // Class to mock the free system functions.
    Bsys_select* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_sys_select() { m_oldptr = sys_select_h; sys_select_h = this; }
    virtual ~Mock_sys_select() override { sys_select_h = m_oldptr; }

    MOCK_METHOD(int, select,
                (int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds,
                 struct timeval* timeout),
                (override));
};

 * In a gtest you will instantiate the Mock class, maybe as protected member
 * variable at the constructor of the testsuite:

    Mock_sys_select m_mocked_sys_select;

 *  and call it with: m_mocked_sys_select.select(..)
 * clang-format on
*/

} // namespace upnplib

#endif // UPNPLIB_SYS_SELECTIF_HPP
