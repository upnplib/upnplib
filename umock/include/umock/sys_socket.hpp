#ifndef UPNPLIB_MOCKING_SYS_SOCKET_HPP
#define UPNPLIB_MOCKING_SYS_SOCKET_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-01-27

#include "upnplib/port.hpp"
#include "upnplib/visibility.hpp"

namespace umock {

class Sys_socketInterface {
  public:
    virtual ~Sys_socketInterface() = default;
    // clang-format off
    virtual SOCKET socket(int domain, int type, int protocol) = 0;
    virtual int bind(SOCKET sockfd, const struct sockaddr* addr, socklen_t addrlen) = 0;
    virtual int listen(SOCKET sockfd, int backlog) = 0;
    virtual SOCKET accept(SOCKET sockfd, struct sockaddr* addr, socklen_t* addrlen) = 0;
    virtual SSIZEP_T recv(SOCKET sockfd, char* buf, SIZEP_T len, int flags) = 0;
    virtual SSIZEP_T recvfrom(SOCKET sockfd, char* buf, SIZEP_T len, int flags, struct sockaddr* src_addr, socklen_t* addrlen) = 0;
    virtual SSIZEP_T send(SOCKET sockfd, const char* buf, SIZEP_T len, int flags) = 0;
    virtual SSIZEP_T sendto(SOCKET sockfd, const char* buf, SIZEP_T len, int flags, const struct sockaddr* dest_addr, socklen_t addrlen) = 0;
    virtual int connect(SOCKET sockfd, const struct sockaddr* addr, socklen_t addrlen) = 0;
    virtual int getsockopt(SOCKET sockfd, int level, int optname, void* optval, socklen_t* optlen) = 0;
    virtual int setsockopt(SOCKET sockfd, int level, int optname, const char* optval, socklen_t optlen) = 0;
    virtual int getsockname(SOCKET sockfd, struct sockaddr* addr, socklen_t* addrlen) = 0;
    virtual int shutdown(SOCKET sockfd, int how) = 0;
    // clang-format on
};

//
// This is the wrapper class for the real (library?) function
// ----------------------------------------------------------
class Sys_socketReal : public Sys_socketInterface {
  public:
    virtual ~Sys_socketReal() override = default;
    // clang-format off
    SOCKET socket(int domain, int type, int protocol) override {
        return ::socket(domain, type, protocol);
    }
    int bind(SOCKET sockfd, const struct sockaddr* addr, socklen_t addrlen) override {
        return ::bind(sockfd, addr, addrlen);
    }
    int listen(SOCKET sockfd, int backlog) override {
        return ::listen(sockfd, backlog);
    }
    SOCKET accept(SOCKET sockfd, struct sockaddr* addr, socklen_t* addrlen) override {
        return ::accept(sockfd, addr, addrlen);
    }
    SSIZEP_T recv(SOCKET sockfd, char* buf, SIZEP_T len, int flags) override {
        return ::recv(sockfd, buf, len, flags);
    }
    SSIZEP_T recvfrom(SOCKET sockfd, char* buf, SIZEP_T len, int flags, struct sockaddr* src_addr, socklen_t* addrlen) override {
        return ::recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
    }
    SSIZEP_T send(SOCKET sockfd, const char* buf, SIZEP_T len, int flags) override {
        return ::send(sockfd, buf, len, flags);
    }
    SSIZEP_T sendto(SOCKET sockfd, const char* buf, SIZEP_T len, int flags, const struct sockaddr* dest_addr, socklen_t addrlen) override {
        return ::sendto(sockfd, buf, len, flags, dest_addr, addrlen);
    }
    int connect(SOCKET sockfd, const struct sockaddr* addr, socklen_t addrlen) override {
        return ::connect(sockfd, addr, addrlen);
    }
    int getsockopt(SOCKET sockfd, int level, int optname, void* optval, socklen_t* optlen) override {
#ifdef _WIN32
        return ::getsockopt(sockfd, level, optname, (char*)optval, optlen);
#else
        return ::getsockopt(sockfd, level, optname, optval, optlen);
#endif
    }
    int setsockopt(SOCKET sockfd, int level, int optname, const char* optval, socklen_t optlen) override {
        return ::setsockopt(sockfd, level, optname, optval, optlen);
    }
    int getsockname(SOCKET sockfd, struct sockaddr* addr, socklen_t* addrlen) override {
        return ::getsockname(sockfd, addr, addrlen);
    }
    int shutdown(SOCKET sockfd, int how) override {
        return ::shutdown(sockfd, how);
    }
    // clang-format on
};

//
// This is the caller or injector class that injects the class (worker) to be
// used, real or mocked functions.
// clang-format off
/* Example:
    Sys_socketReal sys_socket_realObj; // already done below
    Sys_socket(&sys_socket_realObj);   // already done below
    { // Other scope, e.g. within a gtest
        class Sys_socketMock : public Sys_socketInterface { ...; MOCK_METHOD(...) };
        Sys_socketMock sys_socket_mockObj;
        Sys_socket sys_socket_injectObj(&sys_socket_mockObj); // obj. name doesn't matter
        EXPECT_CALL(sys_socket_mockObj, ...);
    } // End scope, mock objects are destructed, worker restored to default.
*/ //------------------------------------------------------------------------
// clang-format on
class UPNPLIB_API Sys_socket {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    Sys_socket(Sys_socketReal* a_ptr_realObj) {
        m_ptr_workerObj = (Sys_socketInterface*)a_ptr_realObj;
    }

    // This constructor is used to inject the pointer to the mocking function.
    Sys_socket(Sys_socketInterface* a_ptr_mockObj) {
        m_ptr_oldObj = m_ptr_workerObj;
        m_ptr_workerObj = a_ptr_mockObj;
    }

    // The destructor is ussed to restore the old pointer.
    virtual ~Sys_socket() { m_ptr_workerObj = m_ptr_oldObj; }

    // Methods
    // clang-format off
    virtual SOCKET socket(int domain, int type, int protocol) {
        return m_ptr_workerObj->socket(domain, type, protocol);
    }
    virtual int bind(SOCKET sockfd, const struct sockaddr* addr, socklen_t addrlen) {
        return m_ptr_workerObj->bind(sockfd, addr, addrlen);
    }
    virtual int listen(SOCKET sockfd, int backlog) {
        return m_ptr_workerObj->listen(sockfd, backlog);
    }
    virtual SOCKET accept(SOCKET sockfd, struct sockaddr* addr, socklen_t* addrlen) {
        return m_ptr_workerObj->accept(sockfd, addr, addrlen);
    }
    virtual SSIZEP_T recv(SOCKET sockfd, char* buf, SIZEP_T len, int flags) {
        return m_ptr_workerObj->recv(sockfd, buf, len, flags);
    }
    virtual SSIZEP_T recvfrom(SOCKET sockfd, char* buf, SIZEP_T len, int flags, struct sockaddr* src_addr, socklen_t* addrlen) {
        return m_ptr_workerObj->recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
    }
    virtual SSIZEP_T send(SOCKET sockfd, const char* buf, SIZEP_T len, int flags) {
        return m_ptr_workerObj->send(sockfd, buf, len, flags);
    }
    virtual SSIZEP_T sendto(SOCKET sockfd, const char* buf, SIZEP_T len, int flags, const struct sockaddr* dest_addr, socklen_t addrlen) {
        return m_ptr_workerObj->sendto(sockfd, buf, len, flags, dest_addr, addrlen);
    }
    virtual int connect(SOCKET sockfd, const struct sockaddr* addr, socklen_t addrlen) {
        return m_ptr_workerObj->connect(sockfd, addr, addrlen);
    }
    virtual int getsockopt(SOCKET sockfd, int level, int optname, void* optval, socklen_t* optlen) {
    #ifdef _WIN32
        return m_ptr_workerObj->getsockopt(sockfd, level, optname, (char*)optval, optlen);
    #else
        return m_ptr_workerObj->getsockopt(sockfd, level, optname, optval, optlen);
    #endif
    }
    virtual int setsockopt(SOCKET sockfd, int level, int optname, const char* optval, socklen_t optlen) {
        return m_ptr_workerObj->setsockopt(sockfd, level, optname, optval, optlen);
    }
    virtual int getsockname(SOCKET sockfd, struct sockaddr* addr, socklen_t* addrlen) {
        return m_ptr_workerObj->getsockname(sockfd, addr, addrlen);
    }
    virtual int shutdown(SOCKET sockfd, int how) {
        return m_ptr_workerObj->shutdown(sockfd, how);
    }
    // clang-format on

  private:
    // Next variable must be static. Please note that a static member variable
    // belongs to the class, but not to the instantiated object. This is
    // important here for mocking because the pointer is also valid on all
    // objects of this class. With inline we do not need an extra definition
    // line outside the class.
    static inline Sys_socketInterface* m_ptr_workerObj;
    Sys_socketInterface* m_ptr_oldObj{};
};

// On program start create an object and inject pointer to the real functions.
// This will exist until program end. Because this is a header file the object
// must be static otherwise we will get a linker error of "multiple definition"
// if included in more than one source file.
static Sys_socketReal sys_socket_realObj;
static Sys_socket sys_socket_h(&sys_socket_realObj);

} // namespace umock

#endif // UPNPLIB_MOCKING_SYS_SOCKET_HPP
