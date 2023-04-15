// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-04-15

#include <upnplib/addrinfo.hpp>
#include <upnplib/socket.hpp>

#include <gmock/gmock.h>

using testing::HasSubstr;


namespace upnplib {
bool old_code{false}; // Managed in upnplib_gtest_main.inc

TEST(AddrinfoTestSuite, get_successful) {
    // If node is not empty  AI_PASSIVE is ignored.
    WINSOCK_INIT_P

    // Test Unit
    CAddrinfo ai1("localhost", "50001", AF_INET6, SOCK_STREAM,
                  AI_PASSIVE | AI_NUMERICSERV);

    // Returns what getaddrinfo() returns.
    EXPECT_EQ(ai1->ai_family, AF_INET6);
    // Returns what getaddrinfo() returns.
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    // Different on platforms: Ubuntu & MacOS return 6, win32 returns 0.
    // We just return that what was requested by the user.
    EXPECT_EQ(ai1->ai_protocol, 0);
    // Different on platforms: Ubuntu returns 1025, MacOS & win32 return 0.
    // We just return that what was requested by the user.
    EXPECT_EQ(ai1->ai_flags, AI_PASSIVE | AI_NUMERICSERV);
    // Returns what getaddrinfo() returns.
    EXPECT_EQ(ai1.addr_str(), "::1");
    EXPECT_EQ(ai1.port(), 50001);
}

TEST(AddrinfoTestSuite, get_passive_addressinfo) {
    // To get a passive address info, node must be empty otherwise flag
    // AI_PASSIVE is ignored.
    WINSOCK_INIT_P

    // Test Unit
    CAddrinfo ai1("", "50006", AF_INET6, SOCK_STREAM,
                  AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);

    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);
    // wildcard address ipv4 = 0.0.0.0, ipv6 = ::/128
    EXPECT_EQ(ai1.addr_str(), "::");
    EXPECT_EQ(ai1.port(), 50006);
}

TEST(AddrinfoTestSuite, get_info_loopback_interface) {
    // To get info of the loopback interface node must be empty without
    // AI_PASSIVE flag set.
    WINSOCK_INIT_P

    // Test Unit
    CAddrinfo ai1("", "50007", AF_UNSPEC, SOCK_STREAM,
                  AI_NUMERICHOST | AI_NUMERICSERV);

    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai1.addr_str(), "::1");
    EXPECT_EQ(ai1.port(), 50007);
}

TEST(AddrinfoTestSuite, uninitilized_port_nummer) {
    // With a node but an empty service the returned port number in the address
    // structure remains uninitialized. It appears to be initialized to zero
    // nonetheless, but should not be relied upon.
    WINSOCK_INIT_P

    // Test Unit
    CAddrinfo ai1("::1", "", AF_INET6, SOCK_STREAM,
                  AI_NUMERICHOST | AI_NUMERICSERV);

    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai1.addr_str(), "::1");
    EXPECT_EQ(ai1.port(), 0);
}

TEST(AddrinfoTestSuite, get_fails) {
    // Test Unit. Address family does not match the numeric host address.
    EXPECT_THAT(
        []() {
            CAddrinfo ai1("127.0.0.1", "50003", AF_INET6, SOCK_STREAM,
                          AI_NUMERICHOST | AI_NUMERICSERV);
        },
        // errid(-9)="Address family for hostname not supported"
        ThrowsMessage<std::runtime_error>(
            HasSubstr("ERROR! Failed to get address information: errid(")));
}

TEST(AddrinfoTestSuite, copy_successful) {
    // This tests the copy constructor.
    // Get valid address information.
    WINSOCK_INIT_P

    CAddrinfo ai1("127.0.0.1", "50002", AF_INET, SOCK_DGRAM,
                  AI_NUMERICHOST | AI_NUMERICSERV);

    // Test Unit
    { // scope for ai2
        CAddrinfo ai2 = ai1;

        EXPECT_EQ(ai2->ai_family, AF_INET);
        EXPECT_EQ(ai2->ai_socktype, SOCK_DGRAM);
        EXPECT_EQ(ai2->ai_protocol, 0);
        EXPECT_EQ(ai2->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
        EXPECT_EQ(ai2.addr_str(), "127.0.0.1");
        EXPECT_EQ(ai2.port(), 50002);
    } // End scope, ai2 will be destructed

    // Check if ai1 is still available.
    EXPECT_EQ(ai1->ai_family, AF_INET);
    EXPECT_EQ(ai1->ai_socktype, SOCK_DGRAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai1.addr_str(), "127.0.0.1");
    EXPECT_EQ(ai1.port(), 50002);
}
/*
 * TEST(AddrinfoTestSuite, copy_fails) {
 *      This isn't possible because the test would be:
 *      CAddrinfo ai2 = ai1;
 *      and I cannot find a way to provide an invalid object ai1 that the
 *      compiler accepts.
 * }
 */
TEST(AddrinfoTestSuite, assign_other_object_successful) {
    // This tests the copy assignment operator.
    // Get two valid address informations.
    WINSOCK_INIT_P

    // With node != nullptr AI_PASSIVE is ignored.
    CAddrinfo ai1("::1", "50004", AF_INET6, SOCK_STREAM,
                  AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);

    CAddrinfo ai2("localhost", "50005", AF_INET, SOCK_DGRAM, AI_NUMERICSERV);

    // Test Unit
    ai2 = ai1;

    // Check ai2.
    EXPECT_EQ(ai2->ai_family, AF_INET6);
    EXPECT_EQ(ai2->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai2->ai_protocol, 0);
    EXPECT_EQ(ai2->ai_flags, AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai2.addr_str(), "::1");
    EXPECT_EQ(ai2.port(), 50004);

    // Check if ai1 is still available.
    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai1.addr_str(), "::1");
    EXPECT_EQ(ai1.port(), 50004);
}

} // namespace upnplib


int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include <upnplib/gtest_main.inc>
    return gtest_return_code; // managed in upnplib/gtest_main.inc
}
