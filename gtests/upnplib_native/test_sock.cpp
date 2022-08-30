// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-08-31

#include "upnplib/sock.hpp"

#include "upnplib/gtest.hpp"
#include "gmock/gmock.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::Pointee;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::ThrowsMessage;

using ::upnplib::testing::ContainsStdRegex;

namespace upnplib {
bool old_code{false}; // Managed in upnplib_gtest_main.inc
bool github_actions = std::getenv("GITHUB_ACTIONS");

//
// Mocked system calls
// ===================
class MockSysSocket : public ISysSocket {
  public:
    MOCK_METHOD(int, getsockname,
                (int sockfd, struct sockaddr* addr, socklen_t* addrlen),
                (override));
};

//
// struct SockAddr and struct SocketAddr TestSuite
// ===============================================
// With testing struct SocketAddr this tests also its base struct SockAddr.
TEST(SocketAddrTestSuite, successful_execution) {
    struct SocketAddr sock;
    EXPECT_EQ(sock.addr_in->sin_family, AF_INET);

    EXPECT_EQ(sock.addr_get(), "0.0.0.0");
    EXPECT_EQ(sock.addr_get_port(), 0);

    sock.addr_set("192.168.169.170", 4711);
    EXPECT_EQ(sock.addr_get(), "192.168.169.170");
    EXPECT_EQ(sock.addr_get_port(), 4711);

    char buf_ntop[INET6_ADDRSTRLEN]{};
    EXPECT_NE(
        inet_ntop(AF_INET, &sock.addr_in->sin_addr, buf_ntop, sizeof(buf_ntop)),
        nullptr);
    EXPECT_STREQ(buf_ntop, "192.168.169.170");
    EXPECT_EQ(ntohs(sock.addr_in->sin_port), 4711);
}

TEST(SocketAddrTestSuite, set_wrong_address) {
    struct SocketAddr sock;

    // Test Unit
    EXPECT_THAT([&sock] { sock.addr_set("192.168.169.999", 4711); },
                ThrowsMessage<std::invalid_argument>(
                    ContainsStdRegex("at \\*/.+\\[\\d+\\]: Invalid ip "
                                     "address '192\\.168\\.169\\.999'")));
}

TEST(SocketAddrTestSuite, get_wrong_address) {
    struct SocketAddr sock;

    // This is a wrong family entry that should trigger an error on getting an
    // ip address
    sock.addr_ss.ss_family = -1;

    // Test Unit
    EXPECT_THAT([&sock] { sock.addr_get(); },
                ThrowsMessage<std::runtime_error>(ContainsStdRegex(
                    "at \\*/.+\\[\\d+\\]: Got invalid ip address")));
}

TEST(SocketAddrTestSuite, get_address_from_invalid_socket) {
    SOCKET sockfd{1101};
    struct SocketAddr sock;

    // Test Unit
    EXPECT_THAT(([&sock, sockfd] { sock.addr_get(sockfd); }),
                ThrowsMessage<std::runtime_error>(ContainsStdRegex(
                    "at \\*/.+\\[\\d+\\]: Got invalid ip address from fd " +
                    std::to_string(sockfd) + "\\. .+")));
}

TEST(SocketAddrTestSuite, get_address_from_mocked_socket) {
    // First create a socket address structure that will be returned by the
    // mocked getsockname() system call:
    struct SocketAddr ret_sock;
    ret_sock.addr_set("192.168.5.6", 59876);

    // Now fill a socket structure with the address from a (mocked) socket
    // query.
    SOCKET sockfd{1102};
    class MockSysSocket mocked_sys_socketObj;
    // Inject mocked sys_socket object
    struct SocketAddr sock(&mocked_sys_socketObj);

    // This is the expectation that returns the address from the socket
    // (provided with ret_sock above).
    EXPECT_CALL(mocked_sys_socketObj,
                getsockname(sockfd, _, Pointee(sizeof(ret_sock.addr_ss))))
        .WillOnce(DoAll(SetArgPointee<1>(*ret_sock.addr), Return(0)));

    // Test Unit
    EXPECT_EQ(sock.addr_get(sockfd), "192.168.5.6");
    EXPECT_EQ(sock.addr_get_port(), 59876);

    char buf_ntop[INET6_ADDRSTRLEN]{};
    EXPECT_NE(
        inet_ntop(AF_INET, &sock.addr_in->sin_addr, buf_ntop, sizeof(buf_ntop)),
        nullptr);
    EXPECT_STREQ(buf_ntop, "192.168.5.6");
    EXPECT_EQ(ntohs(sock.addr_in->sin_port), 59876);
}

} // namespace upnplib

//
int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "upnplib/gtest_main.inc"
}
