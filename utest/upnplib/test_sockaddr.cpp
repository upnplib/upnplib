// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-11-02

#include <upnplib/general.hpp>
#include <upnplib/sockaddr.hpp>
#include <upnplib/socket.hpp>

#include <utest/utest.hpp>

namespace utest {

using ::testing::EndsWith;
using ::testing::HasSubstr;
using ::testing::ThrowsMessage;

using ::upnplib::CSocket;
using ::upnplib::sockaddrcmp;
using ::upnplib::SSockaddr_storage;
using ::upnplib::to_addr_str;
using ::upnplib::to_addrport_str;
using ::upnplib::to_port;


// SSockaddr_storage TestSuite
// ===========================
TEST(SockaddrCmpTestSuite, compare_equal_ipv6_sockaddrs_successful) {
    SSockaddr_storage saddr1;
    saddr1 = "[2001:db8::33]:50033";
    SSockaddr_storage saddr2;
    saddr2 = "[2001:db8::33]:50033";

    // Test Unit
    EXPECT_TRUE(sockaddrcmp(&saddr1.ss, &saddr2.ss));
}

TEST(SockaddrCmpTestSuite, compare_equal_ipv4_sockaddrs_successful) {
    SSockaddr_storage saddr1;
    saddr1 = "192.168.167.166:50037";
    SSockaddr_storage saddr2;
    saddr2 = "192.168.167.166:50037";

    // Test Unit
    EXPECT_TRUE(sockaddrcmp(&saddr1.ss, &saddr2.ss));
}

TEST(SockaddrCmpTestSuite, compare_different_ipv6_sockaddrs) {
    // Test Unit, compare with different ports
    SSockaddr_storage saddr1;
    saddr1 = "[2001:db8::35]:50035";
    SSockaddr_storage saddr2;
    saddr2 = "[2001:db8::35]:50036";
    EXPECT_FALSE(sockaddrcmp(&saddr1.ss, &saddr2.ss));

    // Test Unit, compare with different address
    saddr2 = "[2001:db8::36]:50035";
    EXPECT_FALSE(sockaddrcmp(&saddr1.ss, &saddr2.ss));

    // Test Unit, compare with different address family
    saddr2 = "192.168.171.172:50035";
    EXPECT_FALSE(sockaddrcmp(&saddr1.ss, &saddr2.ss));
}

TEST(SockaddrCmpTestSuite, compare_different_ipv4_sockaddrs) {
    // Test Unit, compare with different ports
    SSockaddr_storage saddr1;
    saddr1 = "192.168.13.14:50038";
    SSockaddr_storage saddr2;
    saddr2 = "192.168.13.14:50039";
    EXPECT_FALSE(sockaddrcmp(&saddr1.ss, &saddr2.ss));

    // Test Unit, compare with different address
    saddr2 = "192.168.13.15:50038";
    EXPECT_FALSE(sockaddrcmp(&saddr1.ss, &saddr2.ss));
}

TEST(SockaddrCmpTestSuite, compare_unspecified_sockaddrs) {
    // Test Unit with unspecified address family will always return true if
    // equal, independent of address and port.
    SSockaddr_storage saddr1;
    saddr1.ss.ss_family = AF_UNSPEC;
    SSockaddr_storage saddr2;
    saddr2.ss.ss_family = AF_UNSPEC;
    EXPECT_TRUE(sockaddrcmp(&saddr1.ss, &saddr2.ss));

    saddr2 = "[2001:db8::40]:50040";
    EXPECT_FALSE(sockaddrcmp(&saddr1.ss, &saddr2.ss));
    EXPECT_FALSE(sockaddrcmp(&saddr2.ss, &saddr1.ss));
}

TEST(SockaddrCmpTestSuite, compare_nullptr_to_sockaddrs) {
    SSockaddr_storage saddr1;
    saddr1 = "[2001:db8::34]:50034";

    // Test Unit
    EXPECT_TRUE(sockaddrcmp(nullptr, nullptr));
    EXPECT_FALSE(sockaddrcmp(nullptr, &saddr1.ss));
    EXPECT_FALSE(sockaddrcmp(&saddr1.ss, nullptr));
}

TEST(SockaddrStorageTestSuite, copy_and_assign_structure) {
    SSockaddr_storage ss1;
    ss1 = "[2001:db8::1]:50001";

    // Test Unit copy
    // With UPNPLIB_WITH_TRACE we do not see "Construct ..." because the
    // default copy constructor is used here.
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
                    HasSubstr("] EXCEPTION MSG1033: ")));
    EXPECT_EQ(saddr.ss.ss_family, AF_INET);
    EXPECT_EQ(saddr.get_addr_str(), "192.168.47.48");
    EXPECT_EQ(saddr.get_port(), 50022);
}

TEST(SockaddrStorageTestSuite, fill_structure_from_function_output) {
    // Get local interface address with service.
    // const CAddrinfo ai("[::1]", "50046", AF_INET6, SOCK_STREAM,
    //                    AI_NUMERICHOST | AI_NUMERICSERV);

    // Create an unbound socket object
    CSocket sockObj(AF_INET6, SOCK_STREAM);

    // Bind the local address to the socket.
    // ASSERT_NO_THROW(sockObj.bind(ai));

    // Get a sockaddr storage structure
    // SSockaddr_storage ssObj;
    // socklen_t sslen = sizeof(ssObj.ss);

    // Test Unit
    EXPECT_FALSE(sockObj.is_bound());
    // errno = 0;
    // if (::getsockname(sockObj, (sockaddr*)&ssObj.ss, &sslen) != 0) {
    //     std::cout << std::strerror(errno) << "\n";
    //     GTEST_FAIL() << "::getsockname() returns SOCKET_ERROR";
    // }
    // EXPECT_EQ(ssObj.ss.ss_family, AF_INET6);
}

TEST(SockaddrStorageTestSuite, set_address_and_port_fail) {
    SSockaddr_storage saddr;

    EXPECT_THAT(
        [&saddr]() { saddr = "[2001::db8::1]"; },
        ThrowsMessage<std::invalid_argument>(EndsWith(
            "] EXCEPTION MSG1043: Invalid ip address '[2001::db8::1]'")));

    EXPECT_THAT([&saddr]() { saddr = "2001:db8::1]"; },
                ThrowsMessage<std::invalid_argument>(EndsWith(
                    "] EXCEPTION MSG1044: Invalid ip address '2001'")));

    EXPECT_THROW({ saddr = "[2001:db8::2"; }, std::out_of_range);

    EXPECT_THROW({ saddr = "[2001:db8::3]50003"; }, std::out_of_range);

    EXPECT_THAT(
        [&saddr]() { saddr = "[2001:db8::35003]"; },
        ThrowsMessage<std::invalid_argument>(EndsWith(
            "] EXCEPTION MSG1043: Invalid ip address '[2001:db8::35003]'")));

    EXPECT_THAT(
        [&saddr]() { saddr = "192.168.66.67."; },
        ThrowsMessage<std::invalid_argument>(EndsWith(
            "] EXCEPTION MSG1044: Invalid ip address '192.168.66.67.'")));

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
        reinterpret_cast<const sockaddr_in6*>(&saddr1.ss)->sin6_addr.s6_addr;
    const unsigned char* const s6_addr2 =
        reinterpret_cast<const sockaddr_in6*>(&saddr2.ss)->sin6_addr.s6_addr;
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

TEST(SockaddrStorageTestSuite, get_sslen) {
    SSockaddr_storage saddr1;
    EXPECT_EQ(saddr1.get_sslen(), sizeof(::sockaddr_storage));
}

TEST(ToPortTestSuite, str_to_port) {
    EXPECT_EQ(to_port("123"), 123);
    EXPECT_EQ(to_port("00456"), 456);
    EXPECT_EQ(to_port("65535"), 65535);
    EXPECT_EQ(to_port(""), 0);
    EXPECT_EQ(to_port("0"), 0);
    EXPECT_EQ(to_port("9"), 9);
    EXPECT_EQ(to_port("00000"), 0);

    EXPECT_THAT(
        []() { to_port("000000"); },
        ThrowsMessage<std::invalid_argument>(HasSubstr(
            "] EXCEPTION MSG1033: Failed to get port number for \"000000\"")));

    EXPECT_THAT(
        []() { to_port("65536"); },
        ThrowsMessage<std::invalid_argument>(HasSubstr(
            "] EXCEPTION MSG1033: Failed to get port number for \"65536\"")));

    EXPECT_THAT(
        []() { to_port("-1"); },
        ThrowsMessage<std::invalid_argument>(HasSubstr(
            "] EXCEPTION MSG1033: Failed to get port number for \"-1\"")));

    EXPECT_THAT(
        []() { to_port("123456"); },
        ThrowsMessage<std::invalid_argument>(HasSubstr(
            "] EXCEPTION MSG1033: Failed to get port number for \"123456\"")));

    EXPECT_THAT(
        []() { to_port(" "); },
        ThrowsMessage<std::invalid_argument>(HasSubstr(
            "] EXCEPTION MSG1033: Failed to get port number for \" \"")));

    EXPECT_THAT(
        []() { to_port(" 123"); },
        ThrowsMessage<std::invalid_argument>(HasSubstr(
            "] EXCEPTION MSG1033: Failed to get port number for \" 123\"")));

    EXPECT_THAT(
        []() { to_port("123 "); },
        ThrowsMessage<std::invalid_argument>(HasSubstr(
            "] EXCEPTION MSG1033: Failed to get port number for \"123 \"")));

    EXPECT_THAT(
        []() { to_port("123.4"); },
        ThrowsMessage<std::invalid_argument>(HasSubstr(
            "] EXCEPTION MSG1033: Failed to get port number for \"123.4\"")));

    EXPECT_THAT(
        []() { to_port(":1234"); },
        ThrowsMessage<std::invalid_argument>(HasSubstr(
            "] EXCEPTION MSG1033: Failed to get port number for \":1234\"")));

    EXPECT_THAT(
        []() { to_port("12x34"); },
        ThrowsMessage<std::invalid_argument>(HasSubstr(
            "] EXCEPTION MSG1033: Failed to get port number for \"12x34\"")));
}

TEST(ToAddrStrTestSuite, sockaddr_to_address_string) {
    SSockaddr_storage saddr;

    EXPECT_EQ(to_addr_str(&saddr.ss), "");

    saddr.ss.ss_family = AF_UNSPEC;
    EXPECT_EQ(to_addr_str(&saddr.ss), "");

    saddr.ss.ss_family = AF_INET6;
    EXPECT_EQ(to_addr_str(&saddr.ss), "[::]");

    saddr.ss.ss_family = AF_INET;
    EXPECT_EQ(to_addr_str(&saddr.ss), "0.0.0.0");

    saddr = "[2001:db8::4]";
    EXPECT_EQ(to_addr_str(&saddr.ss), "[2001:db8::4]");

    saddr = "192.168.88.99";
    EXPECT_EQ(to_addr_str(&saddr.ss), "192.168.88.99");

    saddr.ss.ss_family = AF_UNIX;
    EXPECT_THAT([&saddr]() { to_addr_str(&saddr.ss); },
                ThrowsMessage<std::invalid_argument>(HasSubstr(
                    "] EXCEPTION MSG1036: Unsupported address family 1")));
}

TEST(ToAddrStrTestSuite, sockaddr_to_address_port_string) {
    SSockaddr_storage saddr;

    EXPECT_EQ(to_addrport_str(&saddr.ss), "");

    saddr.ss.ss_family = AF_UNSPEC;
    EXPECT_EQ(to_addrport_str(&saddr.ss), "");

    saddr.ss.ss_family = AF_INET6;
    EXPECT_EQ(to_addrport_str(&saddr.ss), "[::]:0");

    saddr.ss.ss_family = AF_INET;
    EXPECT_EQ(to_addrport_str(&saddr.ss), "0.0.0.0:0");

    saddr = "[2001:db8::4]";
    EXPECT_EQ(to_addrport_str(&saddr.ss), "[2001:db8::4]:0");

    saddr = "192.168.88.100";
    EXPECT_EQ(to_addrport_str(&saddr.ss), "192.168.88.100:0");

    saddr = "[2001:db8::5]:56789";
    EXPECT_EQ(to_addrport_str(&saddr.ss), "[2001:db8::5]:56789");

    saddr = "192.168.88.101:54321";
    EXPECT_EQ(to_addrport_str(&saddr.ss), "192.168.88.101:54321");

    saddr.ss.ss_family = AF_UNIX;
    EXPECT_THAT([&saddr]() { to_addrport_str(&saddr.ss); },
                ThrowsMessage<std::invalid_argument>(HasSubstr(
                    "] EXCEPTION MSG1036: Unsupported address family 1")));
}

} // namespace utest


int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
    WINSOCK_INIT
#include <utest/utest_main.inc>
    return gtest_return_code; // managed in gtest_main.inc
}
