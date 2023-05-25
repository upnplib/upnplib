// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-05-28

#include <upnplib/addrinfo.hpp>
#include <upnplib/socket.hpp>

#include <gmock/gmock.h>

using testing::HasSubstr;


namespace upnplib {
bool old_code{false}; // Managed in upnplib_gtest_main.inc

TEST(AddrinfoTestSuite, get_successful) {
    // If node is not empty AI_PASSIVE is ignored.
    // Flag AI_NUMERICHOST must always be set to avoid expensive name resolution
    // from external DNS server. This will be changed when we need name
    // resolution.

    // Test Unit
    CAddrinfo ai1("[::1]", "50001", AF_INET6, SOCK_STREAM,
                  AI_PASSIVE | AI_NUMERICHOST);

    // Returns what ::getaddrinfo() returns.
    EXPECT_EQ(ai1->ai_family, AF_INET6);
    // Returns what ::getaddrinfo() returns.
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    // Different on platforms: Ubuntu & MacOS return 6, win32 returns 0.
    // We just return that what was requested by the user.
    EXPECT_EQ(ai1->ai_protocol, 0);
    // Different on platforms: Ubuntu returns 1025, MacOS & win32 return 0.
    // We just return that what was requested by the user.
    EXPECT_EQ(ai1->ai_flags, AI_PASSIVE | AI_NUMERICHOST);
    // Returns what ::getaddrinfo() returns.
    EXPECT_EQ(ai1.addr_str(), "[::1]");
    EXPECT_EQ(ai1.port(), 50001);
}

TEST(AddrinfoTestSuite, alphanumeric_host_is_not_supported) {
    // Test Unit
    EXPECT_THAT(
        []() { CAddrinfo ai1("localhost", "50032", AF_INET6, SOCK_STREAM); },
        ThrowsMessage<std::invalid_argument>(
            HasSubstr("ERROR! Failed to get address information: invalid "
                      "numeric node address.")));
}

TEST(AddrinfoTestSuite, get_unknown_host) {
    // With AI_NUMERICHOST "localhost" is unknown.

    // Test Unit
    EXPECT_THAT(
        []() {
            CAddrinfo ai1("localhost", "50031", AF_INET6, SOCK_STREAM,
                          AI_NUMERICHOST);
        },
        ThrowsMessage<std::invalid_argument>(
            HasSubstr("ERROR! Failed to get address information: invalid "
                      "numeric node address.")));
}

TEST(AddrinfoTestSuite, get_passive_addressinfo) {
    // To get a passive address info, node must be empty ("") or an empty
    // address ("[]") otherwise flag AI_PASSIVE is ignored.
    // Node "" or "[]" are always equivalent.

    // Test Unit
    // Can also use "" for node.
    CAddrinfo ai1("[]", "50006", AF_INET6, SOCK_STREAM,
                  AI_PASSIVE | AI_NUMERICHOST);

    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_PASSIVE | AI_NUMERICHOST);
    // wildcard address ipv4 = 0.0.0.0, ipv6 = ::/128
    EXPECT_EQ(ai1.addr_str(), "[::]");
    EXPECT_EQ(ai1.port(), 50006);
}

TEST(AddrinfoTestSuite, get_info_loopback_interface) {
    // To get info of the loopback interface node must be empty without
    // AI_PASSIVE flag set.
    // Node "" or "[]" are always equivalent.

    // Test Unit
    CAddrinfo ai1("", "50007", AF_UNSPEC, SOCK_STREAM, AI_NUMERICHOST);

    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai1.addr_str(), "[::1]");
    EXPECT_EQ(ai1.port(), 50007);
}

TEST(AddrinfoTestSuite, invalid_ipv6_node_address) {
    // Test Unit. Node address must be surounded with brackets.
    EXPECT_THAT(
        []() {
            CAddrinfo ai1("::1", "", AF_INET6, SOCK_STREAM, AI_NUMERICHOST);
        },
        ThrowsMessage<std::invalid_argument>(
            HasSubstr("ERROR! Failed to get address information: invalid "
                      "numeric node address.")));

    EXPECT_THAT(
        []() {
            CAddrinfo ai1("[::1", "", AF_INET6, SOCK_STREAM, AI_NUMERICHOST);
        },
        ThrowsMessage<std::invalid_argument>(
            HasSubstr("ERROR! Failed to get address information: invalid "
                      "numeric node address.")));

    EXPECT_THAT(
        []() {
            CAddrinfo ai1("::1]", "", AF_INET6, SOCK_STREAM, AI_NUMERICHOST);
        },
        ThrowsMessage<std::invalid_argument>(
            HasSubstr("ERROR! Failed to get address information: invalid "
                      "numeric node address.")));
}

TEST(AddrinfoTestSuite, empty_service) {
    // With a node but an empty service the returned port number in the address
    // structure is set to 0.

    // Test Unit
    CAddrinfo ai1("[2001:db8::2]", "", AF_INET6, SOCK_STREAM, AI_NUMERICHOST);

    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai1.addr_str(), "[2001:db8::2]");
    EXPECT_EQ(ai1.port(), 0);
}

TEST(AddrinfoTestSuite, get_fails) {
    // Test Unit. Address family does not match the numeric host address.
    EXPECT_THAT(
        []() {
            CAddrinfo ai1("127.0.0.1", "50003", AF_INET6, SOCK_STREAM,
                          AI_NUMERICHOST);
        },
        // errid(-9)="Address family for hostname not supported"
        ThrowsMessage<std::invalid_argument>(
            HasSubstr("ERROR! Failed to get address information: invalid "
                      "numeric node address.")));
}

TEST(AddrinfoTestSuite, copy_successful) {
    // This tests the copy constructor.

    // Get valid address information.
    CAddrinfo ai1("127.0.0.1", "50002", AF_INET, SOCK_DGRAM, AI_NUMERICHOST);

    // Test Unit
    { // scope for ai2
        CAddrinfo ai2 = ai1;

        EXPECT_EQ(ai2->ai_family, AF_INET);
        EXPECT_EQ(ai2->ai_socktype, SOCK_DGRAM);
        EXPECT_EQ(ai2->ai_protocol, 0);
        EXPECT_EQ(ai2->ai_flags, AI_NUMERICHOST);
        EXPECT_EQ(ai2.addr_str(), "127.0.0.1");
        EXPECT_EQ(ai2.port(), 50002);
    } // End scope, ai2 will be destructed

    // Check if ai1 is still available.
    EXPECT_EQ(ai1->ai_family, AF_INET);
    EXPECT_EQ(ai1->ai_socktype, SOCK_DGRAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai1.addr_str(), "127.0.0.1");
    EXPECT_EQ(ai1.port(), 50002);
}
/*
 * TEST(AddrinfoTestSuite, copy_fails) {
 *      This isn't possible because the test would be:
 *      CAddrinfo ai2 = ai1;
 *      and I cannot find a way to provide an invalid object ai1 that the
 *      compiler accepts. --Ingo
 * }
 */
TEST(AddrinfoTestSuite, assign_other_object_successful) {
    // This tests the copy assignment operator.
    // Get two valid address informations.

    // With not empty node AI_PASSIVE is ignored.
    CAddrinfo ai1("[2001:db8::1]", "50004", AF_INET6, SOCK_STREAM,
                  AI_PASSIVE | AI_NUMERICHOST);

    CAddrinfo ai2("192.168.47.11", "50005", AF_INET, SOCK_DGRAM,
                  AI_NUMERICHOST);

    // Test Unit
    ai2 = ai1;

    // Check ai2.
    EXPECT_EQ(ai2->ai_family, AF_INET6);
    EXPECT_EQ(ai2->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai2->ai_protocol, 0);
    EXPECT_EQ(ai2->ai_flags, AI_PASSIVE | AI_NUMERICHOST);
    EXPECT_EQ(ai2.addr_str(), "[2001:db8::1]");
    EXPECT_EQ(ai2.port(), 50004);

    // Check if ai1 is still available.
    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_PASSIVE | AI_NUMERICHOST);
    EXPECT_EQ(ai1.addr_str(), "[2001:db8::1]");
    EXPECT_EQ(ai1.port(), 50004);
}

TEST(AddrinfoTestSuite, compare_two_ipv6_address_infos_successful) {
    CAddrinfo ai1("[2001:db8::41]", "50041", AF_INET6, SOCK_STREAM,
                  AI_NUMERICHOST);
    CAddrinfo ai2("[2001:db8::41]", "50041", AF_INET6, SOCK_STREAM,
                  AI_NUMERICHOST);

    // Test Unit
    EXPECT_TRUE(ai1 == ai2);
}

TEST(AddrinfoTestSuite, compare_two_ipv4_address_infos_successful) {
    CAddrinfo ai1("192.168.66.42", "50042", AF_INET, SOCK_DGRAM,
                  AI_NUMERICHOST);
    CAddrinfo ai2("192.168.66.42", "50042", AF_INET, SOCK_DGRAM,
                  AI_NUMERICHOST);

    // Test Unit
    EXPECT_TRUE(ai1 == ai2);
}

TEST(AddrinfoTestSuite, compare_different_address_infos) {
    CAddrinfo ai1("[2001:db8::41]", "50041", AF_INET6, SOCK_STREAM,
                  AI_NUMERICHOST | AI_NUMERICSERV, IPPROTO_TCP);

    // Test Unit with different address
    CAddrinfo ai2("[2001:db8::42]", "50041", AF_INET6, SOCK_STREAM,
                  AI_NUMERICHOST | AI_NUMERICSERV, IPPROTO_TCP);
    EXPECT_FALSE(ai1 == ai2);

    // Test Unit with different service
    CAddrinfo ai3("[2001:db8::41]", "50043", AF_INET6, SOCK_STREAM,
                  AI_NUMERICHOST | AI_NUMERICSERV, IPPROTO_TCP);
    EXPECT_FALSE(ai1 == ai3);

    // Test Unit with different flags
    CAddrinfo ai4("[2001:db8::41]", "50041", AF_INET6, SOCK_STREAM,
                  AI_NUMERICHOST | AI_NUMERICSERV, IPPROTO_TCP);
    ai4->ai_flags = AI_NUMERICHOST;
    EXPECT_FALSE(ai1 == ai4);

    // Test Unit with different address family
    ai4->ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;
    ai4->ai_family = AF_INET;
    EXPECT_FALSE(ai1 == ai4);

    // Test Unit with different address length
    ai4->ai_family = AF_INET6;
    ai4->ai_addrlen--;
    EXPECT_FALSE(ai1 == ai4);

    // Test Unit with different socket type
    ai4->ai_addrlen++;
    ai4->ai_socktype = SOCK_DGRAM;
    EXPECT_FALSE(ai1 == ai4);

    // Test Unit with different protocol
    ai1->ai_socktype = SOCK_DGRAM;
    ai1->ai_protocol = 0;
    ai4->ai_protocol = IPPROTO_UDP;
    EXPECT_FALSE(ai1 == ai4);
}

} // namespace upnplib


int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
    WINSOCK_INIT_P
#include <upnplib/gtest_main.inc>
    return gtest_return_code; // managed in upnplib/gtest_main.inc
}
