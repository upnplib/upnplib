// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-10-16

#include "upnplib/mocking/sys_socket.inc"

namespace upnplib::mocking {

// clang-format off
int Sys_socketReal::socket(int domain, int type, int protocol) {
    return ::socket(domain, type, protocol);
}

int Sys_socketReal::bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    return ::bind(sockfd, addr, addrlen);
}

int Sys_socketReal::listen(int sockfd, int backlog) {
    return ::listen(sockfd, backlog);
}

int Sys_socketReal::accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen) {
    return ::accept(sockfd, addr, addrlen);
}

size_t Sys_socketReal::recv(int sockfd, char* buf, size_t len, int flags) {
    return ::recv(sockfd, buf, len, flags);
}

size_t Sys_socketReal::recvfrom(int sockfd, char* buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen) {
    return ::recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
}

size_t Sys_socketReal::send(int sockfd, const char* buf, size_t len, int flags) {
    return ::send(sockfd, buf, len, flags);
}

int Sys_socketReal::connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    return ::connect(sockfd, addr, addrlen);
}

int Sys_socketReal::getsockopt(int sockfd, int level, int optname, void* optval, socklen_t* optlen) {
#ifdef _WIN32
    return ::getsockopt(sockfd, level, optname, (char*)optval, optlen);
#else
    return ::getsockopt(sockfd, level, optname, optval, optlen);
#endif
}

int Sys_socketReal::setsockopt(int sockfd, int level, int optname, const char* optval, socklen_t optlen) {
    return ::setsockopt(sockfd, level, optname, optval, optlen);
}

int Sys_socketReal::getsockname(int sockfd, struct sockaddr* addr, socklen_t* addrlen) {
    return ::getsockname(sockfd, addr, addrlen);
}

int Sys_socketReal::shutdown(int sockfd, int how) {
    return ::shutdown(sockfd, how);
}
// clang-format on

//
// This constructor is used to inject the pointer to the real function.
Sys_socket::Sys_socket(Sys_socketReal* a_ptr_realObj) {
    m_ptr_workerObj = (Sys_socketInterface*)a_ptr_realObj;
}

// This constructor is used to inject the pointer to the mocking function.
Sys_socket::Sys_socket(Sys_socketInterface* a_ptr_mockObj) {
    m_ptr_oldObj = m_ptr_workerObj;
    m_ptr_workerObj = a_ptr_mockObj;
}

// The destructor is ussed to restore the old pointer.
Sys_socket::~Sys_socket() { m_ptr_workerObj = m_ptr_oldObj; }

// Methods
// clang-format off
int Sys_socket::socket(int domain, int type, int protocol) {
    return m_ptr_workerObj->socket(domain, type, protocol);
}

int Sys_socket::bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    return m_ptr_workerObj->bind(sockfd, addr, addrlen);
}

int Sys_socket::listen(int sockfd, int backlog) {
    return m_ptr_workerObj->listen(sockfd, backlog);
}

int Sys_socket::accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen) {
    return m_ptr_workerObj->accept(sockfd, addr, addrlen);
}

size_t Sys_socket::recv(int sockfd, char* buf, size_t len, int flags) {
    return m_ptr_workerObj->recv(sockfd, buf, len, flags);
}

size_t Sys_socket::recvfrom(int sockfd, char* buf, size_t len, int flags, struct sockaddr* src_addr, socklen_t* addrlen) {
    return m_ptr_workerObj->recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
}

size_t Sys_socket::send(int sockfd, const char* buf, size_t len, int flags) {
    return m_ptr_workerObj->send(sockfd, buf, len, flags);
}

int Sys_socket::connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    return m_ptr_workerObj->connect(sockfd, addr, addrlen);
}

int Sys_socket::getsockopt(int sockfd, int level, int optname, void* optval, socklen_t* optlen) {
#ifdef _WIN32
    return m_ptr_workerObj->getsockopt(sockfd, level, optname, (char*)optval, optlen);
#else
    return m_ptr_workerObj->getsockopt(sockfd, level, optname, optval, optlen);
#endif
}

int Sys_socket::setsockopt(int sockfd, int level, int optname, const char* optval, socklen_t optlen) {
    return m_ptr_workerObj->setsockopt(sockfd, level, optname, optval, optlen);
}

int Sys_socket::getsockname(int sockfd, struct sockaddr* addr, socklen_t* addrlen) {
    return m_ptr_workerObj->getsockname(sockfd, addr, addrlen);
}

int Sys_socket::shutdown(int sockfd, int how) {
    return m_ptr_workerObj->shutdown(sockfd, how);
}
// clang-format on

// On program start create an object and inject pointer to the real functions.
// This will exist until program end.
Sys_socketReal sys_socket_realObj;
UPNPLIB_API Sys_socket sys_socket_h(&sys_socket_realObj);

} // namespace upnplib::mocking
