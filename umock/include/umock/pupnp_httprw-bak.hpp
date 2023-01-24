#ifndef UPNPLIB_PUPNP_HTTPRW_HPP
#define UPNPLIB_PUPNP_HTTPRW_HPP
// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-01-25

#include "upnplib/port.hpp"
#include <sys/socket.h>

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
                                          int connect_res) override;
#endif
    int private_connect(SOCKET sockfd, const struct sockaddr* serv_addr,
                        socklen_t addrlen) override;
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
    PupnpHttpRw(PupnpHttpRwReal* a_ptr_realObj);

    // This constructor is used to inject the pointer to the mocking function.
    PupnpHttpRw(PupnpHttpRwInterface* a_ptr_mockObj);

    // The destructor is used to restore the old pointer.
    virtual ~PupnpHttpRw();

    // Methods
    virtual int private_connect(SOCKET sockfd, const struct sockaddr* serv_addr,
                                socklen_t addrlen);
#ifndef UPNP_ENABLE_BLOCKING_TCP_CONNECTIONS
    virtual int Check_Connect_And_Wait_Connection(SOCKET sock, int connect_res);
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

extern PupnpHttpRw pupnp_httprw;

} // namespace umock

#endif // UPNPLIB_PUPNP_HTTPRW_HPP
