// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-11-13

// Include source code for testing. So we have also direct access to static
// functions which need to be tested.
#include <upnplib/src/net/addrinfo.cpp>

#include <upnplib/socket.hpp>
#include <utest/utest.hpp>


namespace utest {

using ::testing::AnyOf;
using ::testing::StartsWith;

using ::upnplib::CAddrinfo;
using ::upnplib::is_numeric_node;


TEST(AddrinfoTestSuite, get_numeric_host_successful) {
    // If node is not empty AI_PASSIVE is ignored.
    // Flag AI_NUMERICHOST should be set if possible to avoid expensive name
    // resolution from external DNS server.

    // Test Unit with string port number
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

    // Test Unit with numeric port number
    CAddrinfo ai2("[2001:db8::2]", 50048, AF_INET6, SOCK_STREAM,
                  AI_NUMERICHOST);

    EXPECT_EQ(ai2->ai_family, AF_INET6);
    EXPECT_EQ(ai2->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai2->ai_protocol, 0);
    EXPECT_EQ(ai2->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai2.addr_str(), "[2001:db8::2]");
    EXPECT_EQ(ai2.port(), 50048);

    // Test Unit with unspecified ipv6 address
    CAddrinfo ai3("[::]", 0, AF_INET6, 0, AI_NUMERICHOST | AI_NUMERICSERV);

    EXPECT_EQ(ai3->ai_family, AF_INET6);
    EXPECT_EQ(ai3->ai_socktype, 0);
    EXPECT_EQ(ai3->ai_protocol, 0);
    EXPECT_EQ(ai3->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai3.addr_str(), "[::]");
    EXPECT_EQ(ai3.port(), 0);

    // Test Unit with unspecified ipv4 address
    CAddrinfo ai4("0.0.0.0", 0, AF_INET, 0, AI_NUMERICHOST | AI_NUMERICSERV);

    EXPECT_EQ(ai4->ai_family, AF_INET);
    EXPECT_EQ(ai4->ai_socktype, 0);
    EXPECT_EQ(ai4->ai_protocol, 0);
    EXPECT_EQ(ai4->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai4.addr_str(), "0.0.0.0");
    EXPECT_EQ(ai4.port(), 0);
}

TEST(AddrinfoTestSuite, get_implicit_ipv6_host) {
    // Test Unit
    CAddrinfo ai1("[2001:db8::3]", "50032", AF_INET6, SOCK_STREAM,
                  AI_NUMERICHOST);

    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    // Numeric host implicit set
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai1.addr_str(), "[2001:db8::3]");
    EXPECT_EQ(ai1.port(), 50032);

    // Test Unit
    CAddrinfo ai3("[2001:db8::5]", "50051", AF_UNSPEC, SOCK_STREAM);

    EXPECT_EQ(ai3->ai_family, AF_INET6);
    EXPECT_EQ(ai3->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai3->ai_protocol, 0);
    EXPECT_EQ(ai3->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai3.addr_str(), "[2001:db8::5]");
    EXPECT_EQ(ai3.port(), 50051);

    // Test Unit
    CAddrinfo ai2("localhost", "50050", AF_INET6);

    EXPECT_EQ(ai2->ai_family, AF_INET6);
    EXPECT_EQ(ai2->ai_socktype, 0);
    EXPECT_EQ(ai2->ai_protocol, 0);
    EXPECT_EQ(ai2->ai_flags, 0);
    EXPECT_EQ(ai2.addr_str(), "[::1]");
    EXPECT_EQ(ai2.port(), 50050);
}

TEST(AddrinfoTestSuite, get_alphanumeric_host_successful) {
    // If node is not empty AI_PASSIVE is ignored.
    // Test Unit
    CAddrinfo ai1("localhost", "50049", AF_UNSPEC, SOCK_DGRAM);

    EXPECT_THAT(ai1->ai_family, AnyOf(AF_INET6, AF_INET));
    EXPECT_EQ(ai1->ai_socktype, SOCK_DGRAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, 0);
    EXPECT_THAT(ai1.addr_str(), AnyOf("[::1]", "127.0.0.1"));
    EXPECT_EQ(ai1.port(), 50049);
}

TEST(AddrinfoTestSuite, get_unknown_host) {
    // With AI_NUMERICHOST "localhost" is unknown.
    // Test Unit
    CAddrinfo ai1("localhost", "50031", AF_INET, SOCK_STREAM, AI_NUMERICHOST);

    EXPECT_EQ(ai1->ai_family, AF_INET);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai1.addr_str(), "0.0.0.0");
    EXPECT_EQ(ai1.port(), 50031);

    // Test Unit
    CAddrinfo ai2("localhost", "50052", AF_INET6, SOCK_DGRAM, AI_NUMERICHOST);

    EXPECT_EQ(ai2->ai_family, AF_INET6);
    EXPECT_EQ(ai2->ai_socktype, SOCK_DGRAM);
    EXPECT_EQ(ai2->ai_protocol, 0);
    EXPECT_EQ(ai2->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai2.addr_str(), "[::]");
    EXPECT_EQ(ai2.port(), 50052);

    // Test Unit
    CAddrinfo ai3("localhost", "50053", AF_UNSPEC, SOCK_STREAM, AI_NUMERICHOST);

    EXPECT_EQ(ai3->ai_family, AF_INET6);
    EXPECT_EQ(ai3->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai3->ai_protocol, 0);
    EXPECT_EQ(ai3->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai3.addr_str(), "[::]");
    EXPECT_EQ(ai3.port(), 50053);

    // Test Unit
    // An alphanumeric node with enclosing brackets may be valid if
    // defined anywhere (/etc/hosts file, DNS lookup, etc.) but here it
    // shouldn't.
    CAddrinfo ai4("[localhost]", "50055", AF_UNSPEC, SOCK_DGRAM,
                  AI_NUMERICHOST);

    EXPECT_EQ(ai4->ai_family, AF_INET6);
    EXPECT_EQ(ai4->ai_socktype, SOCK_DGRAM);
    EXPECT_EQ(ai4->ai_protocol, 0);
    EXPECT_EQ(ai4->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai4.addr_str(), "[::]");
    EXPECT_EQ(ai4.port(), 50055);
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
    // When using this to listen, it will listen on all local network
    // interfaces.
    EXPECT_EQ(ai1.addr_str(), "[::]");
    EXPECT_EQ(ai1.port(), 50006);
}

TEST(AddrinfoTestSuite, get_info_loopback_interface) {
    // To get info of the loopback interface, node must be empty without
    // AI_PASSIVE flag set. The value AF_UNSPEC indicates that getaddrinfo()
    // should return socket addresses for any address family (either IPv4 or
    // IPv6) that can be used with node and service.
    // Node "" or "[]" are always equivalent.

    // Test Unit
    CAddrinfo ai1("", "50007", AF_UNSPEC, SOCK_STREAM, AI_NUMERICHOST);

    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai1.port(), 50007);
    switch (ai1->ai_family) {
    case AF_INET6:
        EXPECT_EQ(ai1.addr_str(), "[::1]");
        break;
    case AF_INET:
        EXPECT_EQ(ai1.addr_str(), "127.0.0.1");
        break;
    default:
        FAIL() << "invalid address family " << ai1->ai_family;
    }

    // Test Unit
    CAddrinfo ai2("", "50056", AF_INET6);

    EXPECT_EQ(ai2->ai_family, AF_INET6);
    EXPECT_EQ(ai2->ai_socktype, 0);
    EXPECT_EQ(ai2->ai_protocol, 0);
    EXPECT_EQ(ai2->ai_flags, 0);
    EXPECT_EQ(ai2.addr_str(), "[::1]");
    EXPECT_EQ(ai2.port(), 50056);

    // Test Unit
    CAddrinfo ai3("", "50057", AF_INET);

    EXPECT_EQ(ai3->ai_family, AF_INET);
    EXPECT_EQ(ai3->ai_socktype, 0);
    EXPECT_EQ(ai3->ai_protocol, 0);
    EXPECT_EQ(ai3->ai_flags, 0);
    EXPECT_EQ(ai3.addr_str(), "127.0.0.1");
    EXPECT_EQ(ai3.port(), 50057);

    // Test Unit
    CAddrinfo ai4("", "50058", AF_INET6, SOCK_STREAM,
                  AI_NUMERICHOST | AI_NUMERICSERV);

    EXPECT_EQ(ai4->ai_family, AF_INET6);
    EXPECT_EQ(ai4->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai4->ai_protocol, 0);
    EXPECT_EQ(ai4->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai4.addr_str(), "[::1]");
    EXPECT_EQ(ai4.port(), 50058);
}

TEST(AddrinfoTestSuite, invalid_ipv6_node_address) {
    // Test Unit. Node address must be surounded with brackets.

    CAddrinfo ai1("::1", "", AF_INET6, SOCK_STREAM, AI_NUMERICHOST);

    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai1.addr_str(), "[::]");
    EXPECT_EQ(ai1.port(), 0);

    CAddrinfo ai2("[::1", "", AF_INET6, SOCK_STREAM, AI_NUMERICHOST);

    EXPECT_EQ(ai2->ai_family, AF_INET6);
    EXPECT_EQ(ai2->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai2->ai_protocol, 0);
    EXPECT_EQ(ai2->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai2.addr_str(), "[::]");
    EXPECT_EQ(ai2.port(), 0);

    CAddrinfo ai3("::1]", "", AF_INET6, SOCK_STREAM, AI_NUMERICHOST);

    EXPECT_EQ(ai3->ai_family, AF_INET6);
    EXPECT_EQ(ai3->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai3->ai_protocol, 0);
    EXPECT_EQ(ai3->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai3.addr_str(), "[::]");
    EXPECT_EQ(ai3.port(), 0);
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
    CAddrinfo ai1("127.0.0.1", "50003", AF_INET6, SOCK_STREAM, AI_NUMERICHOST);

    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai1.addr_str(), "[::]");
    EXPECT_EQ(ai1.port(), 50003);
}

TEST(AddrinfoTestSuite, copy_ipv6_successful) {
    // This tests the copy constructor.

    // Get valid address information.
    CAddrinfo ai1("[2001:db8::6]", "50054", AF_INET6, SOCK_STREAM,
                  AI_NUMERICHOST);

    // Test Unit
    { // scope for ai2
        CAddrinfo ai2 = ai1;

        EXPECT_EQ(ai2->ai_family, AF_INET6);
        EXPECT_EQ(ai2->ai_socktype, SOCK_STREAM);
        EXPECT_EQ(ai2->ai_protocol, 0);
        EXPECT_EQ(ai2->ai_flags, AI_NUMERICHOST);
        EXPECT_EQ(ai1.addr_str(), "[2001:db8::6]");
        EXPECT_EQ(ai2.port(), 50054);
    } // End scope, ai2 will be destructed

    // Check if ai1 is still available.
    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai1.addr_str(), "[2001:db8::6]");
    EXPECT_EQ(ai1.port(), 50054);
}

TEST(AddrinfoTestSuite, copy_ipv4_successful) {
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

TEST(AddrinfoTestSuite, is_numeric_node_successful) {
    EXPECT_EQ(is_numeric_node("[2001:db8::4]", AF_INET6), AF_INET6);
    EXPECT_EQ(is_numeric_node("192.168.47.9", AF_INET), AF_INET);
    EXPECT_EQ(is_numeric_node("192.168.47.8"), AF_INET);
    EXPECT_EQ(is_numeric_node("[2001:db8::5]"), AF_INET6);
    EXPECT_EQ(is_numeric_node("[::1]"), AF_INET6);
    EXPECT_EQ(is_numeric_node("[::]"), AF_INET6);
}

TEST(AddrinfoTestSuite, is_numeric_node_fails) {
    EXPECT_FALSE(is_numeric_node("2001:db8::4", AF_INET6));
    EXPECT_FALSE(is_numeric_node("localhost", AF_INET6));
    EXPECT_FALSE(is_numeric_node("192.168.47.a", AF_INET));
    EXPECT_FALSE(is_numeric_node("example.com"));
    EXPECT_FALSE(is_numeric_node("::1"));
    EXPECT_FALSE(is_numeric_node("[::1"));
    EXPECT_FALSE(is_numeric_node("::1]"));
    EXPECT_FALSE(is_numeric_node("::"));
    if (!github_actions) {
        // This gives a runtime error. Since C++23 adopted [P2166]
        // (http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p2166r1.html)
        // nullptr is prohibited now. Check if the compiler fails instead of
        // having a runtime error.
        std::cout << CYEL "[ TODO     ] " CRES << __LINE__
                  << ": Check if using a nullptr to a std::string is detected "
                     "by the compiler now.\n";
        EXPECT_FALSE(is_numeric_node(nullptr));
    }
}

} // namespace utest


int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
    WINSOCK_INIT
#include <utest/utest_main.inc>
    return gtest_return_code; // managed in gtest_main.inc
}
