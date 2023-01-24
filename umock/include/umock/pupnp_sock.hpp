#ifndef UPNPLIB_PUPNP_SOCK_HPP
#define UPNPLIB_PUPNP_SOCK_HPP
// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-01-26

// This is a header only mocking include file. When included it is present
// direct in the source code and can be used to mock static functions that are
// hidden to the global context. With some returns and pointer variables this
// include does not consume many resources. But still, it's better for the
// global context with exported symbols to use a header + sorce file compile
// unit as shown in other mocking files.

#include "upnplib/port.hpp"

int sock_make_no_blocking(SOCKET sock);
int sock_make_blocking(SOCKET socks);

namespace umock {

class PupnpSockInterface {
  public:
    virtual ~PupnpSockInterface() = default;
    // clang-format off
    virtual int sock_make_no_blocking(SOCKET sock) = 0;
    virtual int sock_make_blocking(SOCKET sock) = 0;
    // clang-format on
};

//
// This is the wrapper class for the real function
// -----------------------------------------------
class PupnpSockReal : public PupnpSockInterface {
  public:
    virtual ~PupnpSockReal() override = default;
    int sock_make_no_blocking(SOCKET sock) override {
        return ::sock_make_no_blocking(sock);
    }
    int sock_make_blocking(SOCKET sock) override {
        return ::sock_make_blocking(sock);
    }
};

//
// This is the caller or injector class that injects the class (worker) to be
// used, real or mocked functions.
// clang-format off
/* Example:
    PupnpSockReal pupnp_sock_realObj; // already done below
    PupnpSock(&pupnp_sock_realObj);   // already done below
    { // Other scope, e.g. within a gtest
        class PupnpSockMock : public PupnpSockInterface { ...; MOCK_METHOD(...) };
        PupnpSockMock pupnp_sock_mockObj;
        PupnpSock pupnp_sock_injectObj(&pupnp_sock_mockObj); // obj. name doesn't matter
        EXPECT_CALL(pupnp_sock_mockObj, ...);
    } // End scope, mock objects are destructed, worker restored to default. */
// clang-format on
//------------------------------------------------------------------------------
class PupnpSock {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    PupnpSock(PupnpSockReal* a_ptr_realObj) {
        m_ptr_workerObj = (PupnpSockInterface*)a_ptr_realObj;
    }

    // This constructor is used to inject the pointer to the mocking function.
    PupnpSock(PupnpSockInterface* a_ptr_mockObj) {
        m_ptr_oldObj = m_ptr_workerObj;
        m_ptr_workerObj = a_ptr_mockObj;
    }

    // The destructor is used to restore the old pointer.
    virtual ~PupnpSock() { m_ptr_workerObj = m_ptr_oldObj; }

    // Methods
    virtual int sock_make_blocking(SOCKET sock) {
        return m_ptr_workerObj->sock_make_blocking(sock);
    }

    virtual int sock_make_no_blocking(SOCKET sock) {
        return m_ptr_workerObj->sock_make_no_blocking(sock);
    }

  private:
    // Next variable must be static. Please note that a static member variable
    // belongs to the class, but not to the instantiated object. This is
    // important here for mocking because the pointer is also valid on all
    // objects of this class. With inline we do not need an extra definition
    // line outside the class.
    static inline PupnpSockInterface* m_ptr_workerObj;
    PupnpSockInterface* m_ptr_oldObj{};
};

// On program start create an object and inject pointer to the real functions.
// This will exist until program end.
static PupnpSockReal pupnp_sock_realObj;
static PupnpSock pupnp_sock(&pupnp_sock_realObj);

} // namespace umock

#endif // UPNPLIB_PUPNP_SOCK_HPP
