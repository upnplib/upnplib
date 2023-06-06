// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-06-08

#include <upnplib/socket.hpp>

#include <umock/sys_socket_mock.hpp>


namespace upnplib {
bool old_code{false}; // Managed in upnplib_gtest_main.inc
bool github_actions = std::getenv("GITHUB_ACTIONS");

using testing::_;
using testing::DoAll;
using testing::Pointee;
using testing::Return;
using testing::SetArgPointee;
using testing::SetErrnoAndReturn;
using testing::StartsWith;
using testing::ThrowsMessage;


TEST(SocketTestSuite, get_unbind_ipv6_stream_socket_successful) {
    // Test Unit
    CSocket sockObj(AF_INET6, SOCK_STREAM);

    // An unbound socket returns the unknown ip address and port 0
    EXPECT_NE((SOCKET)sockObj, INVALID_SOCKET);
    EXPECT_EQ(sockObj.get_addr_str(), "[::]");
    EXPECT_EQ(sockObj.get_port(), 0);
    EXPECT_EQ(sockObj.get_af(), AF_INET6);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    EXPECT_FALSE(sockObj.is_v6only());
    EXPECT_FALSE(sockObj.is_bound());
    EXPECT_FALSE(sockObj.is_listen());
}

TEST(SocketTestSuite, get_unbind_ipv6_dgram_socket_successful) {
    // Test Unit
    CSocket sockObj(AF_INET6, SOCK_DGRAM);

    // An unbound socket returns the unknown ip address and port 0
    EXPECT_NE((SOCKET)sockObj, INVALID_SOCKET);
    EXPECT_EQ(sockObj.get_addr_str(), "[::]");
    EXPECT_EQ(sockObj.get_port(), 0);
    EXPECT_EQ(sockObj.get_af(), AF_INET6);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    // This is always true, any setting is ignored
    EXPECT_TRUE(sockObj.is_v6only());
    EXPECT_FALSE(sockObj.is_bound());
    EXPECT_FALSE(sockObj.is_listen());
}

TEST(SocketTestSuite, get_unbind_ipv4_socket_successful) {
    // Test Unit
    CSocket sockObj(AF_INET, SOCK_STREAM);

    // An unbound socket returns the unknown ip address and port 0
    EXPECT_NE((SOCKET)sockObj, INVALID_SOCKET);
    EXPECT_EQ(sockObj.get_addr_str(), "0.0.0.0");
    EXPECT_EQ(sockObj.get_port(), 0);
    EXPECT_EQ(sockObj.get_af(), AF_INET);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    EXPECT_FALSE(sockObj.is_v6only());
    EXPECT_FALSE(sockObj.is_bound());
    EXPECT_FALSE(sockObj.is_listen());
}

TEST(SocketTestSuite, move_socket_successful) {
    // Provide a socket object
    CSocket sock1(AF_INET, SOCK_STREAM);
    SOCKET old_fd_sock1 = sock1;

    // Get local interface address if node is empty and flag AI_PASSIVE is set.
    // Socket type (SOCK_STREAM) must be the same as that from the socket.
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
    EXPECT_TRUE(sock2.is_bound());
    EXPECT_TRUE(sock2.is_listen());
}

TEST(SocketTestSuite, assign_socket_successful) {
    // Get local interface address if node is empty and flag AI_PASSIVE is set.
    // Socket type (SOCK_STREAM) must be the same as that from the socket.
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
    EXPECT_TRUE(sock2.is_bound());
    EXPECT_TRUE(sock2.is_listen());
}

TEST(SocketTestSuite, instantiate_empty_socket) {
    // Test Unit
    CSocket sock;
    EXPECT_EQ((SOCKET)sock, INVALID_SOCKET);

    // All getter from an INVALID_SOCKET throw an exception.
    EXPECT_THAT(
        [&sock]() { sock.get_addr_str(); },
        ThrowsMessage<std::runtime_error>(StartsWith("UPnPlib ERROR 1011!")));
    EXPECT_THAT(
        [&sock]() { sock.get_port(); },
        ThrowsMessage<std::runtime_error>(StartsWith("UPnPlib ERROR 1011!")));
    EXPECT_THAT(
        [&sock]() { sock.get_af(); },
        ThrowsMessage<std::runtime_error>(
            "ERROR! Failed to get address family: \"Bad file descriptor\""));
    EXPECT_THAT(
        [&sock]() { sock.get_sockerr(); },
        ThrowsMessage<std::runtime_error>(StartsWith("UPnPlib ERROR 1011!")));
    EXPECT_THAT(
        [&sock]() { sock.is_reuse_addr(); },
        ThrowsMessage<std::runtime_error>(StartsWith("UPnPlib ERROR 1013!")));
    EXPECT_THAT([&sock]() { sock.is_v6only(); },
                ThrowsMessage<std::runtime_error>(
                    "ERROR! Failed to get socket option 'is_v6only': \"Bad "
                    "file descriptor\""));
    EXPECT_THAT([&sock]() { sock.is_bound(); },
                ThrowsMessage<std::runtime_error>(
                    "UPnPlib ERROR 1010! Failed to get socket option "
                    "'is_bound': \"Bad file "
                    "descriptor\""));
    EXPECT_THAT([&sock]() { sock.is_listen(); },
                ThrowsMessage<std::runtime_error>(
                    "ERROR! Failed to get socket option "
                    "'is_Listen': \"Bad file descriptor\""));
}

TEST(SocketTestSuite, instantiate_with_invalid_socket_fd) {
    // Test Unit
    EXPECT_THAT(
        []() { CSocket sockObj((SOCKET)32000); },
        ThrowsMessage<std::runtime_error>(StartsWith("UPnPlib ERROR 1012!")));
}

TEST(SocketTestSuite, instantiate_with_unbound_socket_fd) {
    // Get a valid socket file descriptor
    const SOCKET sfd = socket(AF_INET6, SOCK_STREAM, 0);

    // Test Unit
    CSocket sockObj(sfd);

    EXPECT_EQ((SOCKET)sockObj, sfd);
    EXPECT_EQ(sockObj.get_addr_str(), "[::]");
    EXPECT_EQ(sockObj.get_port(), 0);
    EXPECT_EQ(sockObj.get_af(), AF_INET6);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    EXPECT_FALSE(sockObj.is_v6only());
    EXPECT_FALSE(sockObj.is_bound());
    EXPECT_FALSE(sockObj.is_listen());

    CLOSE_SOCKET_P(sfd);
}

TEST(SocketTestSuite, instantiate_with_bound_socket_fd) {
    // Get local interface address
    const CAddrinfo ai("", "50023", AF_INET6, SOCK_STREAM,
                       AI_NUMERICHOST | AI_NUMERICSERV);

    // and bind it to a socket.
    CSocket bound_sockObj(AF_INET6, SOCK_STREAM);
    EXPECT_FALSE(bound_sockObj.is_v6only());
    ASSERT_NO_THROW(bound_sockObj.bind(ai));
    EXPECT_TRUE(bound_sockObj.is_v6only());
    SOCKET bound_sock = bound_sockObj;

    // Test Unit with a bound socket.
    CSocket sockObj((SOCKET)bound_sock);

    EXPECT_EQ((SOCKET)sockObj, bound_sock);
    EXPECT_EQ(sockObj.get_addr_str(), "[::1]");
    EXPECT_EQ(sockObj.get_port(), 50023);
    EXPECT_EQ(sockObj.get_af(), AF_INET6);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    EXPECT_TRUE(sockObj.is_v6only());
    EXPECT_TRUE(sockObj.is_bound());
    EXPECT_FALSE(sockObj.is_listen());
}

TEST(SocketBindTestSuite, bind_ipv6_successful) {
    // Get local interface address with service.
    const CAddrinfo ai("[::1]", "50044", AF_INET6, SOCK_STREAM,
                       AI_NUMERICHOST | AI_NUMERICSERV);

    // Create an unbound socket object
    CSocket sockObj(AF_INET6, SOCK_STREAM);

    // Test Unit.
    // This binds the local address to the socket.
    ASSERT_NO_THROW(sockObj.bind(ai));

    EXPECT_EQ(sockObj.get_addr_str(), "[::1]");
    EXPECT_EQ(sockObj.get_port(), 50044);
    EXPECT_EQ(sockObj.get_af(), AF_INET6);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    // v6only is true because it is a socket property that's of domain AF_INET6.
    EXPECT_TRUE(sockObj.is_v6only());
    EXPECT_TRUE(sockObj.is_bound());
    EXPECT_FALSE(sockObj.is_listen());
}

TEST(SocketBindTestSuite, bind_ipv4_successful) {
    // Get local interface address with service.
    const CAddrinfo ai("127.0.0.1", "50045", AF_INET, SOCK_STREAM,
                       AI_NUMERICHOST | AI_NUMERICSERV);

    // Create an unbound socket object
    CSocket sockObj(AF_INET, SOCK_STREAM);

    // Test Unit.
    // This binds the local address to the socket.
    ASSERT_NO_THROW(sockObj.bind(ai));

    EXPECT_EQ(sockObj.get_addr_str(), "127.0.0.1");
    EXPECT_EQ(sockObj.get_port(), 50045);
    EXPECT_EQ(sockObj.get_af(), AF_INET);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    // v6only is false because it is a socket property that's of domain AF_INET.
    EXPECT_FALSE(sockObj.is_v6only());
    EXPECT_TRUE(sockObj.is_bound());
    EXPECT_FALSE(sockObj.is_listen());
}

TEST(SocketBindTestSuite, bind_only_node_successful) {
    // With empty service the operating system returns next free port number.

    // Get local interface address with no service.
    const CAddrinfo ai("[::1]", "", AF_INET6, SOCK_STREAM,
                       AI_NUMERICHOST | AI_NUMERICSERV);

    // Create an unbind socket object
    CSocket sockObj(AF_INET6, SOCK_STREAM);

    // Test Unit.
    ASSERT_NO_THROW(sockObj.bind(ai));

    EXPECT_EQ(sockObj.get_addr_str(), "[::1]");
    // Next free port number but never 0.
    EXPECT_GT(sockObj.get_port(), 0);
    EXPECT_EQ(sockObj.get_af(), AF_INET6);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    // v6only is true because it is a socket property that's of domain AF_INET6.
    EXPECT_TRUE(sockObj.is_v6only());
    EXPECT_TRUE(sockObj.is_bound());
    EXPECT_FALSE(sockObj.is_listen());
}

TEST(SocketBindTestSuite, bind_only_service_successful) {
    // Get local interface address.
    const CAddrinfo ai("", "50012", AF_INET6, SOCK_STREAM,
                       AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);

    // Create an unbind socket object.
    CSocket sockObj(AF_INET6, SOCK_STREAM);

    // Test Unit.
    ASSERT_NO_THROW(sockObj.bind(ai));

    // With ai passive setting (for listening) the presented address is the
    // unknown address. When using this to listen, it will listen on all local
    // network interfaces.
    EXPECT_EQ(sockObj.get_addr_str(), "[::]");
    EXPECT_EQ(sockObj.get_port(), 50012);
    EXPECT_EQ(sockObj.get_af(), AF_INET6);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    // v6only is always false from the operating system, no matter what domain
    // (AF_INET6) is set if we request a passive address information (flag
    // AI_PASSIVE).
    EXPECT_FALSE(sockObj.is_v6only());
    EXPECT_TRUE(sockObj.is_bound());
    EXPECT_FALSE(sockObj.is_listen());
}

TEST(SocketBindTestSuite, bind_with_wrong_addresses) {
    // Get local interface address.
    const CAddrinfo ai("", "50026", AF_INET6, SOCK_STREAM,
                       AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);

    // Test Unit. Binding an empty socket object will fail.
    EXPECT_THAT(
        [&ai]() {
            CSocket sock;
            sock.bind(ai);
        },
        ThrowsMessage<std::runtime_error>(StartsWith(
            "UPnPlib ERROR 1012! Failed to get socket option SO_TYPE:")));

    // Test Unit. Binding with a different AF_INET will fail.
    EXPECT_THAT(
        [&ai]() {
            CSocket sock(AF_INET, SOCK_STREAM);
            sock.bind(ai);
        },
        ThrowsMessage<std::runtime_error>(StartsWith(
            "UPnPlib ERROR 1008! Failed to bind socket to an address:")));
}

TEST(SocketBindTestSuite, bind_with_different_socket_type) {
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

TEST(SocketBindTestSuite, bind_two_times_different_addresses_fail) {
    // Binding a socket two times isn't possible. The socket must be
    // shutdown/closed before bind it again.
    // Provide a socket object
    CSocket sock(AF_INET6, SOCK_STREAM);

    // Get local interface address. Socket type (SOCK_STREAM) must be the same
    // as that from the socket.
    CAddrinfo ai1("", "50017", AF_INET6, SOCK_STREAM,
                  AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);

    // Test Unit.
    ASSERT_NO_THROW(sock.bind(ai1));

    // Try to bind the socket a second time.
    EXPECT_THAT(
        [&sock]() {
            CAddrinfo ai2("", "50018", AF_INET6, SOCK_STREAM,
                          AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);
            sock.bind(ai2);
        },
        ThrowsMessage<std::runtime_error>(StartsWith(
            "UPnPlib ERROR 1008! Failed to bind socket to an address: ")));
}

TEST(SocketBindTestSuite, set_unset_bind_listen_same_address_multiple_times) {
    // Binding a socket again is possible after destruction of the socket that
    // shutdown/close it.
    // Get local interface address.
    CAddrinfo ai1("", "50019", AF_INET6, SOCK_STREAM,
                  AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);

    // Test Unit
    {
        CSocket sock(AF_INET6, SOCK_STREAM);
        EXPECT_NO_THROW(sock.bind(ai1));
        EXPECT_NO_THROW(sock.listen());
    }
    {
        CSocket sock(AF_INET6, SOCK_STREAM);
        EXPECT_NO_THROW(sock.bind(ai1));
        EXPECT_NO_THROW(sock.listen());
    }
    CSocket sock(AF_INET6, SOCK_STREAM);
    EXPECT_NO_THROW(sock.bind(ai1));
    EXPECT_NO_THROW(sock.listen());
}

TEST(SocketBindTestSuite, bind_same_address_two_times) {
    // SKIP on Github Actions
    if (github_actions)
        GTEST_SKIP() << "             known failing test on Github Actions";

    // Binding the same address again to a valid socket is possible and should
    // do nothing.
    // Get local interface address.
    CAddrinfo ai1("", "50024", AF_INET6, SOCK_STREAM,
                  AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);

    // Test Unit
    CSocket sock(AF_INET6, SOCK_STREAM);
    EXPECT_NO_THROW(sock.bind(ai1));

    EXPECT_EQ(sock.get_addr_str(), "[::]");
    EXPECT_EQ(sock.get_port(), 50024);
    EXPECT_EQ(sock.get_af(), AF_INET6);
    EXPECT_EQ(sock.get_sockerr(), 0);
    EXPECT_FALSE(sock.is_reuse_addr());
    EXPECT_FALSE(sock.is_v6only());
    EXPECT_TRUE(sock.is_bound());
    EXPECT_FALSE(sock.is_listen());

    // Doing the same again doesn't hurt.
    EXPECT_NO_THROW(sock.bind(ai1));

    EXPECT_EQ(sock.get_addr_str(), "[::]");
    EXPECT_EQ(sock.get_port(), 50024);
    EXPECT_EQ(sock.get_af(), AF_INET6);
    EXPECT_EQ(sock.get_sockerr(), 0);
    EXPECT_FALSE(sock.is_reuse_addr());
    EXPECT_FALSE(sock.is_v6only());
    EXPECT_TRUE(sock.is_bound());
    EXPECT_FALSE(sock.is_listen());
}

TEST(SocketBindTestSuite, listen_to_same_address_multiple_times) {
    // Listen on the same address again of a valid socket is possible and should
    // do nothing.

    // Get local interface address.
    CAddrinfo ai1("", "50025", AF_INET6, SOCK_STREAM,
                  AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);

    // Test Unit
    CSocket sockObj(AF_INET6, SOCK_STREAM);
    ASSERT_NO_THROW(sockObj.bind(ai1));
    ASSERT_NO_THROW(sockObj.listen());

    EXPECT_NO_THROW(sockObj.listen());
}

TEST(SocketBindTestSuite, is_bound_successful) {
    //
}

TEST(SocketTestSuite, set_wrong_arguments) {
    // Test Unit. Set wrong address family.
    EXPECT_THAT([]() { CSocket sock((sa_family_t)-1, SOCK_STREAM); },
                ThrowsMessage<std::runtime_error>(
                    StartsWith("ERROR! Failed to create socket: ")));

    // Test Unit. Set wrong socket type.
    EXPECT_THAT([]() { CSocket sock(AF_INET6, -1); },
                ThrowsMessage<std::runtime_error>(
                    StartsWith("ERROR! Failed to create socket: ")));
}

// // This has already be done as part of other tests.
// TEST(SocketTestSuite, listen_successful) {
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

TEST(SocketV6onlyTestSuite, modify_v6only_af_inet6_stream_successful) {
    // Get socket
    CSocket sockObj(AF_INET6, SOCK_STREAM);
    EXPECT_FALSE(sockObj.is_v6only());

    // Set option IPV6_V6ONLY. Of course this can only be done with AF_INET6 on
    // an unbound socket.
    // Test Unit
    ASSERT_NO_THROW(sockObj.set_v6only(true));
    EXPECT_TRUE(sockObj.is_v6only());
    ASSERT_NO_THROW(sockObj.set_v6only(false));
    EXPECT_FALSE(sockObj.is_v6only());
}

TEST(SocketV6onlyTestSuite, modify_v6only_af_inet6_dgram) {
    // With SOCK_DGRAM the flag IP6_V6ONLY is always set and ignores any
    // modification.

    // Get socket
    CSocket sockObj(AF_INET6, SOCK_DGRAM);
    EXPECT_TRUE(sockObj.is_v6only());

    // Set option IPV6_V6ONLY. This is always ignored.
    // Test Unit
    ASSERT_NO_THROW(sockObj.set_v6only(false));
    EXPECT_TRUE(sockObj.is_v6only());
}

TEST(SocketV6onlyTestSuite, modify_check_af_inet_v6only) {
    // This must always return false because on AF_INET we cannot have v6only.
    // Get socket
    CSocket sockObj(AF_INET, SOCK_STREAM);
    EXPECT_FALSE(sockObj.is_v6only());

    // Test Unit
    ASSERT_NO_THROW(sockObj.set_v6only(true));
    EXPECT_FALSE(sockObj.is_v6only());
    ASSERT_NO_THROW(sockObj.set_v6only(false));
    EXPECT_FALSE(sockObj.is_v6only());
}

TEST(SocketV6onlyTestSuite, modify_v6only_on_bound_af_inet6_socket) {
    // Get local interface address with service.
    const CAddrinfo ai("[::1]", "50047", AF_INET6, SOCK_DGRAM,
                       AI_NUMERICHOST | AI_NUMERICSERV);

    // Create an unbound socket object
    CSocket sockObj(AF_INET6, SOCK_DGRAM);
    EXPECT_TRUE(sockObj.is_v6only()); // Always set with SOCK_DGRAM

    // This binds the local address to the socket and always sets ipv6_v6only
    // because we bind to an AF_INET6 address/socket.
    ASSERT_NO_THROW(sockObj.bind(ai));
    EXPECT_TRUE(sockObj.is_v6only());

    // Test Unit
    EXPECT_NO_THROW(sockObj.set_v6only(false));

    // Nothing has changed, the socket is valid.
    EXPECT_EQ(sockObj.get_addr_str(), "[::1]");
    EXPECT_EQ(sockObj.get_port(), 50047);
    EXPECT_EQ(sockObj.get_af(), AF_INET6);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    // Not set to false because the socket is already bound to an AF_INET6
    // address and because it is always set with SOCK_DGRAM.
    EXPECT_TRUE(sockObj.is_v6only());
    EXPECT_TRUE(sockObj.is_bound());
    EXPECT_FALSE(sockObj.is_listen());
}

TEST(SocketV6onlyTestSuite, modify_v6only_on_bound_af_inet_socket) {
    // Get local interface address with service.
    const CAddrinfo ai("127.0.0.1", "50048", AF_INET, SOCK_STREAM,
                       AI_NUMERICHOST | AI_NUMERICSERV);

    // Create an unbound socket object
    CSocket sockObj(AF_INET, SOCK_STREAM);
    EXPECT_FALSE(sockObj.is_v6only());

    // This binds the local address to the socket and never sets ipv6_v6only
    // because we bind to an AF_INET address/socket.
    ASSERT_NO_THROW(sockObj.bind(ai));
    EXPECT_FALSE(sockObj.is_v6only());

    // Test Unit
    // This is silently ignored.
    EXPECT_NO_THROW(sockObj.set_v6only(true));

    // Nothing has changed, the socket is valid.
    EXPECT_EQ(sockObj.get_addr_str(), "127.0.0.1");
    EXPECT_EQ(sockObj.get_port(), 50048);
    EXPECT_EQ(sockObj.get_af(), AF_INET);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    // Never set to true because it is an AF_INET socket.
    EXPECT_FALSE(sockObj.is_v6only());
    EXPECT_TRUE(sockObj.is_bound());
    EXPECT_FALSE(sockObj.is_listen());
}

TEST(SocketV6onlyTestSuite, unset_v6only_on_passive_af_inet6_socket) {
    // Get local interface address with service.
    const CAddrinfo ai("", "50049", AF_INET6, SOCK_STREAM,
                       AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);

    // Create an unbound socket object
    CSocket sockObj(AF_INET6, SOCK_STREAM);
    EXPECT_FALSE(sockObj.is_v6only());

    // Test Unit
    // This binds the local address to the socket but does not modify
    // IPV6_V6ONLY because we bind to a passive AF_INET6 address/socket.
    ASSERT_NO_THROW(sockObj.bind(ai));
    // Even with AF_INET6 the flag is still unset.
    EXPECT_FALSE(sockObj.is_v6only());

    // It cannot be set afer binding.
    EXPECT_NO_THROW(sockObj.set_v6only(true));

    // Nothing has changed, the socket is valid.
    EXPECT_EQ(sockObj.get_addr_str(), "[::]");
    EXPECT_EQ(sockObj.get_port(), 50049);
    EXPECT_EQ(sockObj.get_af(), AF_INET6);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    // Not set to true because the socket is already bound to an AF_INET6
    // address.
    EXPECT_FALSE(sockObj.is_v6only());
    EXPECT_TRUE(sockObj.is_bound());
    EXPECT_FALSE(sockObj.is_listen());
}

TEST(SocketV6onlyTestSuite, set_v6only_on_passive_af_inet6_socket) {
    // Get local interface address with service.
    const CAddrinfo ai("", "50051", AF_INET6, SOCK_DGRAM,
                       AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);

    // Create an unbound socket object
    CSocket sockObj(AF_INET6, SOCK_DGRAM);
    EXPECT_TRUE(sockObj.is_v6only()); // Always set with SOCK_DGRAM

    EXPECT_NO_THROW(sockObj.set_v6only(true));
    EXPECT_TRUE(sockObj.is_v6only());

    // Test Unit
    // This binds the local address to the socket but does not modify
    // IPV6_V6ONLY because we bind to a passive AF_INET6 address/socket.
    ASSERT_NO_THROW(sockObj.bind(ai));
    // Even with AF_INET6 the flag is still unset.
    EXPECT_TRUE(sockObj.is_v6only());

    // It cannot be set afer binding.
    EXPECT_NO_THROW(sockObj.set_v6only(false));

    // Nothing has changed, the socket is valid.
    EXPECT_EQ(sockObj.get_addr_str(), "[::]");
    EXPECT_EQ(sockObj.get_port(), 50051);
    EXPECT_EQ(sockObj.get_af(), AF_INET6);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    // Not set to false because the socket is already bound to an AF_INET6
    // address and because it is always set with SOCK_DGRAM.
    EXPECT_TRUE(sockObj.is_v6only());
    EXPECT_TRUE(sockObj.is_bound());
    EXPECT_FALSE(sockObj.is_listen());
}

TEST(SocketV6onlyTestSuite, modify_v6only_on_passive_af_inet_socket) {
    // Get local interface address with service.
    const CAddrinfo ai("", "50050", AF_INET, SOCK_DGRAM,
                       AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);

    // Create an unbound socket object
    CSocket sockObj(AF_INET, SOCK_DGRAM);
    EXPECT_FALSE(sockObj.is_v6only());

    // Test Unit
    // This binds the local address to the socket and always resets ipv6_v6only
    // even with passive mode because we bind to an AF_INET address/socket.
    ASSERT_NO_THROW(sockObj.bind(ai));
    EXPECT_FALSE(sockObj.is_v6only());

    // It cannot be set afer binding.
    EXPECT_NO_THROW(sockObj.set_v6only(true));

    // Nothing has changed, the socket is valid.
    EXPECT_EQ(sockObj.get_addr_str(), "0.0.0.0");
    EXPECT_EQ(sockObj.get_port(), 50050);
    EXPECT_EQ(sockObj.get_af(), AF_INET);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    // Not set to true because the socket is already bound to an AF_INET
    // address.
    EXPECT_FALSE(sockObj.is_v6only());
    EXPECT_TRUE(sockObj.is_bound());
    EXPECT_FALSE(sockObj.is_listen());
}

TEST(SocketTestSuite, get_addr_str_ipv6_successful) {
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
        ThrowsMessage<std::runtime_error>(StartsWith("UPnPlib ERROR 1011!")));
}

TEST(SocketTestSuite, get_addr_str_from_unbound_socket) {
    // Get a valid socket but do not bind it to an address.
    CSocket sock(AF_INET6, SOCK_STREAM);
    EXPECT_EQ(sock.get_addr_str(), "[::]");
    EXPECT_EQ(sock.get_port(), 0);
#if 0
    EXPECT_THAT(
        []() {
            CSocket sock(AF_INET6, SOCK_STREAM);
            sock.get_addr_str();
        },
        ThrowsMessage<std::runtime_error>(
            "ERROR! Failed to get socket address/port: \"not bound to an "
            "address\""));
#endif
}

TEST(SocketTestSuite, get_addr_str_syscall_fail) {
    // Get local interface address.
    const CAddrinfo ai("", "50015", AF_INET6, SOCK_STREAM,
                       AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);
    // Get a socket and bind it to the local address.
    CSocket sock(AF_INET6, SOCK_STREAM);
    ASSERT_NO_THROW(sock.bind(ai));

    // Mock system function getsockname().
    umock::Sys_socketMock mocked_sys_socketObj;
    umock::Sys_socket sys_socket_injectObj(&mocked_sys_socketObj);
    EXPECT_CALL(
        mocked_sys_socketObj,
        getsockname((SOCKET)sock, _, Pointee((int)sizeof(sockaddr_storage))))
        .WillOnce(SetErrnoAndReturn(ENOBUFS, SOCKET_ERROR));

    // Test Unit
    EXPECT_THAT(
        [&sock]() { sock.get_addr_str(); },
        ThrowsMessage<std::runtime_error>(StartsWith("UPnPlib ERROR 1001!")));
}

TEST(SocketTestSuite, get_addr_str_invalid_address_family) {
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
                    StartsWith("UPnPlib ERROR 1002!")));
}

} // namespace upnplib


int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
    WINSOCK_INIT_P
#include <upnplib/gtest_main.inc>
    return gtest_return_code; // managed in upnplib/gtest_main.inc
}
