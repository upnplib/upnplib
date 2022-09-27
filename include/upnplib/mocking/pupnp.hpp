#ifndef UPNPLIB_PUPNP_HPP
#define UPNPLIB_PUPNP_HPP
// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-09-27

// This is a header only mocking include file. When included it is present
// direct in the source code and can be used to mock static functions that are
// hidden to the global context. With some returns and pointer variables this
// include does not consume many resources. But still, it's better for the
// global context with exported symbols to use a header + sorce file compile
// unit as shown in other mocking files.

#ifndef UPNP_ENABLE_BLOCKING_TCP_CONNECTIONS
static int Check_Connect_And_Wait_Connection(SOCKET sock, int connect_res);
#endif
static int private_connect(SOCKET sockfd, const struct sockaddr* serv_addr,
                           socklen_t addrlen);

namespace upnplib {
namespace mocking {

class PupnpInterface {
  public:
    virtual ~PupnpInterface() = default;
    // clang-format off
    virtual int sock_make_no_blocking(SOCKET sock) = 0;
    virtual int sock_make_blocking(SOCKET sock) = 0;
#ifndef UPNP_ENABLE_BLOCKING_TCP_CONNECTIONS
    virtual int Check_Connect_And_Wait_Connection(SOCKET sock, int connect_res) = 0;
#endif
    virtual int private_connect(SOCKET sockfd, const struct sockaddr* serv_addr, socklen_t addrlen) = 0;
    // clang-format on
};

//
// This is the wrapper class for the real function
// -----------------------------------------------
class PupnpReal : public PupnpInterface {
  public:
    virtual ~PupnpReal() override = default;
    int sock_make_no_blocking(SOCKET sock) {
        return ::sock_make_no_blocking(sock);
    }
    int sock_make_blocking(SOCKET sock) { return ::sock_make_blocking(sock); }
#ifndef UPNP_ENABLE_BLOCKING_TCP_CONNECTIONS
    int Check_Connect_And_Wait_Connection(SOCKET sock, int connect_res) {
        return ::Check_Connect_And_Wait_Connection(sock, connect_res);
    }
#endif
    int private_connect(SOCKET sockfd, const struct sockaddr* serv_addr,
                        socklen_t addrlen) {
        return ::private_connect(sockfd, serv_addr, addrlen);
    }
};

//
// This is the caller or injector class that injects the class (worker) to be
// used, real or mocked functions.
/* Example:
    PupnpReal pupnp_realObj; // already done below
    Pupnp(&pupnp_realObj);   // already done below
    { // Other scope, e.g. within a gtest
        class PupnpMock : public PupnpInterface { ...; MOCK_METHOD(...) };
        PupnpMock pupnp_mockObj;
        Pupnp pupnp_injectObj(&pupnp_mockObj); // obj. name doesn't matter
        EXPECT_CALL(pupnp_mockObj, ...);
    } // End scope, mock objects are destructed, worker restored to default.
*///----------------------------------------------------------------------------
class Pupnp {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    Pupnp(PupnpReal* a_ptr_realObj) {
        m_ptr_workerObj = (PupnpInterface*)a_ptr_realObj;
    }

    // This constructor is used to inject the pointer to the mocking function.
    Pupnp(PupnpInterface* a_ptr_mockObj) {
        m_ptr_oldObj = m_ptr_workerObj;
        m_ptr_workerObj = a_ptr_mockObj;
    }

    // The destructor is used to restore the old pointer.
    virtual ~Pupnp() { m_ptr_workerObj = m_ptr_oldObj; }

    // Methods
    virtual int private_connect(SOCKET sockfd, const struct sockaddr* serv_addr,
                                socklen_t addrlen) {
        return m_ptr_workerObj->private_connect(sockfd, serv_addr, addrlen);
    }

    virtual int sock_make_blocking(SOCKET sock) {
        return m_ptr_workerObj->sock_make_blocking(sock);
    }

    virtual int sock_make_no_blocking(SOCKET sock) {
        return m_ptr_workerObj->sock_make_no_blocking(sock);
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
    static inline PupnpInterface* m_ptr_workerObj;
    PupnpInterface* m_ptr_oldObj{};
};

// On program start create an object and inject pointer to the real functions.
// This will exist until program end.
PupnpReal pupnp_realObj;
Pupnp pupnp(&pupnp_realObj);

} // namespace mocking
} // namespace upnplib

#endif // UPNPLIB_PUPNP_HPP
