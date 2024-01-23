#ifndef UMOCK_SYS_SOCKET_MOCK_HPP
#define UMOCK_SYS_SOCKET_MOCK_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-12-26

#include <umock/sys_socket.hpp>
#include <upnplib/port.hpp>
#include <gmock/gmock.h>

namespace umock {

// For details about using this constant have a look at
// [./gtests README]
// (https://github.com/upnplib/upnplib/tree/main/gtests#find-different-test-variables).
constexpr SOCKET sfd_base{};


class UPNPLIB_API Sys_socketMock : public umock::Sys_socketInterface {
  public:
    Sys_socketMock();
    virtual ~Sys_socketMock() override;
    // clang-format off
    DISABLE_MSVC_WARN_4251
    MOCK_METHOD(SOCKET, socket, (int domain, int type, int protocol), (override));
    MOCK_METHOD(int, bind, (SOCKET sockfd, const struct sockaddr* addr, socklen_t addrlen), (override));
    MOCK_METHOD(int, listen, (SOCKET sockfd, int backlog), (override));
    MOCK_METHOD(SOCKET, accept, (SOCKET sockfd, struct sockaddr* addr, socklen_t* addrlen), (override));
    MOCK_METHOD(int, getsockopt, (SOCKET sockfd, int level, int optname, void* optval, socklen_t* optlen), (override));
    MOCK_METHOD(int, setsockopt, (SOCKET sockfd, int level, int optname, const void* optval, socklen_t optlen), (override));
    MOCK_METHOD(int, getsockname, (SOCKET sockfd, struct sockaddr* addr, socklen_t* addrlen), (override));
    MOCK_METHOD(SSIZEP_T, recv, (SOCKET sockfd, char* buf, SIZEP_T len, int flags), (override));
    MOCK_METHOD(SSIZEP_T, recvfrom, (SOCKET sockfd, char* buf, SIZEP_T len, int flags, struct sockaddr* src_addr, socklen_t* addrlen), (override));
    MOCK_METHOD(SSIZEP_T, send, (SOCKET sockfd, const char* buf, SIZEP_T len, int flags), (override));
    MOCK_METHOD(SSIZEP_T, sendto, (SOCKET sockfd, const char* buf, SIZEP_T len, int flags, const struct sockaddr* dest_addr, socklen_t addrlen), (override));
    MOCK_METHOD(int, connect, (SOCKET sockfd, const struct sockaddr* addr, socklen_t addrlen), (override));
    MOCK_METHOD(int, shutdown, (SOCKET sockfd, int how), (override));
    MOCK_METHOD(int, select, (SOCKET nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout), (override));
    ENABLE_MSVC_WARN
    // clang-format on
};

} // namespace umock

#endif // UMOCK_SYS_SOCKET_MOCK_HPP
