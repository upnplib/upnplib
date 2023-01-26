#ifndef UPNPLIB_PUPNP_HTTPRW_HPP
#define UPNPLIB_PUPNP_HTTPRW_HPP
// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-01-27

// This is a header only mocking include file. When included it is present
// direct in the source code and can be used to mock static functions that are
// hidden to the global context. With some returns and pointer variables this
// include does not consume many resources.

#include "upnplib/port.hpp"

#ifndef UPNP_ENABLE_BLOCKING_TCP_CONNECTIONS
static int Check_Connect_And_Wait_Connection(SOCKET sock, int connect_res);
#endif
static int private_connect(SOCKET sockfd, const struct sockaddr* serv_addr,
                           socklen_t addrlen);

namespace umock {

class PupnpHttpRwInterface {
  public:
    virtual ~PupnpHttpRwInterface() = default;
    // clang-format off
#ifndef UPNP_ENABLE_BLOCKING_TCP_CONNECTIONS
    virtual int Check_Connect_And_Wait_Connection(SOCKET sock, int connect_res) = 0;
#endif
    virtual int private_connect(SOCKET sockfd, const struct sockaddr* serv_addr, socklen_t addrlen) = 0;
    // clang-format on
};

//
// This is the wrapper class for the real function
// -----------------------------------------------
class PupnpHttpRwReal : public PupnpHttpRwInterface {
  public:
    virtual ~PupnpHttpRwReal() override = default;
#ifndef UPNP_ENABLE_BLOCKING_TCP_CONNECTIONS
    int Check_Connect_And_Wait_Connection(SOCKET sock,
                                          int connect_res) override {
        return ::Check_Connect_And_Wait_Connection(sock, connect_res);
    }
#endif
    int private_connect(SOCKET sockfd, const struct sockaddr* serv_addr,
                        socklen_t addrlen) override {
        return ::private_connect(sockfd, serv_addr, addrlen);
    }
};

//
// This is the caller or injector class that injects the class (worker) to be
// used, real or mocked functions.
// clang-format off
/* Example:
    PupnpHttpRwReal pupnp_httprw_realObj; // already done below
    PupnpHttpRw(&pupnp_httprw_realObj);   // already done below
    { // Other scope, e.g. within a gtest
        class PupnpHttpRwMock : public PupnpHttpRwInterface { ...; MOCK_METHOD(...) };
        PupnpHttpRwMock pupnp_httprw_mockObj;
        PupnpHttpRw pupnp_httprw_injectObj(&pupnp_httprw_mockObj); // obj. name doesn't matter
        EXPECT_CALL(pupnp_httprw_mockObj, ...);
    } // End scope, mock objects are destructed, worker restored to default. */
// clang-format on
//------------------------------------------------------------------------------
class PupnpHttpRw {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    PupnpHttpRw(PupnpHttpRwReal* a_ptr_realObj) {
        m_ptr_workerObj = (PupnpHttpRwInterface*)a_ptr_realObj;
    }

    // This constructor is used to inject the pointer to the mocking function.
    PupnpHttpRw(PupnpHttpRwInterface* a_ptr_mockObj) {
        m_ptr_oldObj = m_ptr_workerObj;
        m_ptr_workerObj = a_ptr_mockObj;
    }

    // The destructor is used to restore the old pointer.
    virtual ~PupnpHttpRw() { m_ptr_workerObj = m_ptr_oldObj; }

    // Methods
    virtual int private_connect(SOCKET sockfd, const struct sockaddr* serv_addr,
                                socklen_t addrlen) {
        return m_ptr_workerObj->private_connect(sockfd, serv_addr, addrlen);
    }
#ifndef UPNP_ENABLE_BLOCKING_TCP_CONNECTIONS
    virtual int Check_Connect_And_Wait_Connection(SOCKET sock,
                                                  int connect_res) {
        return m_ptr_workerObj->Check_Connect_And_Wait_Connection(sock,
                                                                  connect_res);
    }
#endif

  private:
    // Next variable must be static. Please note that a static member variable
    // belongs to the class, but not to the instantiated object. This is
    // important here for mocking because the pointer is also valid on all
    // objects of the class. With inline we do not need an extra definition line
    // outside the class.
    static inline PupnpHttpRwInterface* m_ptr_workerObj;
    PupnpHttpRwInterface* m_ptr_oldObj{};
};

// On program start create an object and inject pointer to the real functions.
// This will exist until program end. Because this is a header file the object
// must be static otherwise we will get a linker error of "multiple definition"
// if included in more than one source file.
static PupnpHttpRwReal pupnp_httprw_realObj;
static PupnpHttpRw pupnp_httprw(&pupnp_httprw_realObj);

} // namespace umock

#endif // UPNPLIB_PUPNP_HTTPRW_HPP
