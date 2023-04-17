#ifndef UMOCK_SYS_SOCKET_MOCK_HPP
#define UMOCK_SYS_SOCKET_MOCK_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-04-17

#include <umock/sys_socket.hpp>
#include <gmock/gmock.h>

namespace umock {

class Sys_socketMock : public umock::Sys_socketInterface {
  public:
    virtual ~Sys_socketMock() override = default;
    // clang-format off
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
    // clang-format on
};

} // namespace umock

#endif // UMOCK_SYS_SOCKET_MOCK_HPP
