// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-04-20

#include <upnplib/socket.hpp>

#include <umock/sys_socket_mock.hpp>

using testing::_;
using testing::DoAll;
using testing::Pointee;
using testing::Return;
using testing::SetArgPointee;
using testing::SetErrnoAndReturn;
using testing::StartsWith;
using testing::ThrowsMessage;


namespace upnplib {
bool old_code{false}; // Managed in upnplib_gtest_main.inc

TEST(SocketTestSuite, get_socket_successful) {
    WINSOCK_INIT_P

    // Test Unit
    CSocket sock(AF_INET6, SOCK_STREAM);

    EXPECT_THAT([&sock]() { sock.get_port(); },
                ThrowsMessage<std::runtime_error>(
                    "ERROR! Failed to get socket address/port: \"not bound to "
                    "an address\""));
    EXPECT_EQ(sock.get_sockerr(), 0);
    EXPECT_FALSE(sock.is_reuse_addr());
    EXPECT_FALSE(sock.is_v6only());
    EXPECT_FALSE(sock.is_bind());
    EXPECT_FALSE(sock.is_listen());
}

TEST(SocketTestSuite, move_socket_successful) {
    WINSOCK_INIT_P

    // Provide a socket object
    CSocket sock1(AF_INET, SOCK_STREAM);
    SOCKET old_fd_sock1 = sock1;

    // Get local interface address. Socket type (SOCK_STREAM) must be the same
    // as that from the socket.
    CAddrinfo ai("", "50009", AF_INET, SOCK_STREAM,
                 AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);

    // Bind address to the socket.
    ASSERT_NO_THROW(sock1.bind(ai));

    // Configure socket to listen so it will accept connections.
    ASSERT_NO_THROW(sock1.listen());

    // Test Unit, move sock1 to a new sock2
    // CSocket sock2 = sock1; // This does not compile because it needs a copy
    //                           constructor that isn't available. We restrict
    //                           to move only.
    // This moves the socket file descriptor.
    CSocket sock2 = std::move(sock1);

    // The socket file descriptor has been moved to the new object.
    EXPECT_EQ((SOCKET)sock2, old_fd_sock1);
    // The old object has now an invalid socket file descriptor.
    EXPECT_EQ((SOCKET)sock1, INVALID_SOCKET);

    // Tests of socket object (sock1) with INVALID_SOCKET see later test.
    // Check if new socket is valid.
    EXPECT_EQ(sock2.get_port(), 50009);
    EXPECT_EQ(sock2.get_sockerr(), 0);
    EXPECT_FALSE(sock2.is_reuse_addr());
    EXPECT_FALSE(sock2.is_v6only());
    EXPECT_TRUE(sock2.is_bind());
    EXPECT_TRUE(sock2.is_listen());
}

TEST(SocketTestSuite, assign_socket_successful) {
    WINSOCK_INIT_P

    // Get local interface address. Socket type (SOCK_STREAM) must be the same
    // as that from the socket.
    CAddrinfo ai("", "50010", AF_INET6, SOCK_STREAM,
                 AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);

    // Provide first of two socket objects.
    CSocket sock1(AF_INET6, SOCK_STREAM);
    SOCKET old_fd_sock1 = sock1;

    // Bind local interface address to the socket.
    ASSERT_NO_THROW(sock1.bind(ai));

    // Configure socket to listen so it will accept connections.
    ASSERT_NO_THROW(sock1.listen());

    // Provide second empty socket.
    CSocket sock2;

    // Test Unit. We can only move. Copy a socket resource is not useful.
    sock2 = std::move(sock1);

    // The socket file descriptor has been moved to the destination object.
    EXPECT_EQ((SOCKET)sock2, old_fd_sock1);
    // The old object has now an invalid socket file descriptor.
    EXPECT_EQ((SOCKET)sock1, INVALID_SOCKET);

    // Tests of socket object (sock1) with INVALID_SOCKET see later test.
    // Check if new socket is valid.
    EXPECT_EQ(sock2.get_port(), 50010);
    EXPECT_EQ(sock2.get_sockerr(), 0);
    EXPECT_FALSE(sock2.is_reuse_addr());
    EXPECT_FALSE(sock2.is_v6only());
    EXPECT_TRUE(sock2.is_bind());
    EXPECT_TRUE(sock2.is_listen());
}

TEST(SocketTestSuite, object_with_invalid_socket_fd) {
    WINSOCK_INIT_P

    // Test Unit
    CSocket sock;
    EXPECT_EQ((SOCKET)sock, INVALID_SOCKET);

    // All getter from an INVALID_SOCKET throw an exception.
    EXPECT_THAT([&sock]() { sock.get_port(); },
                ThrowsMessage<std::runtime_error>(
                    "ERROR! Failed to get socket address/port: \"Bad file "
                    "descriptor\""));
    EXPECT_THAT([&sock]() { sock.get_sockerr(); },
                ThrowsMessage<std::runtime_error>(StartsWith(
                    "ERROR! Failed to get socket option SO_ERROR: ")));
    EXPECT_THAT([&sock]() { sock.is_reuse_addr(); },
                ThrowsMessage<std::runtime_error>(StartsWith(
                    "ERROR! Failed to get socket option SO_REUSEADDR: ")));
    EXPECT_THAT([&sock]() { sock.is_v6only(); },
                ThrowsMessage<std::runtime_error>(
                    "ERROR! Failed to get socket option 'is_v6only': \"Bad "
                    "file descriptor\""));
    EXPECT_THAT([&sock]() { sock.is_bind(); },
                ThrowsMessage<std::runtime_error>(
                    "ERROR! Failed to get socket option 'is_bind': \"Bad file "
                    "descriptor\""));
    EXPECT_THAT([&sock]() { sock.is_listen(); },
                ThrowsMessage<std::runtime_error>(
                    "ERROR! Failed to get socket option "
                    "'is_Listen': \"Bad file descriptor\""));
}

TEST(SocketTestSuite, set_bind) {
    WINSOCK_INIT_P

    // Get local interface address.
    const CAddrinfo ai("", "50012", AF_INET6, SOCK_STREAM,
                       AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);

    // Test Unit.
    // This binds the local address. You will get an error "address already in
    // use" if you try to bind to it again in this test.
    CSocket sock(AF_INET6, SOCK_STREAM);
    ASSERT_NO_THROW(sock.bind(ai));
    EXPECT_EQ(sock.get_port(), 50012); // This tests the binding

    // Test Unit. Binding an empty socket object will fail.
    EXPECT_THAT(
        [&ai]() {
            CSocket sock;
            sock.bind(ai);
        },
        ThrowsMessage<std::runtime_error>(
            StartsWith("ERROR! Failed to bind socket to an address:")));

    // Test Unit. Binding with a different AF_INET will fail.
    EXPECT_THAT(
        [&ai]() {
            CSocket sock(AF_INET, SOCK_STREAM);
            sock.bind(ai);
        },
        ThrowsMessage<std::runtime_error>(
            StartsWith("ERROR! Failed to bind socket to an address:")));
}

TEST(SocketTestSuite, set_bind_with_different_socket_type) {
    WINSOCK_INIT_P

    // Get local interface address.
    const CAddrinfo ai("", "50011", AF_INET, SOCK_DGRAM,
                       AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);

    // Test Unit. Binding with different SOCK_STREAM (not SOCK_DGRAM) will fail.
    EXPECT_THAT(
        [&ai]() {
            CSocket sock(AF_INET, SOCK_STREAM);
            sock.bind(ai);
        },
        ThrowsMessage<std::runtime_error>(StartsWith(
            "ERROR! Failed to bind socket to an address: \"socket type of "
            "address (")));
}

TEST(SocketTestSuite, set_bind_two_times_fail) {
    // Binding a socket two times isn't possible. The socket must be
    // shutdown/closed before bind it again.
    WINSOCK_INIT_P

    // Provide a socket object
    CSocket sock1(AF_INET6, SOCK_STREAM);

    // Get local interface address. Socket type (SOCK_STREAM) must be the same
    // as that from the socket.
    CAddrinfo ai1("", "50017", AF_INET6, SOCK_STREAM,
                  AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);

    // Test Unit.
    EXPECT_NO_THROW(sock1.bind(ai1));
    // Try to bind the socket a second time.
    EXPECT_THAT(
        [&sock1]() {
            CAddrinfo ai2("", "50018", AF_INET6, SOCK_STREAM,
                          AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);
            sock1.bind(ai2);
        },
        ThrowsMessage<std::runtime_error>(
            StartsWith("ERROR! Failed to bind socket to an address: ")));
}

TEST(SocketTestSuite, set_bind_same_address_multiple_times) {
    // Binding a socket again is possible after destruction of the socket that
    // shutdown/close it.
    WINSOCK_INIT_P

    // Get local interface address.
    CAddrinfo ai1("", "50019", AF_INET6, SOCK_STREAM,
                  AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);

    // Test Unit
    {
        CSocket sock1(AF_INET6, SOCK_STREAM);
        EXPECT_NO_THROW(sock1.bind(ai1));
        EXPECT_NO_THROW(sock1.listen());
    }
    {
        CSocket sock1(AF_INET6, SOCK_STREAM);
        EXPECT_NO_THROW(sock1.bind(ai1));
        EXPECT_NO_THROW(sock1.listen());
    }
    CSocket sock1(AF_INET6, SOCK_STREAM);
    EXPECT_NO_THROW(sock1.bind(ai1));
    EXPECT_NO_THROW(sock1.listen());
}

TEST(SocketTestSuite, set_wrong_arguments) {
    // Test Unit. Set wrong address family.
    EXPECT_THAT([]() { CSocket sock(-1, SOCK_STREAM); },
                ThrowsMessage<std::runtime_error>(
                    StartsWith("ERROR! Failed to create socket: ")));

    // Test Unit. Set wrong socket type.
    EXPECT_THAT([]() { CSocket sock(AF_INET6, -1); },
                ThrowsMessage<std::runtime_error>(
                    StartsWith("ERROR! Failed to create socket: ")));
}

// // This has already be done as part of other tests.
// TEST(SocketTestSuite, listen_successful) {
//     WINSOCK_INIT_P
//
//     // Get socket
//     CSocket sock(AF_INET6, SOCK_STREAM);
//
//     // Get local interface address info and bind it to the socket.
//     CAddrinfo ai("", "50008", AF_INET6, SOCK_STREAM,
//                  AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);
//     ASSERT_NO_THROW(sock.bind(ai));
//
//     // Configure socket to listen so it will accept connections.
//     ASSERT_NO_THROW(sock.listen());
//
//     // Test Unit
//     EXPECT_TRUE(sock.is_listen());
// }

TEST(SocketTestSuite, check_af_inet6_v6only) {
    WINSOCK_INIT_P

    // Get socket
    CSocket sock(AF_INET6, SOCK_DGRAM);

    // Set option IPV6_V6ONLY. Of course this can only be done with AF_INET6.
    constexpr int so_option = 1; // Set IPV6_V6ONLY
    constexpr socklen_t optlen{sizeof(so_option)};
    ASSERT_EQ(::setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&so_option,
                           optlen),
              0);
    // Test Unit
    EXPECT_TRUE(sock.is_v6only());
}

TEST(SocketTestSuite, check_af_inet_v6only) {
    // This must always return false because on AF_INET we cannot have v6only.
    WINSOCK_INIT_P

    // Get socket
    CSocket sock(AF_INET, SOCK_STREAM);

    // Test Unit
    EXPECT_FALSE(sock.is_v6only());
}

TEST(SocketTestSuite, get_addr_str_ipv6_successful) {
    WINSOCK_INIT_P

    // Get local interface address.
    const CAddrinfo ai("", "50013", AF_INET6, SOCK_STREAM,
                       AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);
    // Get a socket and bind it to the local address.
    CSocket sock(AF_INET6, SOCK_STREAM);
    ASSERT_NO_THROW(sock.bind(ai));

    // Test Unit
    EXPECT_EQ(sock.get_addr_str(), "[::]");
}

TEST(SocketTestSuite, get_addr_str_ipv4_successful) {
    WINSOCK_INIT_P

    // Get local interface address.
    const CAddrinfo ai("", "50014", AF_INET, SOCK_DGRAM,
                       AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);
    // Get a socket and bind it to the local address.
    CSocket sock(AF_INET, SOCK_DGRAM);
    ASSERT_NO_THROW(sock.bind(ai));

    // Test Unit
    EXPECT_EQ(sock.get_addr_str(), "0.0.0.0");
}

TEST(SocketTestSuite, get_addr_str_from_invalid_socket) {
    // Test Unit wit empty socket.
    EXPECT_THAT(
        []() {
            CSocket sock;
            sock.get_addr_str();
        },
        ThrowsMessage<std::runtime_error>(
            "ERROR! Failed to get socket address/port: \"Bad file "
            "descriptor\""));
}

TEST(SocketTestSuite, get_addr_str_from_unbound_socket) {
    WINSOCK_INIT_P

    // Get a valid socket but do not bind it to an address.
    EXPECT_THAT(
        []() {
            CSocket sock(AF_INET6, SOCK_STREAM);
            sock.get_addr_str();
        },
        ThrowsMessage<std::runtime_error>(
            "ERROR! Failed to get socket address/port: \"not bound to an "
            "address\""));
}

TEST(SocketTestSuite, get_addr_str_syscall_fail) {
    WINSOCK_INIT_P

    // Get local interface address.
    const CAddrinfo ai("", "50015", AF_INET6, SOCK_STREAM,
                       AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);
    // Get a socket and bind it to the local address.
    CSocket sock(AF_INET6, SOCK_STREAM);
    ASSERT_NO_THROW(sock.bind(ai));

    // Mock system function
    umock::Sys_socketMock mocked_sys_socketObj;
    umock::Sys_socket sys_socket_injectObj(&mocked_sys_socketObj);
    EXPECT_CALL(
        mocked_sys_socketObj,
        getsockname((SOCKET)sock, _, Pointee((int)sizeof(sockaddr_storage))))
        .WillOnce(SetErrnoAndReturn(ENOBUFS, SOCKET_ERROR));

    // Test Unit
    EXPECT_THAT([&sock]() { sock.get_addr_str(); },
                ThrowsMessage<std::runtime_error>(
                    StartsWith("ERROR! Failed to get socket address/port:")));
}

TEST(SocketTestSuite, get_addr_str_invalid_address_family) {
    WINSOCK_INIT_P

    // Get local interface address.
    const CAddrinfo ai("", "50016", AF_INET6, SOCK_STREAM,
                       AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);
    // Get a socket and bind it to the local address.
    CSocket sock(AF_INET6, SOCK_STREAM);
    ASSERT_NO_THROW(sock.bind(ai));

    // Provide invalid address family.
    sockaddr_storage ss{};
    ss.ss_family = (sa_family_t)255;

    // Mock system function
    umock::Sys_socketMock mocked_sys_socketObj;
    umock::Sys_socket sys_socket_injectObj(&mocked_sys_socketObj);
    // This is called one time (.WillOnce()), but if the expectation fails it is
    // called two times. I don't know why. Maybe it is an issue of gmock? --Ingo
    EXPECT_CALL(
        mocked_sys_socketObj,
        getsockname((SOCKET)sock, _, Pointee((int)sizeof(sockaddr_storage))))
        .WillRepeatedly(DoAll(SetArgPointee<1>(*(sockaddr*)&ss), Return(0)));

    // Test Unit
    EXPECT_THAT([&sock]() { sock.get_addr_str(); },
                ThrowsMessage<std::invalid_argument>(
                    "ERROR! Failed to get a socket address string (IP "
                    "address): unknown address family 255"));
}

} // namespace upnplib


int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include <upnplib/gtest_main.inc>
    return gtest_return_code; // managed in upnplib/gtest_main.inc
}
