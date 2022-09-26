#ifndef UPNPLIB_MOCKING_UNISTD_HPP
#define UPNPLIB_MOCKING_UNISTD_HPP
// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-09-25

#include "upnplib/visibility.hpp"

// clang-format off
#ifdef _WIN32
  #include <fcntl.h>
  #include <winsock2.h>
  #define UPNPLIB_CLOSE_SOCKET closesocket
  #define UPNPLIB_SOCKET_TYPE SOCKET
#else
  #include <unistd.h>
  #define UPNPLIB_CLOSE_SOCKET close
  #define UPNPLIB_SOCKET_TYPE int
#endif
// clang-format on

namespace upnplib {
namespace mocking {

class UnistdInterface {
  public:
    virtual ~UnistdInterface() = default;
    virtual int UPNPLIB_CLOSE_SOCKET(UPNPLIB_SOCKET_TYPE fd) = 0;
};

//
// This is the wrapper class for the real (library?) function
// ----------------------------------------------------------
class UnistdReal : public UnistdInterface {
  public:
    virtual ~UnistdReal() override = default;
    int UPNPLIB_CLOSE_SOCKET(UPNPLIB_SOCKET_TYPE fd) override;
};

//
// This is the caller or injector class that injects the class (worker) to be
// used, real or mocked functions.
/* Example:
    UnistdReal unistd_realObj;
    Unistd(&unistd_realObj;
    { // Other scope, e.g. within a gtest
        class UnistdMock { ...; MOCK_METHOD(...) };
        UnistdMock unistd_mockObj;
        Unistd(&unistd_mockObj);
    } // End scope, mock objects are destructed, worker restored to default.
*///------------------------------------------------------------------------
class UPNPLIB_API Unistd {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    Unistd(UnistdReal* a_ptr_realObj);

    // This constructor is used to inject the pointer to the mocking function.
    Unistd(UnistdInterface* a_ptr_mockObj);

    // The destructor is ussed to restore the default pointer.
    virtual ~Unistd();

    virtual int UPNPLIB_CLOSE_SOCKET(UPNPLIB_SOCKET_TYPE fd);

  private:
    // Must be static to be persistent also available on a new constructed
    // object. With inline we do not need an extra definition line outside the
    // class. I also make the symbol hidden so the variable cannot be accessed
    // globaly with Unistd::m_ptr_workerObj.
    UPNPLIB_LOCAL static inline UnistdInterface* m_ptr_workerObj;
    UnistdInterface* m_ptr_oldObj{};
};

extern Unistd UPNPLIB_API unistd_h;

} // namespace mocking
} // namespace upnplib

#endif // UPNPLIB_MOCKING_UNISTD_HPP
