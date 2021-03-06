// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-06-16

#ifndef UPNPLIB_UNISTDIF_HPP
#define UPNPLIB_UNISTDIF_HPP

// clang-format off
#ifdef _WIN32
  #include <fcntl.h>
  #include <winsock2.h>
  #define PUPNP_CLOSE_SOCKET closesocket
  #define PUPNP_SOCKET_TYPE SOCKET
#else
  #include <unistd.h>
  #define PUPNP_CLOSE_SOCKET close
  #define PUPNP_SOCKET_TYPE int
#endif
// clang-format on

//
namespace upnplib {

class Bunistd {
    // Real class to call the system functions
  public:
    virtual ~Bunistd() {}

    virtual int PUPNP_CLOSE_SOCKET(PUPNP_SOCKET_TYPE fd) {
        return ::PUPNP_CLOSE_SOCKET(fd);
    }
};

// Global pointer to the current object (real or mocked), will be modified by
// the constructor of the mock object.
EXPORT_SPEC extern Bunistd* unistd_h;

// In the production code you just prefix the old system call with
// 'upnplib::unistd_h->' so the new call looks like this:
//  upnplib::unistd_h->close(fd);

/* clang-format off
 * The following class should be copied to the test source. You do not need to
 * copy all MOCK_METHOD, only that are acually used. It is not a good
 * idea to move it here to the header. It uses googletest macros and you always
 * have to compile the code with googletest even for production and not used.

class Mock_unistd : public Bunistd {
    // Class to mock the free system functions.
    Bunistd* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_unistd() { m_oldptr = unistd_h; unistd_h = this; }
    virtual ~Mock_unistd() override { unistd_h = m_oldptr; }

    MOCK_METHOD(int, PUPNP_CLOSE_SOCKET, (PUPNP_SOCKET_TYPE fd), (override));
};

 * In a gtest you will instantiate the Mock class, maybe as protected member
 * variable at the constructor of the testsuite:

    Mock_unistd m_mocked_unistd;

 *  and call it with: m_mocked_unistd.close(fd)
 * clang-format on
*/

} // namespace upnplib

#endif // UPNPLIB_UNISTDIF_HPP
