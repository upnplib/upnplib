// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-05-04

#include <upnplib/sockaddr.hpp>
#include <upnplib/port.hpp>
#include <upnplib/gtest.hpp>

#include <umock/sys_socket_mock.hpp>


namespace upnplib {
bool old_code{false}; // Managed in upnplib_gtest_main.inc
bool github_actions = std::getenv("GITHUB_ACTIONS");

using ::testing::_;
using ::testing::DoAll;
using ::testing::EndsWith;
using ::testing::Pointee;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::ThrowsMessage;

using ::upnplib::testing::ContainsStdRegex;
using ::upnplib::testing::MatchesStdRegex;


// SSockaddr_storage TestSuite
// ===========================
TEST(SockaddrStorageTestSuite, copy_and_assign_structure) {
    SSockaddr_storage ss1;
    ss1 = "[2001:db8::1]:50001";

    // Test Unit copy
    SSockaddr_storage ss2 = ss1;
    EXPECT_EQ(ss2.ss.ss_family, AF_INET6);
    EXPECT_EQ(ss2.get_addr_str(), "[2001:db8::1]");
    EXPECT_EQ(ss2.get_port(), 50001);

    // Test Unit assign
    ss1 = "192.168.251.252:50002";
    SSockaddr_storage ss3;
    ss3 = ss1;
    EXPECT_EQ(ss3.ss.ss_family, AF_INET);
    EXPECT_EQ(ss3.get_addr_str(), "192.168.251.252");
    EXPECT_EQ(ss3.get_port(), 50002);
}

TEST(SockaddrStorageTestSuite, set_address_and_port_successful) {
    SSockaddr_storage saddr;

    EXPECT_EQ(saddr.ss.ss_family, AF_UNSPEC);
    EXPECT_EQ(saddr.get_addr_str(), "");
    EXPECT_EQ(saddr.get_port(), 0);

    saddr.ss.ss_family = AF_INET6;
    EXPECT_EQ(saddr.get_addr_str(), "[::]");
    EXPECT_EQ(saddr.get_port(), 0);

    saddr.ss.ss_family = AF_INET;
    EXPECT_EQ(saddr.get_addr_str(), "0.0.0.0");
    EXPECT_EQ(saddr.get_port(), 0);

    saddr = "";
    EXPECT_EQ(saddr.ss.ss_family, AF_UNSPEC);
    EXPECT_EQ(saddr.get_addr_str(), "");
    EXPECT_EQ(saddr.get_port(), 0);

    saddr = "[2001:db8::1]";
    EXPECT_EQ(saddr.ss.ss_family, AF_INET6);
    EXPECT_EQ(saddr.get_addr_str(), "[2001:db8::1]");
    EXPECT_EQ(saddr.get_port(), 0);

    saddr.ss.ss_family = AF_UNSPEC;
    saddr = "[2001:db8::2]:50020";
    EXPECT_EQ(saddr.ss.ss_family, AF_INET6);
    EXPECT_EQ(saddr.get_addr_str(), "[2001:db8::2]");
    EXPECT_EQ(saddr.get_port(), 50020);

    saddr.ss.ss_family = AF_INET;
    saddr = "[2001:db8::3]:";
    EXPECT_EQ(saddr.ss.ss_family, AF_INET6);
    EXPECT_EQ(saddr.get_addr_str(), "[2001:db8::3]");
    EXPECT_EQ(saddr.get_port(), 0);

    saddr = "50021";
    EXPECT_EQ(saddr.ss.ss_family, AF_INET6);
    EXPECT_EQ(saddr.get_addr_str(), "[2001:db8::3]");
    EXPECT_EQ(saddr.get_port(), 50021);

    saddr = "192.168.77.88:";
    EXPECT_EQ(saddr.ss.ss_family, AF_INET);
    EXPECT_EQ(saddr.get_addr_str(), "192.168.77.88");
    EXPECT_EQ(saddr.get_port(), 0);

    saddr.ss.ss_family = AF_UNSPEC;
    saddr = "192.168.47.49:50022";
    EXPECT_EQ(saddr.ss.ss_family, AF_INET);
    EXPECT_EQ(saddr.get_addr_str(), "192.168.47.49");
    EXPECT_EQ(saddr.get_port(), 50022);

    saddr = "192.168.47.48";
    EXPECT_EQ(saddr.ss.ss_family, AF_INET);
    EXPECT_EQ(saddr.get_addr_str(), "192.168.47.48");
    EXPECT_EQ(saddr.get_port(), 50022);

    // Check that a failing call does not modify old settings.
    EXPECT_THAT([&saddr]() { saddr = "50x23"; },
                ThrowsMessage<std::invalid_argument>(
                    "ERROR! Failed to get port number for \"50x23\""));
    EXPECT_EQ(saddr.ss.ss_family, AF_INET);
    EXPECT_EQ(saddr.get_addr_str(), "192.168.47.48");
    EXPECT_EQ(saddr.get_port(), 50022);
}

TEST(SockaddrStorageTestSuite, set_address_and_port_fail) {
    SSockaddr_storage saddr;

    EXPECT_THAT([&saddr]() { saddr = "[2001::db8::1]"; },
                ThrowsMessage<std::invalid_argument>(
                    EndsWith("Invalid ip address '[2001::db8::1]'")));

    EXPECT_THAT([&saddr]() { saddr = "2001:db8::1]"; },
                ThrowsMessage<std::invalid_argument>(
                    EndsWith("Invalid ip address '2001'")));

    EXPECT_THROW({ saddr = "[2001:db8::2"; }, std::out_of_range);

    EXPECT_THROW({ saddr = "[2001:db8::3]50003"; }, std::out_of_range);

    EXPECT_THAT([&saddr]() { saddr = "[2001:db8::35003]"; },
                ThrowsMessage<std::invalid_argument>(
                    EndsWith("Invalid ip address '[2001:db8::35003]'")));

    EXPECT_THAT([&saddr]() { saddr = "192.168.66.67."; },
                ThrowsMessage<std::invalid_argument>(
                    EndsWith("Invalid ip address '192.168.66.67.'")));

    EXPECT_THAT([&saddr]() { saddr = "192.168.66.67z"; },
                ThrowsMessage<std::invalid_argument>(
                    EndsWith("Invalid ip address '192.168.66.67z'")));
}

TEST(SockaddrStorageTestSuite, compare_ipv6_address) {
    SSockaddr_storage saddr1;
    saddr1 = "[2001:db8::27]:50027";
    SSockaddr_storage saddr2;
    saddr2 = "[2001:db8::27]:50027";

    // Show how to address and compare ipv6 address. It is stored with
    // unsigned char s6_addr[16]; so we have to use memcmp().
    const unsigned char* const s6_addr1 =
        ((sockaddr_in6*)&saddr1.ss)->sin6_addr.s6_addr;
    const unsigned char* const s6_addr2 =
        ((sockaddr_in6*)&saddr2.ss)->sin6_addr.s6_addr;
    EXPECT_EQ(memcmp(s6_addr1, s6_addr2, sizeof(*s6_addr1)), 0);

    // Test Unit compare successful
    EXPECT_TRUE(saddr1 == saddr2.ss);

    // Test Unit different address family
    saddr2 = "192.168.0.27:50027";
    EXPECT_FALSE(saddr1 == saddr2.ss);

    // Test Unit different address
    saddr2 = "[2001:db8::28]:50027";
    EXPECT_FALSE(saddr1 == saddr2.ss);

    // Test Unit different port
    saddr2 = "[2001:db8::27]:50028";
    EXPECT_FALSE(saddr1 == saddr2.ss);

    // Test Unit with unsupported address family
    saddr2 = "[2001:db8::27]:50027";
    saddr1.ss.ss_family = AF_UNIX;
    EXPECT_FALSE(saddr1 == saddr2.ss);
    saddr1.ss.ss_family = AF_INET6;
    saddr2.ss.ss_family = AF_UNIX;
    EXPECT_FALSE(saddr1 == saddr2.ss);
    saddr2.ss.ss_family = AF_INET6;
    EXPECT_TRUE(saddr1 == saddr2.ss);
}

TEST(SockaddrStorageTestSuite, compare_ipv4_address) {
    SSockaddr_storage saddr1;
    saddr1 = "192.168.0.1:50029";
    SSockaddr_storage saddr2;
    saddr2 = "192.168.0.1:50029";

    // Test Unit compare successful
    EXPECT_TRUE(saddr1 == saddr2.ss);

    // Test Unit different address family
    saddr2 = "[2001:db8::27]:50027";
    EXPECT_FALSE(saddr1 == saddr2.ss);

    // Test Unit different address
    saddr2 = "192.168.0.2:50029";
    EXPECT_FALSE(saddr1 == saddr2.ss);

    // Test Unit different port
    saddr2 = "192.168.0.1:50030";
    EXPECT_FALSE(saddr1 == saddr2.ss);

    // Test Unit with unsupported address family
    saddr2 = "192.168.0.1:50029";
    saddr1.ss.ss_family = AF_UNIX;
    EXPECT_FALSE(saddr1 == saddr2.ss);
    saddr1.ss.ss_family = AF_INET;
    saddr2.ss.ss_family = AF_UNIX;
    EXPECT_FALSE(saddr1 == saddr2.ss);
    saddr2.ss.ss_family = AF_INET;
    EXPECT_TRUE(saddr1 == saddr2.ss);
}

TEST(ToPortTestSuite, str_to_port) {
    EXPECT_EQ(to_port("123"), 123);
    EXPECT_EQ(to_port("00456"), 456);
    EXPECT_EQ(to_port("65535"), 65535);
    EXPECT_EQ(to_port(""), 0);
    EXPECT_EQ(to_port("0"), 0);
    EXPECT_EQ(to_port("9"), 9);
    EXPECT_EQ(to_port("00000"), 0);

    EXPECT_THAT([]() { to_port("000000"); },
                ThrowsMessage<std::invalid_argument>(
                    "ERROR! Failed to get port number for \"000000\""));

    EXPECT_THAT([]() { to_port("65536"); },
                ThrowsMessage<std::invalid_argument>(
                    "ERROR! Failed to get port number for \"65536\""));

    EXPECT_THAT([]() { to_port("-1"); },
                ThrowsMessage<std::invalid_argument>(
                    "ERROR! Failed to get port number for \"-1\""));

    EXPECT_THAT([]() { to_port("123456"); },
                ThrowsMessage<std::invalid_argument>(
                    "ERROR! Failed to get port number for \"123456\""));

    EXPECT_THAT([]() { to_port(" "); },
                ThrowsMessage<std::invalid_argument>(
                    "ERROR! Failed to get port number for \" \""));

    EXPECT_THAT([]() { to_port(" 123"); },
                ThrowsMessage<std::invalid_argument>(
                    "ERROR! Failed to get port number for \" 123\""));

    EXPECT_THAT([]() { to_port("123 "); },
                ThrowsMessage<std::invalid_argument>(
                    "ERROR! Failed to get port number for \"123 \""));

    EXPECT_THAT([]() { to_port("123.4"); },
                ThrowsMessage<std::invalid_argument>(
                    "ERROR! Failed to get port number for \"123.4\""));

    EXPECT_THAT([]() { to_port(":1234"); },
                ThrowsMessage<std::invalid_argument>(
                    "ERROR! Failed to get port number for \":1234\""));

    EXPECT_THAT([]() { to_port("12x34"); },
                ThrowsMessage<std::invalid_argument>(
                    "ERROR! Failed to get port number for \"12x34\""));
}

TEST(ToAddrStrTestSuite, sockaddr_to_address_string) {
    SSockaddr_storage saddr;

    EXPECT_EQ(to_addr_str(&saddr.ss), "");

    saddr.ss.ss_family = AF_INET6;
    EXPECT_EQ(to_addr_str(&saddr.ss), "[::]");

    saddr.ss.ss_family = AF_INET;
    EXPECT_EQ(to_addr_str(&saddr.ss), "0.0.0.0");

    saddr = "[2001:db8::4]";
    EXPECT_EQ(to_addr_str(&saddr.ss), "[2001:db8::4]");

    saddr = "192.168.88.99";
    EXPECT_EQ(to_addr_str(&saddr.ss), "192.168.88.99");

    saddr.ss.ss_family = AF_UNSPEC;
    EXPECT_EQ(to_addr_str(&saddr.ss), "");

    saddr.ss.ss_family = AF_UNIX;
    EXPECT_THAT([&saddr]() { to_addr_str(&saddr.ss); },
                ThrowsMessage<std::invalid_argument>(
                    "ERROR! Unsupported address family 1"));
}


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

#if 0
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
#endif

} // namespace upnplib

//
int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "upnplib/gtest_main.inc"
    return gtest_return_code; // managed in upnplib/gtest_main.inc
}
