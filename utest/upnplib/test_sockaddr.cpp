// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-11-21

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
using ::upnplib::SSockaddr;
using ::upnplib::to_addr_str;
using ::upnplib::to_addrp_str;
using ::upnplib::to_port;


// SSockaddr TestSuite
// ===========================
class SetAddrPortTest : public ::testing::TestWithParam<
                            std::tuple<const std::string, const ::sa_family_t,
                                       const std::string, const in_port_t>> {};

TEST_P(SetAddrPortTest, set_address_and_port) {
    // Get parameter
    const std::tuple params = GetParam();
    const std::string netaddr = std::get<0>(params);
    const sa_family_t family = std::get<1>(params);
    const std::string addr_str = std::get<2>(params);
    const in_port_t port = std::get<3>(params);

    SSockaddr saddr;
    saddr = netaddr;
    EXPECT_EQ(saddr.ss.ss_family, family);
    EXPECT_EQ(saddr.get_addr_str(), addr_str);
    EXPECT_EQ(saddr.get_addrp_str(), addr_str + ":" + std::to_string(port));
    EXPECT_EQ(saddr.get_port(), port);
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(SetAddrPort, SetAddrPortTest, ::testing::Values(
    // std::make_tuple("", AF_UNSPEC, "", 0), // special, single tested
    // --- Essential for checking bind, see note next test
    std::make_tuple("[::]", AF_INET6, "[::]", 0),
    std::make_tuple("[::]:", AF_INET6, "[::]", 0),
    std::make_tuple("[::]:0", AF_INET6, "[::]", 0),
    // ---
    std::make_tuple("[::]:50064", AF_INET6, "[::]", 50064),
    std::make_tuple("[::1]", AF_INET6, "[::1]", 0),
    std::make_tuple("[::1]:", AF_INET6, "[::1]", 0),
    std::make_tuple("[::1]:0", AF_INET6, "[::1]", 0),
    std::make_tuple("[::1]:50065", AF_INET6, "[::1]", 50065),
    std::make_tuple("[2001:db8::68]", AF_INET6, "[2001:db8::68]", 0),
    std::make_tuple("[2001:db8::67]:", AF_INET6, "[2001:db8::67]", 0),
    std::make_tuple("[2001:db8::66]:50066", AF_INET6, "[2001:db8::66]", 50066),
    // --- Essential for checking bind, see note next test
    std::make_tuple("0.0.0.0", AF_INET, "0.0.0.0", 0),
    std::make_tuple("0.0.0.0:", AF_INET, "0.0.0.0", 0),
    std::make_tuple("0.0.0.0:0", AF_INET, "0.0.0.0", 0),
    // ---
    std::make_tuple("0.0.0.0:50068", AF_INET, "0.0.0.0", 50068),
    std::make_tuple("127.0.0.1", AF_INET, "127.0.0.1", 0),
    std::make_tuple("127.0.0.1:", AF_INET, "127.0.0.1", 0),
    std::make_tuple("127.0.0.1:50069", AF_INET, "127.0.0.1", 50069),
    std::make_tuple("192.168.33.34", AF_INET, "192.168.33.34", 0),
    std::make_tuple("192.168.33.35:", AF_INET, "192.168.33.35", 0),
    std::make_tuple("192.168.33.35:0", AF_INET, "192.168.33.35", 0),
    std::make_tuple("192.168.33.36:50067", AF_INET, "192.168.33.36", 50067)
));
// clang-format on
//
TEST(SockaddrStorageTestSuite, pattern_for_checking_bind) {
    // In addition to the marked pattern above these pattern are also essential
    // for portable checking if a socket is bound to a local interface with an
    // ip address by calling the system function '::bind()'.
    SSockaddr saddr;

    EXPECT_EQ(saddr.ss.ss_family, AF_UNSPEC);
    EXPECT_EQ(saddr.get_addr_str(), "");
    EXPECT_EQ(saddr.get_addrp_str(), "");
    EXPECT_EQ(saddr.get_port(), 0);

    saddr.ss.ss_family = AF_INET6;
    EXPECT_EQ(saddr.get_addr_str(), "[::]");
    EXPECT_EQ(saddr.get_addrp_str(), "[::]:0");
    EXPECT_EQ(saddr.get_port(), 0);

    saddr.ss.ss_family = AF_INET;
    EXPECT_EQ(saddr.get_addr_str(), "0.0.0.0");
    EXPECT_EQ(saddr.get_addrp_str(), "0.0.0.0:0");
    EXPECT_EQ(saddr.get_port(), 0);
}

TEST(SockaddrStorageTestSuite, set_address_and_port_successful) {
    SSockaddr saddr;

    saddr = "";
    EXPECT_EQ(saddr.ss.ss_family, AF_UNSPEC);
    EXPECT_EQ(saddr.get_addr_str(), "");
    EXPECT_EQ(saddr.get_addrp_str(), "");
    EXPECT_EQ(saddr.get_port(), 0);

    // Setting address and port in two steps
    saddr.ss.ss_family = AF_INET;
    saddr = "[2001:db8::3]:";
    EXPECT_EQ(saddr.ss.ss_family, AF_INET6);
    EXPECT_EQ(saddr.get_addr_str(), "[2001:db8::3]");
    EXPECT_EQ(saddr.get_addrp_str(), "[2001:db8::3]:0");
    EXPECT_EQ(saddr.get_port(), 0);
    saddr = "50021";
    EXPECT_EQ(saddr.ss.ss_family, AF_INET6);
    EXPECT_EQ(saddr.get_addr_str(), "[2001:db8::3]");
    EXPECT_EQ(saddr.get_addrp_str(), "[2001:db8::3]:50021");
    EXPECT_EQ(saddr.get_port(), 50021);
    saddr = ":50022";
    EXPECT_EQ(saddr.ss.ss_family, AF_INET6);
    EXPECT_EQ(saddr.get_addr_str(), "[2001:db8::3]");
    EXPECT_EQ(saddr.get_addrp_str(), "[2001:db8::3]:50022");
    EXPECT_EQ(saddr.get_port(), 50022);

    saddr = 0;
    EXPECT_EQ(saddr.ss.ss_family, AF_INET6);
    EXPECT_EQ(saddr.get_addr_str(), "[2001:db8::3]");
    EXPECT_EQ(saddr.get_addrp_str(), "[2001:db8::3]:0");
    EXPECT_EQ(saddr.get_port(), 0);

    saddr = 65535;
    EXPECT_EQ(saddr.ss.ss_family, AF_INET6);
    EXPECT_EQ(saddr.get_addr_str(), "[2001:db8::3]");
    EXPECT_EQ(saddr.get_addrp_str(), "[2001:db8::3]:65535");
    EXPECT_EQ(saddr.get_port(), 65535);

    // This will not modify the port
    saddr = "192.168.47.48";
    EXPECT_EQ(saddr.ss.ss_family, AF_INET);
    EXPECT_EQ(saddr.get_addr_str(), "192.168.47.48");
    EXPECT_EQ(saddr.get_addrp_str(), "192.168.47.48:65535");
    EXPECT_EQ(saddr.get_port(), 65535);

    // Check that a failing call does not modify old settings.
    EXPECT_THAT([&saddr]() { saddr = "65536"; },
                ThrowsMessage<std::invalid_argument>(
                    HasSubstr("] EXCEPTION MSG1033: ")));
    EXPECT_EQ(saddr.ss.ss_family, AF_INET);
    EXPECT_EQ(saddr.get_addr_str(), "192.168.47.48");
    EXPECT_EQ(saddr.get_addrp_str(), "192.168.47.48:65535");
    EXPECT_EQ(saddr.get_port(), 65535);
}

TEST(SockaddrStorageTestSuite, set_address_and_port_fail) {
    SSockaddr saddr;

    // Throws only with AF_UNSPEC that is also 0.
    saddr.ss.ss_family = AF_UNSPEC;
    EXPECT_THAT([&saddr]() { saddr = ":50070"; },
                ThrowsMessage<std::invalid_argument>(
                    EndsWith("] EXCEPTION MSG1044: Invalid netaddress \"\".")));

    // This does not compile with error
    // ‘short unsigned int’ changes value from ‘65536’ to ‘0’.
    // EXPECT_THAT([&saddr]() { saddr = 65536; },
    //             ThrowsMessage<std::invalid_argument>(
    //             EndsWith("] EXCEPTION MSG1044: Invalid netaddress \"\".")));

    EXPECT_THAT([&saddr]() { saddr = ":"; },
                ThrowsMessage<std::invalid_argument>(
                    EndsWith("] EXCEPTION MSG1044: Invalid netaddress \"\".")));

    EXPECT_THAT([&saddr]() { saddr = "garbage"; },
                ThrowsMessage<std::invalid_argument>(
                    EndsWith("] EXCEPTION MSG1033: Failed to get port number "
                             "for \"garbage\".")));

    EXPECT_THAT(
        [&saddr]() { saddr = "[2001::db8::1]"; },
        ThrowsMessage<std::invalid_argument>(EndsWith(
            "] EXCEPTION MSG1043: Invalid netaddress \"[2001::db8::1]\".")));

    EXPECT_THAT([&saddr]() { saddr = "2001:db8::1]"; },
                ThrowsMessage<std::invalid_argument>(EndsWith(
                    "] EXCEPTION MSG1044: Invalid netaddress \"2001\".")));

    EXPECT_THROW({ saddr = "[2001:db8::2"; }, std::out_of_range);

    EXPECT_THROW({ saddr = "[2001:db8::3]50003"; }, std::out_of_range);

    EXPECT_THAT(
        [&saddr]() { saddr = "[2001:db8::35003]"; },
        ThrowsMessage<std::invalid_argument>(EndsWith(
            "] EXCEPTION MSG1043: Invalid netaddress \"[2001:db8::35003]\".")));

    EXPECT_THAT(
        [&saddr]() { saddr = "192.168.66.67."; },
        ThrowsMessage<std::invalid_argument>(EndsWith(
            "] EXCEPTION MSG1044: Invalid netaddress \"192.168.66.67.\".")));

    EXPECT_THAT(
        [&saddr]() { saddr = "192.168.66.67z"; },
        ThrowsMessage<std::invalid_argument>(EndsWith(
            "] EXCEPTION MSG1044: Invalid netaddress \"192.168.66.67z\".")));
}

TEST(SockaddrCmpTestSuite, compare_equal_ipv6_sockaddrs_successful) {
    SSockaddr saddr1;
    saddr1 = "[2001:db8::33]:50033";
    SSockaddr saddr2;
    saddr2 = "[2001:db8::33]:50033";

    // Test Unit
    EXPECT_TRUE(sockaddrcmp(&saddr1.ss, &saddr2.ss));
}

TEST(SockaddrCmpTestSuite, compare_equal_ipv4_sockaddrs_successful) {
    SSockaddr saddr1;
    saddr1 = "192.168.167.166:50037";
    SSockaddr saddr2;
    saddr2 = "192.168.167.166:50037";

    // Test Unit
    EXPECT_TRUE(sockaddrcmp(&saddr1.ss, &saddr2.ss));
}

TEST(SockaddrCmpTestSuite, compare_different_ipv6_sockaddrs) {
    // Test Unit, compare with different ports
    SSockaddr saddr1;
    saddr1 = "[2001:db8::35]:50035";
    SSockaddr saddr2;
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
    SSockaddr saddr1;
    saddr1 = "192.168.13.14:50038";
    SSockaddr saddr2;
    saddr2 = "192.168.13.14:50039";
    EXPECT_FALSE(sockaddrcmp(&saddr1.ss, &saddr2.ss));

    // Test Unit, compare with different address
    saddr2 = "192.168.13.15:50038";
    EXPECT_FALSE(sockaddrcmp(&saddr1.ss, &saddr2.ss));

    // Test Unit, compare with different address family already done with
    // compare_different_ipv6_sockaddrs.
}

TEST(SockaddrCmpTestSuite, compare_unspecified_sockaddrs) {
    // Test Unit with unspecified address family will always return true if
    // equal, independent of address and port.
    SSockaddr saddr1;
    saddr1.ss.ss_family = AF_UNSPEC;
    SSockaddr saddr2;
    saddr2.ss.ss_family = AF_UNSPEC;
    EXPECT_TRUE(sockaddrcmp(&saddr1.ss, &saddr2.ss));

    saddr2 = "[2001:db8::40]:50040";
    EXPECT_FALSE(sockaddrcmp(&saddr1.ss, &saddr2.ss));
    EXPECT_FALSE(sockaddrcmp(&saddr2.ss, &saddr1.ss));
}

TEST(SockaddrCmpTestSuite, compare_nullptr_to_sockaddrs) {
    SSockaddr saddr1;
    saddr1 = "[2001:db8::34]:50034";

    // Test Unit
    EXPECT_TRUE(sockaddrcmp(nullptr, nullptr));
    EXPECT_FALSE(sockaddrcmp(nullptr, &saddr1.ss));
    EXPECT_FALSE(sockaddrcmp(&saddr1.ss, nullptr));
}

TEST(SockaddrStorageTestSuite, copy_and_assign_structure) {
    SSockaddr saddr1;
    saddr1 = "[2001:db8::1]:50001";

    // Test Unit copy
    // With UPNPLIB_WITH_TRACE we do not see "Construct ..." because the
    // default copy constructor is used here.
    SSockaddr saddr2 = saddr1;
    EXPECT_EQ(saddr2.ss.ss_family, AF_INET6);
    EXPECT_EQ(saddr2.get_addr_str(), "[2001:db8::1]");
    EXPECT_EQ(saddr2.get_port(), 50001);

    // Test Unit assign
    saddr1 = "192.168.251.252:50002";
    SSockaddr saddr3;
    saddr3 = saddr1;
    EXPECT_EQ(saddr3.ss.ss_family, AF_INET);
    EXPECT_EQ(saddr3.get_addr_str(), "192.168.251.252");
    EXPECT_EQ(saddr3.get_port(), 50002);
}

TEST(SockaddrStorageTestSuite, compare_ipv6_address) {
    SSockaddr saddr1;
    saddr1 = "[2001:db8::27]:50027";
    SSockaddr saddr2;
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
    SSockaddr saddr1;
    saddr1 = "192.168.0.1:50029";
    SSockaddr saddr2;
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
    SSockaddr saddr;

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
    SSockaddr saddr;

    EXPECT_EQ(to_addrp_str(&saddr.ss), "");

    saddr.ss.ss_family = AF_UNSPEC;
    EXPECT_EQ(to_addrp_str(&saddr.ss), "");

    saddr.ss.ss_family = AF_INET6;
    EXPECT_EQ(to_addrp_str(&saddr.ss), "[::]:0");

    saddr.ss.ss_family = AF_INET;
    EXPECT_EQ(to_addrp_str(&saddr.ss), "0.0.0.0:0");

    saddr = "[2001:db8::4]";
    EXPECT_EQ(to_addrp_str(&saddr.ss), "[2001:db8::4]:0");

    saddr = "192.168.88.100";
    EXPECT_EQ(to_addrp_str(&saddr.ss), "192.168.88.100:0");

    saddr = "[2001:db8::5]:56789";
    EXPECT_EQ(to_addrp_str(&saddr.ss), "[2001:db8::5]:56789");

    saddr = "192.168.88.101:54321";
    EXPECT_EQ(to_addrp_str(&saddr.ss), "192.168.88.101:54321");

    saddr.ss.ss_family = AF_UNIX;
    EXPECT_THAT([&saddr]() { to_addrp_str(&saddr.ss); },
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
