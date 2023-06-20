// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-06-21

#include <upnplib/socket.hpp>
#include <upnplib/addrinfo.hpp>

#include <umock/sys_socket_mock.hpp>

#include <random>

#include <gmock/gmock.h>


namespace upnplib {
bool old_code{false}; // Managed in upnplib_gtest_main.inc
bool github_actions = std::getenv("GITHUB_ACTIONS");

using ::testing::_;
using ::testing::DoAll;
using ::testing::Pointee;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::SetErrnoAndReturn;
using ::testing::StartsWith;
using ::testing::ThrowsMessage;


// Create a simple random number generator for port numbers.
// We need this because we do not reuse addresses before TIME_WAIT has expired
// (socket option SO_REUSEADDR = false). Quick Repeating tests may violate this
// rule and fail. So we have always to use different socket addresses and that
// is already given with different port numbers.
std::random_device rd;         // obtain a random number from hardware
std::minstd_rand random(rd()); // seed the generator
std::uniform_int_distribution<in_port_t> portno(49152, 65535); // define range


TEST(SocketBasicTestSuite, instantiate_socket_successful) {
    SOCKET sfd = ::socket(AF_INET6, SOCK_STREAM, 0);
    ASSERT_NE(sfd, INVALID_SOCKET);

    // Test Unit
    CSocket_basic sockObj(sfd);

    EXPECT_EQ((SOCKET)sockObj, sfd);
    EXPECT_EQ(sockObj.get_family(), AF_INET6);
    EXPECT_EQ(sockObj.get_type(), SOCK_STREAM);
    EXPECT_EQ(sockObj.get_addr_str(), "[::]");
    EXPECT_EQ(sockObj.get_port(), 0);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());

    CLOSE_SOCKET_P(sfd);
}

TEST(SocketBasicTestSuite, instantiate_with_bound_socket_fd) {
    // Get local interface address
    const CAddrinfo ai("", "8080", AF_INET6, SOCK_STREAM,
                       AI_NUMERICHOST | AI_NUMERICSERV);

    // and bind it to a socket.
    CSocket bound_sockObj(AF_INET6, SOCK_STREAM);
    EXPECT_FALSE(bound_sockObj.is_v6only());
    ASSERT_NO_THROW(bound_sockObj.bind(ai));
    EXPECT_TRUE(bound_sockObj.is_v6only());
    SOCKET bound_sock = bound_sockObj;

    // Test Unit with a bound socket.
    CSocket_basic sockObj(bound_sock);

    EXPECT_EQ((SOCKET)sockObj, bound_sock);
    EXPECT_EQ(sockObj.get_family(), AF_INET6);
    EXPECT_EQ(sockObj.get_type(), SOCK_STREAM);
    EXPECT_EQ(sockObj.get_addr_str(), "[::1]");
    EXPECT_EQ(sockObj.get_port(), 8080);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
}

TEST(SocketBasicTestSuite, instantiate_socket_af_unix_sock_stream) {
    SOCKET sfd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    ASSERT_NE(sfd, INVALID_SOCKET);

    CSocket_basic sockObj(sfd);
    EXPECT_NE((SOCKET)sockObj, INVALID_SOCKET);
    EXPECT_EQ(sockObj.get_family(), AF_UNIX);
    EXPECT_EQ(sockObj.get_type(), SOCK_STREAM);
    EXPECT_EQ(sockObj.get_port(), 0);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    EXPECT_THAT(
        [&sockObj]() { sockObj.get_addr_str(); },
        ThrowsMessage<std::runtime_error>(StartsWith("UPnPlib ERROR 1024!")));

    CLOSE_SOCKET_P(sfd);
}

TEST(SocketBasicTestSuite, instantiate_socket_af_unix_sock_dgram) {
    SOCKET sfd = ::socket(AF_UNIX, SOCK_DGRAM, 0);

// Seems this isn't supported on Microsoft Windows
#ifndef _MSC_VER
    ASSERT_NE(sfd, INVALID_SOCKET);
    CSocket_basic sockObj(sfd);
    EXPECT_NE((SOCKET)sockObj, INVALID_SOCKET);
    EXPECT_EQ(sockObj.get_family(), AF_UNIX);
    EXPECT_EQ(sockObj.get_type(), SOCK_DGRAM);
    EXPECT_EQ(sockObj.get_port(), 0);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
#else
    EXPECT_EQ(sfd, INVALID_SOCKET);
#endif

    CLOSE_SOCKET_P(sfd);
}

TEST(SocketBasicTestSuite, instantiate_socket_af_unix_sock_raw) {
    // The GCC compiler accepts with family AF_UNIX the types SOCK_STREAM,
    // SOCK_DGRAM, SOCK_RAW (changes silently to SOCK_DGRAM) and
    // SOCK_SEQPACKET.
    SOCKET sfd = ::socket(AF_UNIX, SOCK_RAW, 0);

// Seems this is only supported by the GCC compiler
#if defined(__GNUC__) && !defined(__clang__)
    ASSERT_NE(sfd, INVALID_SOCKET);
    CSocket_basic sockObj(sfd);
    EXPECT_NE((SOCKET)sockObj, INVALID_SOCKET);
    EXPECT_EQ(sockObj.get_family(), AF_UNIX);
    // Silently changed
    EXPECT_EQ(sockObj.get_type(), SOCK_DGRAM);
    EXPECT_EQ(sockObj.get_port(), 0);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
#else
    EXPECT_EQ(sfd, INVALID_SOCKET);
#endif

    CLOSE_SOCKET_P(sfd);
}

TEST(SocketBasicTestSuite, instantiate_socket_af_unix_sock_seqpacket) {
    // The GCC compiler accepts with family AF_UNIX the types SOCK_STREAM,
    // SOCK_DGRAM, SOCK_RAW (changes silently to SOCK_DGRAM) and
    // SOCK_SEQPACKET.
    SOCKET sfd = ::socket(AF_UNIX, SOCK_SEQPACKET, 0);

// Seems this is only supported by the GCC compiler
#if defined(__GNUC__) && !defined(__clang__)
    ASSERT_NE(sfd, INVALID_SOCKET);
    CSocket_basic sockObj(sfd);
    EXPECT_NE((SOCKET)sockObj, INVALID_SOCKET);
    EXPECT_EQ(sockObj.get_port(), 0);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
#else
    EXPECT_EQ(sfd, INVALID_SOCKET);
#endif

    CLOSE_SOCKET_P(sfd);
}

TEST(SocketTestSuite, verify_system_socket_function) {
    // This verfies different behavior on different platforms.
    EXPECT_EQ(::socket(0, 0, 0), INVALID_SOCKET);
    EXPECT_EQ(::socket(0, SOCK_STREAM, 0), INVALID_SOCKET);
    EXPECT_EQ(::socket(0, SOCK_DGRAM, 0), INVALID_SOCKET);
    EXPECT_EQ(::socket(AF_UNSPEC, 0, 0), INVALID_SOCKET);
    EXPECT_EQ(::socket(AF_UNSPEC, SOCK_STREAM, 0), INVALID_SOCKET);
    EXPECT_EQ(::socket(AF_UNSPEC, SOCK_DGRAM, 0), INVALID_SOCKET);
    EXPECT_EQ(::socket(AF_UNIX, SOCK_RDM, 0), INVALID_SOCKET);

    SOCKET sfd{INVALID_SOCKET};
#ifdef _MSC_VER
    sfd = ::socket(AF_INET, 0, 0);
    EXPECT_NE(sfd, INVALID_SOCKET);
    CLOSE_SOCKET_P(sfd);
    sfd = ::socket(AF_INET6, 0, 0);
    EXPECT_NE(sfd, INVALID_SOCKET);
    CLOSE_SOCKET_P(sfd);
#else
    EXPECT_EQ(::socket(AF_INET, 0, 0), INVALID_SOCKET);
    EXPECT_EQ(::socket(AF_INET6, 0, 0), INVALID_SOCKET);
#endif

    sfd = ::socket(AF_INET, SOCK_STREAM, 0);
    EXPECT_NE(sfd, INVALID_SOCKET);
    CLOSE_SOCKET_P(sfd);
    sfd = ::socket(AF_INET, SOCK_DGRAM, 0);
    EXPECT_NE(sfd, INVALID_SOCKET);
    CLOSE_SOCKET_P(sfd);
    sfd = ::socket(AF_INET6, SOCK_STREAM, 0);
    EXPECT_NE(sfd, INVALID_SOCKET);
    CLOSE_SOCKET_P(sfd);
    sfd = ::socket(AF_INET6, SOCK_DGRAM, 0);
    EXPECT_NE(sfd, INVALID_SOCKET);
    CLOSE_SOCKET_P(sfd);
}

TEST(SocketTestSuite, verify_system_getsockname_function) {
    // int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    SOCKET sfd = ::socket(AF_INET6, SOCK_STREAM, 0);
    ASSERT_NE(sfd, INVALID_SOCKET);

    ::sockaddr_storage ss{};
    socklen_t len = sizeof(ss); // May be modified

#ifndef _MSC_VER
    EXPECT_EQ(::getsockname(sfd, (sockaddr*)&ss, &len), 0);
    EXPECT_EQ(ss.ss_family, AF_INET6);
#else
    ss.ss_family = 0xAAAA;
    EXPECT_NE(::getsockname(sfd, (sockaddr*)&ss, &len), 0);
    EXPECT_EQ(ss.ss_family, 0xAAAA);
#endif
    CLOSE_SOCKET_P(sfd);
}

TEST(SocketTestSuite, get_unbound_ipv6_stream_socket_successful) {
    // Test Unit
    CSocket sockObj(AF_INET6, SOCK_STREAM);

    // An unbound socket returns the unknown ip address and port 0
    EXPECT_NE((SOCKET)sockObj, INVALID_SOCKET);
    EXPECT_EQ(sockObj.get_family(), AF_INET6);
    EXPECT_EQ(sockObj.get_type(), SOCK_STREAM);
    EXPECT_EQ(sockObj.get_addr_str(), "[::]");
    EXPECT_EQ(sockObj.get_port(), 0);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    EXPECT_FALSE(sockObj.is_v6only());
    EXPECT_FALSE(sockObj.is_bound());
    EXPECT_FALSE(sockObj.is_listen());
}

TEST(SocketTestSuite, get_unbound_ipv6_dgram_socket_successful) {
    // Test Unit
    CSocket sockObj(AF_INET6, SOCK_DGRAM);

    // An unbound socket returns the unknown ip address and port 0
    EXPECT_NE((SOCKET)sockObj, INVALID_SOCKET);
    EXPECT_EQ(sockObj.get_family(), AF_INET6);
    EXPECT_EQ(sockObj.get_type(), SOCK_DGRAM);
    EXPECT_EQ(sockObj.get_addr_str(), "[::]");
    EXPECT_EQ(sockObj.get_port(), 0);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    // This is always true on win32, any setting is ignored?
    EXPECT_FALSE(sockObj.is_v6only());
    EXPECT_FALSE(sockObj.is_bound());
    EXPECT_FALSE(sockObj.is_listen());
}

TEST(SocketTestSuite, get_unbound_ipv4_stream_socket_successful) {
    // Test Unit
    CSocket sockObj(AF_INET, SOCK_STREAM);

    // An unbound socket returns the unknown ip address and port 0
    EXPECT_NE((SOCKET)sockObj, INVALID_SOCKET);
    EXPECT_EQ(sockObj.get_family(), AF_INET);
    EXPECT_EQ(sockObj.get_addr_str(), "0.0.0.0");
    EXPECT_EQ(sockObj.get_port(), 0);
    EXPECT_EQ(sockObj.get_type(), SOCK_STREAM);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    EXPECT_FALSE(sockObj.is_v6only());
    EXPECT_FALSE(sockObj.is_bound());
    EXPECT_FALSE(sockObj.is_listen());
}

TEST(SocketTestSuite, get_unbound_ipv4_dgram_socket_successful) {
    // Test Unit
    CSocket sockObj(AF_INET, SOCK_DGRAM);

    // An unbound socket returns the unknown ip address and port 0
    EXPECT_NE((SOCKET)sockObj, INVALID_SOCKET);
    EXPECT_EQ(sockObj.get_addr_str(), "0.0.0.0");
    EXPECT_EQ(sockObj.get_port(), 0);
    EXPECT_EQ(sockObj.get_family(), AF_INET);
    EXPECT_EQ(sockObj.get_type(), SOCK_DGRAM);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    EXPECT_FALSE(sockObj.is_v6only());
    EXPECT_FALSE(sockObj.is_bound());
    EXPECT_FALSE(sockObj.is_listen());
}

TEST(SocketTestSuite, try_to_instantiate_invalid_sockets) {
    EXPECT_THAT([]() { CSocket sockObj(AF_UNSPEC, SOCK_STREAM); },
                ThrowsMessage<std::invalid_argument>(
                    StartsWith("UPnPlib ERROR 1015!")));

    EXPECT_THAT([]() { CSocket sockObj(AF_UNIX, SOCK_STREAM); },
                ThrowsMessage<std::invalid_argument>(
                    StartsWith("UPnPlib ERROR 1015!")));

    EXPECT_THAT([]() { CSocket sockObj(0, SOCK_STREAM); },
                ThrowsMessage<std::invalid_argument>(
                    StartsWith("UPnPlib ERROR 1015!")));

    EXPECT_THAT([]() { CSocket sockObj(AF_INET6, SOCK_RAW); },
                ThrowsMessage<std::invalid_argument>(
                    StartsWith("UPnPlib ERROR 1016!")));

    EXPECT_THAT([]() { CSocket sockObj(AF_INET, 0); },
                ThrowsMessage<std::invalid_argument>(
                    StartsWith("UPnPlib ERROR 1016!")));
}

TEST(SocketTestSuite, instantiate_empty_socket) {
    // Test Unit
    CSocket sockObj;
    EXPECT_EQ((SOCKET)sockObj, INVALID_SOCKET);
    // All getter from an INVALID_SOCKET throw an exception.
    EXPECT_THAT(
        [&sockObj]() { sockObj.get_addr_str(); },
        ThrowsMessage<std::runtime_error>(StartsWith("UPnPlib ERROR 1001!")));
    EXPECT_THAT(
        [&sockObj]() { sockObj.get_port(); },
        ThrowsMessage<std::runtime_error>(StartsWith("UPnPlib ERROR 1031!")));
    EXPECT_THAT(
        [&sockObj]() { sockObj.get_family(); },
        ThrowsMessage<std::runtime_error>(StartsWith("UPnPlib ERROR 1027!")));
    EXPECT_THAT(
        [&sockObj]() { sockObj.get_type(); },
        ThrowsMessage<std::runtime_error>(StartsWith("UPnPlib ERROR 1030!")));
    EXPECT_THAT(
        [&sockObj]() { sockObj.get_sockerr(); },
        ThrowsMessage<std::runtime_error>(StartsWith("UPnPlib ERROR 1011!")));
    EXPECT_THAT(
        [&sockObj]() { sockObj.is_reuse_addr(); },
        ThrowsMessage<std::runtime_error>(StartsWith("UPnPlib ERROR 1013!")));
    EXPECT_THAT(
        [&sockObj]() { sockObj.is_v6only(); },
        ThrowsMessage<std::runtime_error>(StartsWith("UPnPlib ERROR 1028!")));
    EXPECT_THAT(
        [&sockObj]() { sockObj.is_bound(); },
        ThrowsMessage<std::runtime_error>(StartsWith("UPnPlib ERROR 1010!")));
    EXPECT_THAT(
        [&sockObj]() { sockObj.is_listen(); },
        ThrowsMessage<std::runtime_error>(StartsWith("UPnPlib ERROR 1035!")));
}

TEST(SocketTestSuite, move_socket_successful) {
    // Provide a socket object
    CSocket sock1(AF_INET, SOCK_STREAM);
    SOCKET old_fd_sock1 = sock1;

    // Get local interface address when node is empty and flag AI_PASSIVE is
    // set. Socket type (SOCK_STREAM) must be the same as that from the socket.
    CAddrinfo ai("", "8080", AF_INET, SOCK_STREAM,
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
    EXPECT_EQ(sock2.get_family(), AF_INET);
    EXPECT_EQ(sock2.get_type(), SOCK_STREAM);
    EXPECT_EQ(sock2.get_addr_str(), "0.0.0.0");
    EXPECT_EQ(sock2.get_port(), 8080);
    EXPECT_EQ(sock2.get_sockerr(), 0);
    EXPECT_FALSE(sock2.is_reuse_addr());
    EXPECT_FALSE(sock2.is_v6only());
    EXPECT_TRUE(sock2.is_bound());
    EXPECT_TRUE(sock2.is_listen());
}

TEST(SocketTestSuite, assign_socket_successful) {
    // Get local interface address when node is empty and flag AI_PASSIVE is
    // set. Socket type (SOCK_STREAM) must be the same as that from the socket.
    CAddrinfo ai("", "8080", AF_INET6, SOCK_STREAM,
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
    EXPECT_EQ(sock2.get_family(), AF_INET6);
    EXPECT_EQ(sock2.get_type(), SOCK_STREAM);
    EXPECT_EQ(sock2.get_addr_str(), "[::]");
    EXPECT_EQ(sock2.get_port(), 8080);
    EXPECT_EQ(sock2.get_sockerr(), 0);
    EXPECT_FALSE(sock2.is_reuse_addr());
    EXPECT_FALSE(sock2.is_v6only());
    EXPECT_TRUE(sock2.is_bound());
    EXPECT_TRUE(sock2.is_listen());
}

TEST(SocketTestSuite, set_wrong_arguments) {
    // Test Unit. Set wrong address family.
    EXPECT_THAT([]() { CSocket sockObj((sa_family_t)-1, SOCK_STREAM); },
                ThrowsMessage<std::invalid_argument>(StartsWith(
                    "UPnPlib ERROR 1015! Failed to create socket: ")));

    // Test Unit. Set wrong socket type.
    EXPECT_THAT([]() { CSocket sockObj(AF_INET6, -1); },
                ThrowsMessage<std::invalid_argument>(StartsWith(
                    "UPnPlib ERROR 1016! Failed to create socket: ")));
}

TEST(SocketBindTestSuite, bind_ipv6_successful) {
    // Get local interface address with service.
    const CAddrinfo ai("[::1]", "8080", AF_INET6, SOCK_STREAM,
                       AI_NUMERICHOST | AI_NUMERICSERV);

    // Create an unbound socket object
    CSocket sockObj(AF_INET6, SOCK_STREAM);
    EXPECT_FALSE(sockObj.is_v6only());
    // Can be modified with AF_INET6 and before binding
    EXPECT_NO_THROW(sockObj.set_v6only(true));
    EXPECT_TRUE(sockObj.is_v6only());
    EXPECT_NO_THROW(sockObj.set_v6only(false));
    EXPECT_FALSE(sockObj.is_v6only());

    // Test Unit.
    // This binds the local address to the socket and sets IPV6_V6ONLY always
    // to true which cannot be modified afterwards.
    ASSERT_NO_THROW(sockObj.bind(ai));
    EXPECT_TRUE(sockObj.is_v6only());
    EXPECT_NO_THROW(sockObj.set_v6only(false));

    EXPECT_EQ(sockObj.get_family(), AF_INET6);
    EXPECT_EQ(sockObj.get_type(), SOCK_STREAM);
    EXPECT_EQ(sockObj.get_addr_str(), "[::1]");
    EXPECT_EQ(sockObj.get_port(), 8080);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    // v6only is true because it is a socket property that's of domain AF_INET6.
    EXPECT_TRUE(sockObj.is_v6only());
    EXPECT_TRUE(sockObj.is_bound());
    EXPECT_FALSE(sockObj.is_listen());
}

TEST(SocketBindTestSuite, bind_ipv4_successful) {
    // Get local interface address with service.
    const CAddrinfo ai("127.0.0.1", "8080", AF_INET, SOCK_STREAM,
                       AI_NUMERICHOST | AI_NUMERICSERV);

    // Create an unbound socket object
    CSocket sockObj(AF_INET, SOCK_STREAM);
    EXPECT_FALSE(sockObj.is_v6only());
    // Cannot be set on AF_INET
    EXPECT_NO_THROW(sockObj.set_v6only(true));
    EXPECT_FALSE(sockObj.is_v6only());

    // Test Unit.
    // This binds the local address to the socket.
    ASSERT_NO_THROW(sockObj.bind(ai));
    // Can never be set after binding
    EXPECT_NO_THROW(sockObj.set_v6only(true));

    EXPECT_EQ(sockObj.get_family(), AF_INET);
    EXPECT_EQ(sockObj.get_type(), SOCK_STREAM);
    EXPECT_EQ(sockObj.get_addr_str(), "127.0.0.1");
    EXPECT_EQ(sockObj.get_port(), 8080);
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

    // Create an unbound socket object
    CSocket sockObj(AF_INET6, SOCK_STREAM);

    // Test Unit.
    ASSERT_NO_THROW(sockObj.bind(ai));

    EXPECT_EQ(sockObj.get_family(), AF_INET6);
    EXPECT_EQ(sockObj.get_type(), SOCK_STREAM);
    EXPECT_EQ(sockObj.get_addr_str(), "[::1]");
    // Next free port number but never 0.
    EXPECT_GT(sockObj.get_port(), 0);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    // v6only is true because it is a socket property that's of domain AF_INET6.
    EXPECT_TRUE(sockObj.is_v6only());
    EXPECT_TRUE(sockObj.is_bound());
    EXPECT_FALSE(sockObj.is_listen());
}

TEST(SocketBindTestSuite, bind_only_service_successful) {
    // Get local interface address.
    const CAddrinfo ai("", "8080", AF_INET6, SOCK_STREAM,
                       AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);

    // Create an unbind socket object.
    CSocket sockObj(AF_INET6, SOCK_STREAM);

    // Test Unit.
    ASSERT_NO_THROW(sockObj.bind(ai));

    EXPECT_EQ(sockObj.get_family(), AF_INET6);
    EXPECT_EQ(sockObj.get_type(), SOCK_STREAM);
    // With ai passive setting (for listening) the presented address is the
    // unknown address. When using this to listen, it will listen on all local
    // network interfaces.
    EXPECT_EQ(sockObj.get_addr_str(), "[::]");
    EXPECT_EQ(sockObj.get_port(), 8080);
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
    const CAddrinfo ai("", "8080", AF_INET6, SOCK_STREAM,
                       AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);

    // Test Unit. Binding an empty socket object will fail.
    EXPECT_THAT(
        [&ai]() {
            CSocket sockObj;
            sockObj.bind(ai);
        },
        ThrowsMessage<std::runtime_error>(StartsWith(
            "UPnPlib ERROR 1030! Failed to get socket option SO_TYPE:")));

    // Test Unit. Binding with a different AF_INET will fail.
    EXPECT_THAT(
        [&ai]() {
            CSocket sockObj(AF_INET, SOCK_STREAM);
            sockObj.bind(ai);
        },
        ThrowsMessage<std::runtime_error>(StartsWith(
            "UPnPlib ERROR 1008! Failed to bind socket to an address:")));
}

TEST(SocketBindTestSuite, bind_with_different_socket_type) {
    // Get local interface address.
    const CAddrinfo ai("", "8080", AF_INET, SOCK_DGRAM,
                       AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);

    // Test Unit. Binding with different SOCK_STREAM (not SOCK_DGRAM) will fail.
    EXPECT_THAT(
        [&ai]() {
            CSocket sockObj(AF_INET, SOCK_STREAM);
            sockObj.bind(ai);
        },
        ThrowsMessage<std::runtime_error>(
            StartsWith("UPnPlib ERROR 1033! Failed to bind socket to an "
                       "address: \"socket type of "
                       "address (")));
}

TEST(SocketBindTestSuite, bind_two_times_different_addresses_fail) {
    // Binding a socket two times isn't possible. The socket must be
    // shutdown/closed before bind it again.
    // Provide a socket object
    CSocket sockObj(AF_INET6, SOCK_STREAM);

    // Get local interface address. Socket type (SOCK_STREAM) must be the same
    // as that from the socket.
    CAddrinfo ai1("", "8080", AF_INET6, SOCK_STREAM,
                  AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);

    // Test Unit.
    ASSERT_NO_THROW(sockObj.bind(ai1));

    // Try to bind the socket a second time.
    EXPECT_THAT(
        ([&sockObj]() {
            CAddrinfo ai2("", "8081", AF_INET6, SOCK_STREAM,
                          AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);
            sockObj.bind(ai2);
        }),
        ThrowsMessage<std::runtime_error>(StartsWith(
            "UPnPlib ERROR 1008! Failed to bind socket to an address: ")));
}

#if 0 // Don't enable next test permanently!
      // It is very expensive and do not really test a Unit. It's only for
      // humans to show whats going on.
// On Microsoft Windows there is an issue with binding an address to a socket
// in conjunction with the socket option SO_EXCLUSIVEADDRUSE that we use for
// security reasons. Even on successful new opened socket file descriptors it
// may be possible that we cannot bind an unused socket address to it. For
// details of this have look at: [SO_EXCLUSIVEADDRUSE socket option]
// (https://learn.microsoft.com/en-us/windows/win32/winsock/so-exclusiveaddruse)
// The following test examins the situation. It try to bind all free user
// application socket numbers from 49152 to 65635 and shows with which port
// number it fails. It seems the problem exists only for this port range.
// Reusing port 8080 or 8081 works as expected.
#ifdef _MSC_VER
TEST(SocketBindTestSuite, check_binding_passive_all_free_ports) {
    // Binding a socket again is possible after destruction of the socket that
    // shutdown/close it. Get local interface address.
    in_port_t port{49152};
    CAddrinfo ai1("", std::to_string(port), AF_INET6, SOCK_STREAM,
                  AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);
    sockaddr_in6* sa_in6 = (sockaddr_in6*)ai1->ai_addr;

    std::cout << "DEBUG! start port = " << port << "\n";
    for (; port < 65535; port++) {
        CSocket sockObj(AF_INET6, SOCK_STREAM);
        try {
            sockObj.bind(ai1);
        } catch (const std::runtime_error& e) {
            std::cout << "DEBUG! port " << port << ": ";
            std::cout << e.what() << "\n";
        }
        sa_in6->sin6_port = htons(port);
    }

    std::cout << "DEBUG! finished port = " << port << "\n";
}
#endif // MSC_VER
#endif // if 0

#ifdef _MSC_VER
TEST(SocketBindTestSuite, set_unset_bind_win32_same_address_multiple_times) {
    // Binding a socket again is possible after destruction of the socket that
    // shutdown/close it. But we have to take attention about the issue on
    // win32 with the SO_EXCLUSIVEADDRUSE socket option. For details look at
    // the previous test.
    // Get local interface address.
    in_port_t port = portno(random);
    CAddrinfo ai1("", std::to_string(port), AF_INET6, SOCK_STREAM,
                  AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);
    sockaddr_in6* sa_in6 = (sockaddr_in6*)ai1->ai_addr;

    // Test Unit
    for (int i{0}; i < 2; i++) {
        CSocket sockObj(AF_INET6, SOCK_STREAM);
        try {
            sockObj.bind(ai1);
            // std::cout << "DEBUG! Success port " << port << "\n";
        } catch ([[maybe_unused]] const std::runtime_error& e) {
            // std::cout << "DEBUG! port " << port << ": " << e.what() << "\n";
            // Continue with a new port number.
            port = portno(random);
            sa_in6->sin6_port = htons(port);
            // std::this_thread::sleep_for(std::chrono::seconds(5)); // DEBUG!
        }
    }

    // std::cout << "DEBUG! Success port " << port << "\n";
    CSocket sockObj(AF_INET6, SOCK_STREAM);
    ASSERT_NO_THROW(sockObj.bind(ai1));
}

#else  // _MSC_VER

TEST(SocketBindTestSuite, set_unset_bind_unix_same_address_multiple_times) {
    // Binding a socket again is possible after destruction of the socket that
    // shutdown/close it.
    // Get local interface address.
    CAddrinfo ai1("", "8080", AF_INET6, SOCK_STREAM,
                  AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);

    // Test Unit
    {
        CSocket sockObj(AF_INET6, SOCK_STREAM);
        ASSERT_NO_THROW(sockObj.bind(ai1));
    }
    {
        CSocket sockObj(AF_INET6, SOCK_STREAM);
        ASSERT_NO_THROW(sockObj.bind(ai1));
    }
    CSocket sockObj(AF_INET6, SOCK_STREAM);
    ASSERT_NO_THROW(sockObj.bind(ai1));
}
#endif // _MSC_VER

TEST(SocketBindTestSuite, bind_same_address_two_times) {
    // SKIP on Github Actions
    if (github_actions)
        GTEST_SKIP() << "             known failing test on Github Actions";

    // Binding the same address again to a valid socket is possible and should
    // do nothing.
    // Get local interface address.
    CAddrinfo ai1("", "8080", AF_INET6, SOCK_STREAM,
                  AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);

    // Test Unit
    CSocket sockObj(AF_INET6, SOCK_STREAM);
    EXPECT_NO_THROW(sockObj.bind(ai1));

    EXPECT_EQ(sockObj.get_family(), AF_INET6);
    EXPECT_EQ(sockObj.get_type(), SOCK_STREAM);
    EXPECT_EQ(sockObj.get_addr_str(), "[::]");
    EXPECT_EQ(sockObj.get_port(), 8080);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    // v6only is always false from the operating system, no matter what domain
    // (AF_INET6) is set if we request a passive address information (flag
    // AI_PASSIVE).
    EXPECT_FALSE(sockObj.is_v6only());
    EXPECT_TRUE(sockObj.is_bound());
    EXPECT_FALSE(sockObj.is_listen());

    // Doing the same again doesn't hurt.
    EXPECT_NO_THROW(sockObj.bind(ai1));

    EXPECT_EQ(sockObj.get_family(), AF_INET6);
    EXPECT_EQ(sockObj.get_type(), SOCK_STREAM);
    EXPECT_EQ(sockObj.get_addr_str(), "[::]");
    EXPECT_EQ(sockObj.get_port(), 8080);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    // v6only is always false from the operating system, no matter what domain
    // (AF_INET6) is set if we request a passive address information (flag
    // AI_PASSIVE).
    EXPECT_FALSE(sockObj.is_v6only());
    EXPECT_TRUE(sockObj.is_bound());
    EXPECT_FALSE(sockObj.is_listen());
}

TEST(SocketBindTestSuite, listen_to_same_address_multiple_times) {
    // Listen on the same address again of a valid socket is possible and should
    // do nothing.

    // Get local interface address.
    CAddrinfo ai1("", "8080", AF_INET6, SOCK_STREAM,
                  AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);

    // Test Unit
    CSocket sockObj(AF_INET6, SOCK_STREAM);
    ASSERT_NO_THROW(sockObj.bind(ai1));
    ASSERT_NO_THROW(sockObj.listen());

    EXPECT_NO_THROW(sockObj.listen());
}

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
    // With AF_INET6 and SOCK_DGRAM the flag IP6_V6ONLY can be modified.

    // Get socket
    CSocket sockObj(AF_INET6, SOCK_DGRAM);
    EXPECT_FALSE(sockObj.is_v6only());

    // Set option IPV6_V6ONLY. This is always ignored.
    // Test Unit
    ASSERT_NO_THROW(sockObj.set_v6only(true));
    EXPECT_TRUE(sockObj.is_v6only());
}

TEST(SocketV6onlyTestSuite, modify_v6only_af_inet_stream) {
    // With AF_INET the flag IP6_V6ONLY can not be modified, no matter what type
    // is used (e.g. SOCK_STREAM).

    // Get socket
    CSocket sockObj(AF_INET, SOCK_STREAM);
    EXPECT_FALSE(sockObj.is_v6only());

    // Test Unit
    // Set option IPV6_V6ONLY. This is always ignored.
    ASSERT_NO_THROW(sockObj.set_v6only(true));
    EXPECT_FALSE(sockObj.is_v6only());
}

TEST(SocketV6onlyTestSuite, modify_v6only_af_inet_dgram) {
    // With AF_INET the flag IP6_V6ONLY can not be modified, no matter what type
    // is used (e.g. SOCK_DGRAM).

    // Get socket
    CSocket sockObj(AF_INET, SOCK_DGRAM);
    EXPECT_FALSE(sockObj.is_v6only());

    // Set option IPV6_V6ONLY. This is always ignored.
    // Test Unit
    ASSERT_NO_THROW(sockObj.set_v6only(true));
    EXPECT_FALSE(sockObj.is_v6only());
}

TEST(SocketV6onlyTestSuite, modify_v6only_on_bound_af_inet6_stream_socket) {
    // Get local interface address with service.
    const CAddrinfo ai("[::1]", "8080", AF_INET6, SOCK_STREAM,
                       AI_NUMERICHOST | AI_NUMERICSERV);

    // Create an unbound socket object
    CSocket sockObj(AF_INET6, SOCK_STREAM);
    EXPECT_FALSE(sockObj.is_v6only());

    // This binds the local address to the socket and always sets ipv6_v6only
    // because we bind to an AF_INET6 address/socket.
    ASSERT_NO_THROW(sockObj.bind(ai));
    EXPECT_TRUE(sockObj.is_v6only());

    // Test Unit
    EXPECT_NO_THROW(sockObj.set_v6only(false));

    // Nothing has changed, the socket is valid.
    EXPECT_EQ(sockObj.get_family(), AF_INET6);
    EXPECT_EQ(sockObj.get_type(), SOCK_STREAM);
    EXPECT_EQ(sockObj.get_addr_str(), "[::1]");
    EXPECT_EQ(sockObj.get_port(), 8080);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    // Not set to false because the socket is already bound to an AF_INET6.
    EXPECT_TRUE(sockObj.is_v6only());
    EXPECT_TRUE(sockObj.is_bound());
    EXPECT_FALSE(sockObj.is_listen());
}

TEST(SocketV6onlyTestSuite, modify_v6only_on_bound_af_inet6_dgram_socket) {
    // Get local interface address with service.
    const CAddrinfo ai("[::1]", "8080", AF_INET6, SOCK_DGRAM,
                       AI_NUMERICHOST | AI_NUMERICSERV);

    // Create an unbound socket object
    CSocket sockObj(AF_INET6, SOCK_DGRAM);
    EXPECT_FALSE(sockObj.is_v6only());

    // This binds the local address to the socket and always sets ipv6_v6only
    // because we bind to an AF_INET6 address/socket.
    ASSERT_NO_THROW(sockObj.bind(ai));
    EXPECT_TRUE(sockObj.is_v6only());

    // Test Unit
    EXPECT_NO_THROW(sockObj.set_v6only(false));

    // Nothing has changed, the socket is valid.
    EXPECT_EQ(sockObj.get_family(), AF_INET6);
    EXPECT_EQ(sockObj.get_type(), SOCK_DGRAM);
    EXPECT_EQ(sockObj.get_addr_str(), "[::1]");
    EXPECT_EQ(sockObj.get_port(), 8080);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    // Not set to false because the socket is already bound to an AF_INET6.
    EXPECT_TRUE(sockObj.is_v6only());
    EXPECT_TRUE(sockObj.is_bound());
    EXPECT_FALSE(sockObj.is_listen());
}

TEST(SocketV6onlyTestSuite, modify_v6only_on_bound_af_inet_stream_socket) {
    // Get local interface address with service.
    const CAddrinfo ai("127.0.0.1", "8080", AF_INET, SOCK_STREAM,
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
    EXPECT_EQ(sockObj.get_family(), AF_INET);
    EXPECT_EQ(sockObj.get_type(), SOCK_STREAM);
    EXPECT_EQ(sockObj.get_addr_str(), "127.0.0.1");
    EXPECT_EQ(sockObj.get_port(), 8080);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    // Never set to true because it is an AF_INET socket.
    EXPECT_FALSE(sockObj.is_v6only());
    EXPECT_TRUE(sockObj.is_bound());
    EXPECT_FALSE(sockObj.is_listen());
}

TEST(SocketV6onlyTestSuite, modify_v6only_on_bound_af_inet_dgram_socket) {
    // Get local interface address with service.
    const CAddrinfo ai("127.0.0.1", "8080", AF_INET, SOCK_DGRAM,
                       AI_NUMERICHOST | AI_NUMERICSERV);

    // Create an unbound socket object
    CSocket sockObj(AF_INET, SOCK_DGRAM);
    EXPECT_FALSE(sockObj.is_v6only());

    // This binds the local address to the socket and never sets ipv6_v6only
    // because we bind to an AF_INET address/socket.
    ASSERT_NO_THROW(sockObj.bind(ai));
    EXPECT_FALSE(sockObj.is_v6only());

    // Test Unit
    // This is silently ignored.
    EXPECT_NO_THROW(sockObj.set_v6only(true));

    // Nothing has changed, the socket is valid.
    EXPECT_EQ(sockObj.get_family(), AF_INET);
    EXPECT_EQ(sockObj.get_type(), SOCK_DGRAM);
    EXPECT_EQ(sockObj.get_addr_str(), "127.0.0.1");
    EXPECT_EQ(sockObj.get_port(), 8080);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    // Never set to true because it is an AF_INET socket.
    EXPECT_FALSE(sockObj.is_v6only());
    EXPECT_TRUE(sockObj.is_bound());
    EXPECT_FALSE(sockObj.is_listen());
}

TEST(SocketV6onlyTestSuite, unset_v6only_on_passive_af_inet6_stream_socket) {
    // Get local interface address with service.
    const CAddrinfo ai("", "8080", AF_INET6, SOCK_STREAM,
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
    EXPECT_EQ(sockObj.get_family(), AF_INET6);
    EXPECT_EQ(sockObj.get_type(), SOCK_STREAM);
    EXPECT_EQ(sockObj.get_addr_str(), "[::]");
    EXPECT_EQ(sockObj.get_port(), 8080);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    // Not set to true because the socket is already bound to an AF_INET6
    // address.
    EXPECT_FALSE(sockObj.is_v6only());
    EXPECT_TRUE(sockObj.is_bound());
    EXPECT_FALSE(sockObj.is_listen());
}

TEST(SocketV6onlyTestSuite, set_v6only_on_passive_af_inet6_stream_socket) {
    // Get local interface address with service.
    const CAddrinfo ai("", "8080", AF_INET6, SOCK_STREAM,
                       AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);

    // Create an unbound socket object
    CSocket sockObj(AF_INET6, SOCK_STREAM);
    EXPECT_FALSE(sockObj.is_v6only());

    EXPECT_NO_THROW(sockObj.set_v6only(true));
    EXPECT_TRUE(sockObj.is_v6only());

    // Test Unit
    // This binds the local address to the socket but does not modify
    // IPV6_V6ONLY because we bind to a passive AF_INET6 address/socket.
    ASSERT_NO_THROW(sockObj.bind(ai));
    EXPECT_TRUE(sockObj.is_v6only());

    // It cannot be modified afer binding.
    EXPECT_NO_THROW(sockObj.set_v6only(false));

    // Nothing has changed, the socket is valid.
    EXPECT_EQ(sockObj.get_family(), AF_INET6);
    EXPECT_EQ(sockObj.get_type(), SOCK_STREAM);
    EXPECT_EQ(sockObj.get_addr_str(), "[::]");
    EXPECT_EQ(sockObj.get_port(), 8080);
    EXPECT_EQ(sockObj.get_sockerr(), 0);
    EXPECT_FALSE(sockObj.is_reuse_addr());
    // Not set to false because the socket is already bound.
    EXPECT_TRUE(sockObj.is_v6only());
    EXPECT_TRUE(sockObj.is_bound());
    EXPECT_FALSE(sockObj.is_listen());
}

TEST(SocketV6onlyTestSuite, modify_v6only_on_passive_af_inet_dgram_socket) {
    // Get local interface address with service.
    const CAddrinfo ai("", "8080", AF_INET, SOCK_DGRAM,
                       AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);

    // Create an unbound socket object
    CSocket sockObj(AF_INET, SOCK_DGRAM);
    EXPECT_FALSE(sockObj.is_v6only());

    // v6only can never be set on an address family AF_INET.
    EXPECT_NO_THROW(sockObj.set_v6only(true));
    EXPECT_FALSE(sockObj.is_v6only());

    // Test Unit
    // This binds the local address to the socket and always resets ipv6_v6only
    // even with passive mode because we bind to an AF_INET address/socket.
    ASSERT_NO_THROW(sockObj.bind(ai));
    EXPECT_FALSE(sockObj.is_v6only());

    // It cannot be set afer binding.
    EXPECT_NO_THROW(sockObj.set_v6only(true));

    // Nothing has changed, the socket is valid.
    EXPECT_EQ(sockObj.get_family(), AF_INET);
    EXPECT_EQ(sockObj.get_type(), SOCK_DGRAM);
    EXPECT_EQ(sockObj.get_addr_str(), "0.0.0.0");
    EXPECT_EQ(sockObj.get_port(), 8080);
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
    const CAddrinfo ai("", "8080", AF_INET6, SOCK_STREAM,
                       AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);
    // Get a socket and bind it to the local address.
    CSocket sockObj(AF_INET6, SOCK_STREAM);
    ASSERT_NO_THROW(sockObj.bind(ai));

    // Test Unit
    EXPECT_EQ(sockObj.get_addr_str(), "[::]");
}

TEST(SocketTestSuite, get_addr_str_ipv4_successful) {
    // Get local interface address.
    const CAddrinfo ai("", "8080", AF_INET, SOCK_DGRAM,
                       AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);
    // Get a socket and bind it to the local address.
    CSocket sockObj(AF_INET, SOCK_DGRAM);
    ASSERT_NO_THROW(sockObj.bind(ai));

    // Test Unit
    EXPECT_EQ(sockObj.get_addr_str(), "0.0.0.0");
}

TEST(SocketTestSuite, get_addr_str_from_invalid_socket) {
    // Test Unit wit empty socket.
    EXPECT_THAT(
        []() {
            CSocket sockObj;
            sockObj.get_addr_str();
        },
        ThrowsMessage<std::runtime_error>(StartsWith("UPnPlib ERROR 1001!")));
}

TEST(SocketTestSuite, get_addr_str_from_unbound_socket) {
    // Get a valid socket but do not bind it to an address.
    CSocket sockObj(AF_INET6, SOCK_STREAM);
    EXPECT_EQ(sockObj.get_addr_str(), "[::]");
    EXPECT_EQ(sockObj.get_port(), 0);
}

TEST(SocketTestSuite, get_addr_str_syscall_fail) {
    // Get local interface address.
    const CAddrinfo ai("", "8080", AF_INET6, SOCK_STREAM,
                       AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);
    // Get a socket and bind it to the local address.
    CSocket sockObj(AF_INET6, SOCK_STREAM);
    ASSERT_NO_THROW(sockObj.bind(ai));

    // Mock system function getsockname().
    umock::Sys_socketMock mocked_sys_socketObj;
    umock::Sys_socket sys_socket_injectObj(&mocked_sys_socketObj);
    EXPECT_CALL(
        mocked_sys_socketObj,
        getsockname((SOCKET)sockObj, _, Pointee((int)sizeof(sockaddr_storage))))
        .WillOnce(SetErrnoAndReturn(ENOBUFS, SOCKET_ERROR));

    // Test Unit
    EXPECT_THAT(
        [&sockObj]() { sockObj.get_addr_str(); },
        ThrowsMessage<std::runtime_error>(StartsWith("UPnPlib ERROR 1001!")));
}

TEST(SocketTestSuite, get_addr_str_invalid_address_family) {
    // Get local interface address.
    const CAddrinfo ai("", "8080", AF_INET6, SOCK_STREAM,
                       AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);
    // Get a socket and bind it to the local address.
    CSocket sockObj(AF_INET6, SOCK_STREAM);
    ASSERT_NO_THROW(sockObj.bind(ai));

    // Provide invalid address family.
    ::sockaddr_storage ss{};
    ss.ss_family = (sa_family_t)255;

    // Mock system function
    umock::Sys_socketMock mocked_sys_socketObj;
    umock::Sys_socket sys_socket_injectObj(&mocked_sys_socketObj);
    EXPECT_CALL(mocked_sys_socketObj,
                getsockname((SOCKET)sockObj, _,
                            Pointee((int)sizeof(::sockaddr_storage))))
        .WillOnce(DoAll(SetArgPointee<1>(*(sockaddr*)&ss), Return(0)));

    // Test Unit
    EXPECT_THAT(
        [&sockObj]() { sockObj.get_addr_str(); },
        ThrowsMessage<std::runtime_error>(StartsWith("UPnPlib ERROR 1024!")));
}

} // namespace upnplib


int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include <upnplib/gtest_main.inc>
    return gtest_return_code; // managed in upnplib/gtest_main.inc
}
