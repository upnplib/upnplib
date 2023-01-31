// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-01-31

#include "umock/sys_socket.inc"

namespace umock {

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
SOCKET Sys_socket::socket(int domain, int type, int protocol) {
    return m_ptr_workerObj->socket(domain, type, protocol);
}
int Sys_socket::bind(SOCKET sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    return m_ptr_workerObj->bind(sockfd, addr, addrlen);
}
int Sys_socket::listen(SOCKET sockfd, int backlog) {
    return m_ptr_workerObj->listen(sockfd, backlog);
}
SOCKET Sys_socket::accept(SOCKET sockfd, struct sockaddr* addr, socklen_t* addrlen) {
    return m_ptr_workerObj->accept(sockfd, addr, addrlen);
}
SSIZEP_T Sys_socket::recv(SOCKET sockfd, char* buf, SIZEP_T len, int flags) {
    return m_ptr_workerObj->recv(sockfd, buf, len, flags);
}
SSIZEP_T Sys_socket::recvfrom(SOCKET sockfd, char* buf, SIZEP_T len, int flags, struct sockaddr* src_addr, socklen_t* addrlen) {
    return m_ptr_workerObj->recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
}
SSIZEP_T Sys_socket::send(SOCKET sockfd, const char* buf, SIZEP_T len, int flags) {
    return m_ptr_workerObj->send(sockfd, buf, len, flags);
}
SSIZEP_T Sys_socket::sendto(SOCKET sockfd, const char* buf, SIZEP_T len, int flags, const struct sockaddr* dest_addr, socklen_t addrlen) {
    return m_ptr_workerObj->sendto(sockfd, buf, len, flags, dest_addr, addrlen);
}
int Sys_socket::connect(SOCKET sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    return m_ptr_workerObj->connect(sockfd, addr, addrlen);
}
int Sys_socket::getsockopt(SOCKET sockfd, int level, int optname, void* optval, socklen_t* optlen) {
#ifdef _WIN32
    return m_ptr_workerObj->getsockopt(sockfd, level, optname, (char*)optval, optlen);
#else
    return m_ptr_workerObj->getsockopt(sockfd, level, optname, optval, optlen);
#endif
}
int Sys_socket::setsockopt(SOCKET sockfd, int level, int optname, const char* optval, socklen_t optlen) {
    return m_ptr_workerObj->setsockopt(sockfd, level, optname, optval, optlen);
}
int Sys_socket::getsockname(SOCKET sockfd, struct sockaddr* addr, socklen_t* addrlen) {
    return m_ptr_workerObj->getsockname(sockfd, addr, addrlen);
}
int Sys_socket::shutdown(SOCKET sockfd, int how) {
    return m_ptr_workerObj->shutdown(sockfd, how);
}
// clang-format on

//
// On program start create an object and inject pointer to the real functions.
// This will exist until program end.
Sys_socketReal sys_socket_realObj;
UPNPLIB_API Sys_socket sys_socket_h(&sys_socket_realObj);

} // namespace umock
