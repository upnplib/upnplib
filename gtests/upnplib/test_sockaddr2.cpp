// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-06-04

#include <upnplib/sockaddr2.hpp>
#include <upnplib/socket.hpp>
#include <gmock/gmock.h>


namespace upnplib {
bool old_code{false}; // Managed in upnplib_gtest_main.inc

using ::testing::ThrowsMessage;

TEST(SsockaddrStorageTestSuite, point_to_sockaddr_storage) {
    Ssockaddr_storage ss;
    ss.ss_family = (sa_family_t)0xA5A5;
    // Get pointer from reference
    sockaddr_storage* pss = &ss;
    sockaddr_in6* psin6 = (sockaddr_in6*)pss;
    EXPECT_EQ(psin6->sin6_family, (sa_family_t)0xA5A5);
    EXPECT_EQ(((sockaddr_in6*)(sockaddr_storage*)&ss)->sin6_family,
              (sa_family_t)0xA5A5);
    // Next doesn't point to the address family if we have a virtual destructor.
    // EXPECT_NE(((sockaddr_in6*)&ss)->sin6_family, 0xA5A5);
}

TEST(SsockaddrStorageTestSuite, check_addressing_structure) {
    Ssockaddr_storage ss;
    ss.ss_family = (sa_family_t)AF_INET6;
    // Get pointer from reference
    sockaddr_storage* pss = &ss;
    // std::cout << "DEBUG pss = '" << pss << "'\n";
    sockaddr_in6* psin6 = (sockaddr_in6*)pss;
    EXPECT_EQ(psin6->sin6_family, (sa_family_t)AF_INET6);
    EXPECT_EQ(ss.get_addr_str(), "[::]");
}

TEST(SockaddrStorageTestSuite, set_address_and_port_successful) {
    Ssockaddr_storage saddr;

    EXPECT_EQ(saddr.ss_family, AF_UNSPEC);
    EXPECT_EQ(saddr.get_addr_str(), "");
    EXPECT_EQ(saddr.get_port(), 0);

    saddr.ss_family = AF_INET6;
    EXPECT_EQ(saddr.ss_family, AF_INET6);
    EXPECT_EQ(saddr.get_addr_str(), "[::]");
    EXPECT_EQ(saddr.get_port(), 0);

    saddr.ss_family = AF_INET;
    EXPECT_EQ(saddr.get_addr_str(), "0.0.0.0");
    EXPECT_EQ(saddr.get_port(), 0);

    saddr = "";
    EXPECT_EQ(saddr.ss_family, AF_UNSPEC);
    EXPECT_EQ(saddr.get_addr_str(), "");
    EXPECT_EQ(saddr.get_port(), 0);

    saddr = "[2001:db8::1]";
    EXPECT_EQ(saddr.ss_family, AF_INET6);
    EXPECT_EQ(saddr.get_addr_str(), "[2001:db8::1]");
    EXPECT_EQ(saddr.get_port(), 0);

    saddr.ss_family = AF_UNSPEC;
    saddr = "[2001:db8::2]:50020";
    EXPECT_EQ(saddr.ss_family, AF_INET6);
    EXPECT_EQ(saddr.get_addr_str(), "[2001:db8::2]");
    EXPECT_EQ(saddr.get_port(), 50020);

    saddr.ss_family = AF_INET;
    saddr = "[2001:db8::3]:";
    EXPECT_EQ(saddr.ss_family, AF_INET6);
    EXPECT_EQ(saddr.get_addr_str(), "[2001:db8::3]");
    EXPECT_EQ(saddr.get_port(), 0);

    saddr = "50021";
    EXPECT_EQ(saddr.ss_family, AF_INET6);
    EXPECT_EQ(saddr.get_addr_str(), "[2001:db8::3]");
    EXPECT_EQ(saddr.get_port(), 50021);

    saddr = "192.168.77.88:";
    EXPECT_EQ(saddr.ss_family, AF_INET);
    EXPECT_EQ(saddr.get_addr_str(), "192.168.77.88");
    EXPECT_EQ(saddr.get_port(), 0);

    saddr.ss_family = AF_UNSPEC;
    saddr = "192.168.47.49:50022";
    EXPECT_EQ(saddr.ss_family, AF_INET);
    EXPECT_EQ(saddr.get_addr_str(), "192.168.47.49");
    EXPECT_EQ(saddr.get_port(), 50022);

    saddr = "192.168.47.48";
    EXPECT_EQ(saddr.ss_family, AF_INET);
    EXPECT_EQ(saddr.get_addr_str(), "192.168.47.48");
    EXPECT_EQ(saddr.get_port(), 50022);

    // Check that a failing call does not modify old settings.
    EXPECT_THAT([&saddr]() { saddr = "50x23"; },
                ThrowsMessage<std::invalid_argument>(
                    "ERROR! Failed to get port number for \"50x23\""));
    EXPECT_EQ(saddr.ss_family, AF_INET);
    EXPECT_EQ(saddr.get_addr_str(), "192.168.47.48");
    EXPECT_EQ(saddr.get_port(), 50022);
}

TEST(SockaddrStorageTestSuite, fill_structure_from_function_output) {
    WINSOCK_INIT_P

    SOCKET sfd = socket(AF_INET6, SOCK_STREAM, 0);
    ASSERT_NE(sfd, SOCKET_ERROR);
    Ssockaddr_storage ss;
    // ::sockaddr_storage ss;
    socklen_t sslen = sizeof(::sockaddr_storage);

    ASSERT_EQ(::getsockname(sfd, (sockaddr*)(sockaddr_storage*)&ss, &sslen), 0);
    EXPECT_EQ(ss.ss_family, AF_INET6);
}

TEST(ToPortTestSuite, str_to_port) {
    EXPECT_EQ(rework::to_port("123"), 123);
    EXPECT_EQ(rework::to_port("00456"), 456);
    EXPECT_EQ(rework::to_port("65535"), 65535);
    EXPECT_EQ(rework::to_port(""), 0);
    EXPECT_EQ(rework::to_port("0"), 0);
    EXPECT_EQ(rework::to_port("9"), 9);
    EXPECT_EQ(rework::to_port("00000"), 0);

    EXPECT_THAT([]() { rework::to_port("000000"); },
                ThrowsMessage<std::invalid_argument>(
                    "ERROR! Failed to get port number for \"000000\""));

    EXPECT_THAT([]() { rework::to_port("65536"); },
                ThrowsMessage<std::invalid_argument>(
                    "ERROR! Failed to get port number for \"65536\""));

    EXPECT_THAT([]() { rework::to_port("-1"); },
                ThrowsMessage<std::invalid_argument>(
                    "ERROR! Failed to get port number for \"-1\""));

    EXPECT_THAT([]() { rework::to_port("123456"); },
                ThrowsMessage<std::invalid_argument>(
                    "ERROR! Failed to get port number for \"123456\""));

    EXPECT_THAT([]() { rework::to_port(" "); },
                ThrowsMessage<std::invalid_argument>(
                    "ERROR! Failed to get port number for \" \""));

    EXPECT_THAT([]() { rework::to_port(" 123"); },
                ThrowsMessage<std::invalid_argument>(
                    "ERROR! Failed to get port number for \" 123\""));

    EXPECT_THAT([]() { rework::to_port("123 "); },
                ThrowsMessage<std::invalid_argument>(
                    "ERROR! Failed to get port number for \"123 \""));

    EXPECT_THAT([]() { rework::to_port("123.4"); },
                ThrowsMessage<std::invalid_argument>(
                    "ERROR! Failed to get port number for \"123.4\""));

    EXPECT_THAT([]() { rework::to_port(":1234"); },
                ThrowsMessage<std::invalid_argument>(
                    "ERROR! Failed to get port number for \":1234\""));

    EXPECT_THAT([]() { rework::to_port("12x34"); },
                ThrowsMessage<std::invalid_argument>(
                    "ERROR! Failed to get port number for \"12x34\""));
}

TEST(ToAddrStrTestSuite, sockaddr_to_address_string) {
    Ssockaddr_storage saddr;

    EXPECT_EQ(rework::to_addr_str(&saddr), "");

    saddr.ss_family = AF_INET6;
    EXPECT_EQ(rework::to_addr_str(&saddr), "[::]");

    saddr.ss_family = AF_INET;
    EXPECT_EQ(rework::to_addr_str(&saddr), "0.0.0.0");

    saddr = "[2001:db8::4]";
    EXPECT_EQ(rework::to_addr_str(&saddr), "[2001:db8::4]");

    saddr = "192.168.88.99";
    EXPECT_EQ(rework::to_addr_str(&saddr), "192.168.88.99");

    saddr.ss_family = AF_UNSPEC;
    EXPECT_EQ(rework::to_addr_str(&saddr), "");

    saddr.ss_family = AF_UNIX;
    EXPECT_THAT([&saddr]() { rework::to_addr_str(&saddr); },
                ThrowsMessage<std::invalid_argument>(
                    "ERROR! Unsupported address family 1"));
}

} // namespace upnplib


int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "upnplib/gtest_main.inc"
    return gtest_return_code; // managed in upnplib/gtest_main.inc
}
