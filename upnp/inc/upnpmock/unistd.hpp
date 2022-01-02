// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-01-07

#ifndef UPNP_UNISTDIF_HPP
#define UPNP_UNISTDIF_HPP

// clang-format off
#ifdef _WIN32
  #include <fcntl.h>
  #include <winsock2.h>
  #define UPNP_CLOSE_SOCKET closesocket
  #define UPNP_SOCKET_TYPE SOCKET
#else
  #include <unistd.h>
  #define UPNP_CLOSE_SOCKET close
  #define UPNP_SOCKET_TYPE int
#endif
// clang-format on

//
namespace upnp {

class Bunistd {
    // Real class to call the system functions
  public:
    virtual ~Bunistd() {}

    virtual int UPNP_CLOSE_SOCKET(UPNP_SOCKET_TYPE fd) {
        return ::UPNP_CLOSE_SOCKET(fd);
    }
};

// Global pointer to the current object (real or mocked), will be modified by
// the constructor of the mock object.
extern Bunistd* unistd_h;

// In the production code you just prefix the old system call with
// 'upnp::unistd_h->' so the new call looks like this:
//  upnp::unistd_h->close(fd);

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

    MOCK_METHOD(int, UPNP_CLOSE_SOCKET, (UPNP_SOCKET_TYPE fd), (override));
};

 * In a gtest you will instantiate the Mock class, prefered as protected member
 * variable at the constructor of the testsuite:

    Mock_unistd m_mocked_unistd;

 *  and call it with: m_mocked_unistd.close(fd)
 * clang-format on
*/

} // namespace upnp

#endif // UPNP_UNISTDIF_HPP
