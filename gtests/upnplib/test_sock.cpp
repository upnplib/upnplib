// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-03-08

#include "upnplib/port.hpp"
#include "upnplib/sock.hpp"
#include "upnplib/gtest.hpp"

#include "gmock/gmock.h"
#include "umock/sys_socket_mock.hpp"

using ::testing::_;
using ::testing::DoAll;
using ::testing::Pointee;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::ThrowsMessage;

using ::upnplib::testing::ContainsStdRegex;
using ::upnplib::testing::MatchesStdRegex;

namespace upnplib {
bool old_code{false}; // Managed in upnplib_gtest_main.inc
bool github_actions = std::getenv("GITHUB_ACTIONS");

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

TEST(SocketAddrTestSuite, get_address_from_socket) {
    SOCKET sockfd{1102};

    // Provide a sockaddr structure that will be returned by mocked
    // getsockname().
    struct SockAddr ret_sock;
    ret_sock.addr_set("192.168.111.222", 54444);

    // Mock system functions
    umock::Sys_socketMock mocked_sys_socketObj;
    umock::Sys_socket sys_socket_injectObj(&mocked_sys_socketObj);
    EXPECT_CALL(mocked_sys_socketObj, getsockname(sockfd, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(*ret_sock.addr), Return(0)));

    // Test Unit
    struct SocketAddr sock;
    EXPECT_EQ(sock.addr_get(sockfd), "192.168.111.222");
    EXPECT_EQ(sock.addr_get_port(), 54444);
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
    sock.addr_ss.ss_family = (sa_family_t)-1;

    // Test Unit
    EXPECT_THAT([&sock] { sock.addr_get(); },
                ThrowsMessage<std::invalid_argument>(
                    MatchesStdRegex("UPnPlib ERR\\. at \\*/.+\\.cpp\\[\\d+\\], "
                                    ".*addr_get\\(\\), errid=\\d+: .+")));
}

TEST(SocketAddrTestSuite, get_address_from_invalid_socket) {
    SOCKET sockfd{1101};
    struct SocketAddr sock;

    // Test Unit
    EXPECT_THAT(([&sock, sockfd] { sock.addr_get(sockfd); }),
                ThrowsMessage<std::runtime_error>(
                    MatchesStdRegex("UPnPlib ERR\\. at \\*/.+\\.cpp\\[\\d+\\], "
                                    ".*addr_get\\(1101\\), errid=\\d+: .*")));
}

} // namespace upnplib

//
int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "upnplib/gtest_main.inc"
    return gtest_return_code; // managed in upnplib/gtest_main.inc
}
