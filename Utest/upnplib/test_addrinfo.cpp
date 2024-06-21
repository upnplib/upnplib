// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: r024-06-28

// Include source code for testing. So we have also direct access to static
// functions which need to be tested.
#include <Upnplib/src/net/addrinfo.cpp>

#include <upnplib/socket.hpp>
#include <utest/utest.hpp>
#include <umock/netdb_mock.hpp>


namespace utest {

using ::testing::_;
using ::testing::AnyOf;
using ::testing::Field;
using ::testing::Pointee;
using ::testing::Return;
using ::testing::StrictMock;

using ::upnplib::CAddrinfo;


class NetaddrAssignTest
    : public ::testing::TestWithParam<
          std::tuple<const std::string, const std::string>> {};

TEST_P(NetaddrAssignTest, netaddress_assign) {
    // Get parameter
    std::tuple params = GetParam();

    // Test Unit
    CAddrinfo aiObj(std::get<0>(params));
    if (std::get<1>(params) == "") {
        EXPECT_THROW(aiObj.init(), std::runtime_error);
    } else {
        aiObj.init();
        EXPECT_EQ(aiObj.netaddr().str(), std::get<1>(params));
    }
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(
    NetaddrAssign, NetaddrAssignTest,
    ::testing::Values(
        // This Test checks the netaddress with port.
        // With an invalid address part the whole netaddress is unspecified,
        // except with the first following well defined unspecified addresses.
        // A valid addreess with an invalid port results to port 0.
        /*0*/ std::make_tuple("[::]", "[::]:0"),
        std::make_tuple("[::]:", "[::]:0"),
        std::make_tuple("[::]:0", "[::]:0"),
        std::make_tuple("[::]:65535", "[::]:65535"), // port 0 to 65535
        std::make_tuple("0.0.0.0", "0.0.0.0:0"),
        std::make_tuple("0.0.0.0:", "0.0.0.0:0"),
        std::make_tuple("0.0.0.0:0", "0.0.0.0:0"),
        std::make_tuple("0.0.0.0:65535", "0.0.0.0:65535"), // port 0 to 65535
        // Following invalid address parts will be general unspecified ("").
        std::make_tuple("[", ""),
        std::make_tuple("]", ""),
        /*10*/ std::make_tuple("[]", ""),
        std::make_tuple(":", ""),
        std::make_tuple(":0", ""),
        std::make_tuple(":50987", "[::1]:50987"),
        std::make_tuple(".", ""),
        std::make_tuple(".:", ""),
        std::make_tuple(":.", ""),
        std::make_tuple("::", ""),
        std::make_tuple(":::", ""),
        std::make_tuple("[::", ""),
        /*20*/ std::make_tuple("::]", ""),
        // std::make_tuple("[::1", ""), // tested later
        // std::make_tuple("::1]", ""), // tested later
        std::make_tuple("[::1]", "[::1]:0"),
        std::make_tuple("[::1]:", "[::1]:0"),
        std::make_tuple("[::1]:0", "[::1]:0"),
        std::make_tuple("[::1].4", ""), // dot for colon, may be an alphanum
        std::make_tuple("127.0.0.1", "127.0.0.1:0"),
        std::make_tuple("127.0.0.1:", "127.0.0.1:0"),
        std::make_tuple("127.0.0.1:0", "127.0.0.1:0"),
        // std::make_tuple("127.0.0.1.4", ""), // dot for colon, tested later
        std::make_tuple("[2001:db8::43]:", "[2001:db8::43]:0"),
        std::make_tuple("2001:db8::41:59897", ""), // no brackets
        /*30*/ std::make_tuple("[2001:db8::fg]", ""),
        std::make_tuple("[2001:db8::fg]:59877", ""),
        // std::make_tuple("[2001:db8::42]:65535", "[2001:db8::42]:65535"), // tested later
        std::make_tuple("[2001:db8::51]:65536", ""), // invalid port
        std::make_tuple("[2001:db8::52::53]", ""), // double double colon
        std::make_tuple("[2001:db8::52::53]:65535", ""), // double double colon
        std::make_tuple("[12.168.88.95]", ""), // IPv4 address with brackets
        std::make_tuple("[12.168.88.96]:", ""),
        std::make_tuple("[12.168.88.97]:9876", ""),
        // std::make_tuple("192.168.88.98:59876", "192.168.88.98:59876"), // tested later
        std::make_tuple("192.168.88.99:65537", ""), // invalid port
        // std::make_tuple("192.168.88.256:59866", ""), // tested later
        // std::make_tuple("192.168.88.91", "192.168.88.91:0"), // tested later
        // std::make_tuple("garbage:49493", ""), // triggers DNS lookup
        std::make_tuple("[garbage]:49494", ""),
        /*40*/ std::make_tuple("[2001:db8::44]:https", "[2001:db8::44]:443"),
        std::make_tuple("[2001:db8::44]:httpx", ""),
        std::make_tuple("192.168.88.98:http", "192.168.88.98:80"),
        std::make_tuple("192.168.88.98:httpy", ""),
        std::make_tuple("[::1%1]", "[::1]:0"), // should be "[::1%1]:0"),
        // std::make_tuple("[::1%lo]", "[::1]:0"), // should be "[::1%1]:0"),
        std::make_tuple("[fe80::acd%2]:ssh", "[fe80::acd]:22") // should be "[fe80::acd%2]:22")
        // ens1 not given on all platforms
        // std::make_tuple("[fe80::ace%ens1]:ssh", "[fe80::ace]:22") // "[fe80::acd%ens1]:22")
    ));
// clang-format on


class AddrinfoMockFTestSuite : public ::testing::Test {
  protected:
    // Instantiate mocking object.
    StrictMock<umock::NetdbMock> m_netdbObj;
    // Inject the mocking object into the tested code.
    umock::Netdb netdb_injectObj = umock::Netdb(&m_netdbObj);

    // Constructor
    AddrinfoMockFTestSuite() {
        ON_CALL(m_netdbObj, getaddrinfo(_, _, _, _))
            .WillByDefault(Return(EAI_FAMILY));
    }
};


TEST(AddrinfoTestSuite, query_ipv6_addrinfo_successful) {
    CAddrinfo ai1("[2001:db8::8]");

    // Check the initialized object. This is what we have given with the
    // constructor. We get just the initialized hints.
    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai1->ai_addrlen, 0);
    EXPECT_EQ(ai1->ai_addr, nullptr);
    EXPECT_EQ(ai1->ai_canonname, nullptr);
    EXPECT_EQ(ai1->ai_next, nullptr);
    // There is no address information
    EXPECT_EQ(ai1.netaddr().str(), "");

    ASSERT_NO_THROW(ai1.init());

    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai1->ai_addrlen, 28);
    EXPECT_NE(ai1->ai_addr, nullptr);
    EXPECT_EQ(ai1->ai_canonname, nullptr);
    // EXPECT_NE(ai1->ai_next, nullptr); // Depends on available interfaces
    EXPECT_EQ(ai1.netaddr().str(), "[2001:db8::8]:0");
}

TEST(AddrinfoTestSuite, init_ipv6_addrinfo_and_port_successful) {
    CAddrinfo ai1("[2001:db8::14]", "59876");

    ASSERT_NO_THROW(ai1.init());

    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai1->ai_addrlen, 28);
    EXPECT_NE(ai1->ai_addr, nullptr);
    EXPECT_EQ(ai1->ai_canonname, nullptr);
    // EXPECT_NE(ai1->ai_next, nullptr); // Depends on available interfaces
    EXPECT_EQ(ai1.netaddr().str(), "[2001:db8::14]:59876");
}

TEST(AddrinfoTestSuite, init_ipv6_addrinfo_port_successful) {
    CAddrinfo ai1("[2001:db8::15]:59877");

    ASSERT_NO_THROW(ai1.init());

    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai1->ai_addrlen, 28);
    EXPECT_NE(ai1->ai_addr, nullptr);
    EXPECT_EQ(ai1->ai_canonname, nullptr);
    // EXPECT_NE(ai1->ai_next, nullptr); // Depends on available interfaces
    EXPECT_EQ(ai1.netaddr().str(), "[2001:db8::15]:59877");
}

TEST(AddrinfoTestSuite, query_ipv4_addrinfo_successful) {
    CAddrinfo ai1("192.168.200.201");

    // Check the initialized object. This is what we have given with the
    // constructor. We get just the initialized hints.
    EXPECT_EQ(ai1->ai_family, AF_INET);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai1->ai_addrlen, 0);
    EXPECT_EQ(ai1->ai_addr, nullptr);
    EXPECT_EQ(ai1->ai_canonname, nullptr);
    EXPECT_EQ(ai1->ai_next, nullptr);
    // There is no address information
    EXPECT_EQ(ai1.netaddr().str(), "");

    ASSERT_NO_THROW(ai1.init());

    EXPECT_EQ(ai1->ai_family, AF_INET);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai1->ai_addrlen, 16);
    EXPECT_NE(ai1->ai_addr, nullptr);
    EXPECT_EQ(ai1->ai_canonname, nullptr);
    // EXPECT_NE(ai1->ai_next, nullptr); // Depends on available interfaces
    EXPECT_EQ(ai1.netaddr().str(), "192.168.200.201:0");
}

TEST(AddrinfoTestSuite, init_ipv4_and_port_addrinfo_successful) {
    CAddrinfo ai1("192.168.200.202", "54544");

    ASSERT_NO_THROW(ai1.init());

    EXPECT_EQ(ai1->ai_family, AF_INET);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai1->ai_addrlen, 16);
    EXPECT_NE(ai1->ai_addr, nullptr);
    EXPECT_EQ(ai1->ai_canonname, nullptr);
    // EXPECT_NE(ai1->ai_next, nullptr); // Depends on available interfaces
    EXPECT_EQ(ai1.netaddr().str(), "192.168.200.202:54544");
}

TEST(AddrinfoTestSuite, init_ipv4_port_addrinfo_successful) {
    CAddrinfo ai1("192.168.200.203:54545");

    ASSERT_NO_THROW(ai1.init());

    EXPECT_EQ(ai1->ai_family, AF_INET);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai1->ai_addrlen, 16);
    EXPECT_NE(ai1->ai_addr, nullptr);
    EXPECT_EQ(ai1->ai_canonname, nullptr);
    // EXPECT_NE(ai1->ai_next, nullptr); // Depends on available interfaces
    EXPECT_EQ(ai1.netaddr().str(), "192.168.200.203:54545");
}

TEST(AddrinfoTestSuite, double_set_addrinfo_successful) {
    // If node is not empty AI_PASSIVE is ignored.
    // Flag AI_NUMERICHOST should be set if possible to avoid expensive name
    // resolution from external DNS server.

    // Test Unit with numeric port number
    CAddrinfo ai2("[2001:db8::2]", "50048", AF_INET6, SOCK_STREAM,
                  AI_PASSIVE | AI_NUMERICHOST);
    ASSERT_NO_THROW(ai2.init());

    // Returns what ::getaddrinfo() returns.
    EXPECT_EQ(ai2->ai_family, AF_INET6);
    // Returns what ::getaddrinfo() returns.
    EXPECT_EQ(ai2->ai_socktype, SOCK_STREAM);
    // Different on platforms: Ubuntu & MacOS return 6, win32 returns 0.
    // We just return that what was requested by the user.
    EXPECT_EQ(ai2->ai_protocol, 0);
    // Different on platforms: Ubuntu returns 1025, MacOS & win32 return 0.
    // We just return that what was requested by the user.
    EXPECT_EQ(ai2->ai_flags, AI_PASSIVE | AI_NUMERICHOST);
    // Returns what ::getaddrinfo() returns.
    EXPECT_EQ(ai2.netaddr().str(), "[2001:db8::2]:50048");
    // Returns what ::getaddrinfo() returns.

    // Test Unit
    // Getting the address information again is possible but not very useful.
    // Because the same node, service and hints are used the result is exactly
    // the same as before.
    int* old_res{&ai2->ai_flags};
    ASSERT_NO_THROW(ai2.init());

    EXPECT_NE(old_res, &ai2->ai_flags);
    EXPECT_EQ(ai2->ai_family, AF_INET6);
    EXPECT_EQ(ai2->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai2->ai_protocol, 0);
    EXPECT_EQ(ai2->ai_flags, AI_PASSIVE | AI_NUMERICHOST);
    EXPECT_EQ(ai2.netaddr().str(), "[2001:db8::2]:50048");
}

TEST(AddrinfoTestSuite, instantiate_not_init_numeric_host_successful) {
    // If node is not empty AI_PASSIVE is ignored.
    // Flag AI_NUMERICHOST should be set if possible to avoid expensive name
    // resolution from external DNS server.

    // Test Unit with numeric port number
    CAddrinfo ai1("[2001:db8::1]", "50050", AF_INET6, SOCK_STREAM,
                  AI_PASSIVE | AI_NUMERICHOST);

    // Check the initialized object without address information. This is what
    // we have given with the constructor. We get just the initialized hints.
    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_PASSIVE | AI_NUMERICHOST);
    EXPECT_EQ(ai1->ai_addrlen, 0);
    EXPECT_EQ(ai1->ai_addr, nullptr);
    EXPECT_EQ(ai1->ai_canonname, nullptr);
    EXPECT_EQ(ai1->ai_next, nullptr);
    // There is no address information
    EXPECT_EQ(ai1.netaddr().str(), "");
}

TEST(AddrinfoTestSuite, get_implicit_address_family) {
    // It is not needed to set the address family to AF_UNSPEC. That is used by
    // default.

    // Test Unit
    CAddrinfo ai1("[2001:db8::5]", "50051");
    // Same as
    // CAddrinfo ai1("[2001:db8::5]", "50051", AF_UNSPEC, SOCK_STREAM);
    ASSERT_NO_THROW(ai1.init());

    EXPECT_EQ(ai1->ai_family, AF_INET6); // set by syscal ::getaddrinfo
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST); // set by syscal ::getaddrinfo
    EXPECT_EQ(ai1.netaddr().str(), "[2001:db8::5]:50051");

    // Test Unit
    CAddrinfo ai2("192.168.9.10", "50096");
    ASSERT_NO_THROW(ai2.init());

    EXPECT_EQ(ai2->ai_family, AF_INET); // set by syscal ::getaddrinfo
    EXPECT_EQ(ai2->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai2->ai_protocol, 0);
    EXPECT_EQ(ai2->ai_flags, AI_NUMERICHOST); // set by syscal ::getaddrinfo
    EXPECT_EQ(ai2.netaddr().str(), "192.168.9.10:50096");

    // Test Unit, does not trigger a DNS query
    CAddrinfo ai3("localhost", "50049");
    ASSERT_NO_THROW(ai3.init());

    EXPECT_EQ(ai3->ai_family, AF_INET6); // set by syscal ::getaddrinfo
    EXPECT_EQ(ai3->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai3->ai_protocol, 0);
    EXPECT_EQ(ai3->ai_flags, 0);
    EXPECT_EQ(ai3.netaddr().str(), "[::1]:50049");
}

TEST(AddrinfoTestSuite, get_unknown_numeric_host_fails) {
    // With AI_NUMERICHOST "localhost" is unknown. Name resolving does not
    // trigger a DNS query.

    // Test Unit
    CAddrinfo ai1("localhost", "50031", AF_INET, SOCK_STREAM, AI_NUMERICHOST);
    EXPECT_THROW(ai1.init(), std::runtime_error);

    // Does not call ::getaddrinfo(), because invalid numeric IPv6 is detected
    // before.
    CAddrinfo ai2("localhost:50052", AF_INET6, SOCK_DGRAM, AI_NUMERICHOST);
    EXPECT_THROW(ai2.init(), std::runtime_error);

    CAddrinfo ai3("localhost", "50053", AF_UNSPEC, SOCK_STREAM, AI_NUMERICHOST);
    EXPECT_THROW(ai3.init(), std::runtime_error);
}

TEST(AddrinfoTestSuite, get_unknown_alphanumeric_host_fails) {
    CAddrinfo ai1("[localhost]", "50055", AF_UNSPEC, SOCK_DGRAM);
    EXPECT_THROW(ai1.init(), std::runtime_error);

    CAddrinfo ai2("[localhost]:50005", AF_UNSPEC, SOCK_DGRAM);
    EXPECT_THROW(ai2.init(), std::runtime_error);
}

TEST_F(AddrinfoMockFTestSuite, get_addrinfo_out_of_memory) {
    // Mock is_netaddr() that is called three times
    EXPECT_CALL(m_netdbObj,
                getaddrinfo(Pointee(*"localhost"), nullptr,
                            Field(&addrinfo::ai_flags, AI_NUMERICHOST), _))
        .Times(3)
        .WillRepeatedly(Return(EAI_MEMORY));
    // Mock CAddrinfo::init()
    EXPECT_CALL(m_netdbObj,
                getaddrinfo(Pointee(*"localhost"), Pointee(*"50118"),
                            Field(&addrinfo::ai_flags, AI_NUMERICSERV), _))
        .WillOnce(Return(EAI_MEMORY));

    // Test Unit
    CAddrinfo ai("localhost", "50118", AF_UNSPEC, SOCK_STREAM);
    EXPECT_THROW(ai.init(), std::invalid_argument);
}

TEST_F(AddrinfoMockFTestSuite, get_addrinfo_invalid_ipv4_address) {
    // This test triggers a DNS lookup, so I mock it.
    // Mock 'is_netaddr()' that is called three times.
    EXPECT_CALL(m_netdbObj,
                getaddrinfo(Pointee(*"192.168.88.256"), nullptr,
                            Field(&addrinfo::ai_flags, AI_NUMERICHOST), _))
        .Times(3)
        .WillRepeatedly(Return(EAI_NONAME));
    // Mock 'CAddrinfo::init()'
    EXPECT_CALL(m_netdbObj,
                getaddrinfo(Pointee(*"192.168.88.256"), Pointee(*"59866"),
                            Field(&addrinfo::ai_flags, AI_NUMERICSERV), _))
        .WillOnce(Return(EAI_NONAME));

    // Test Unit
    CAddrinfo ai("192.168.88.256:59866");
    EXPECT_THROW(ai.init(), std::runtime_error);
}

TEST_F(AddrinfoMockFTestSuite, get_addrinfo_service_dot_for_colon) {
    // This test triggers a DNS lookup, so I mock it.
    // Mock 'is_netaddr()' that is called three times.
    EXPECT_CALL(m_netdbObj,
                getaddrinfo(Pointee(*"127.0.0.1.4"), nullptr,
                            Field(&addrinfo::ai_flags, AI_NUMERICHOST), _))
        .Times(3)
        .WillRepeatedly(Return(EAI_NONAME));
    // Mock 'CAddrinfo::init()'
    EXPECT_CALL(m_netdbObj,
                getaddrinfo(Pointee(*"127.0.0.1.4"), Pointee(*"0"),
                            Field(&addrinfo::ai_flags, AI_NUMERICSERV), _))
        .WillOnce(Return(EAI_NONAME));

    // Test Unit
    CAddrinfo ai("127.0.0.1.4");
    EXPECT_THROW(ai.init(), std::runtime_error);
}

TEST(AddrinfoTestSuite, invalid_ipv6_node_address) {
    // Test Unit. Node address must be surounded with brackets.
    CAddrinfo ai1("::1", "", AF_INET6, 0, AI_NUMERICHOST);
    EXPECT_THROW(ai1.init(), std::runtime_error);

    CAddrinfo ai2("[::1", "", AF_INET6, 0, AI_NUMERICHOST);
    EXPECT_THROW(ai2.init(), std::runtime_error);

    CAddrinfo ai3("::1]", "", AF_INET6, 0, AI_NUMERICHOST);
    EXPECT_THROW(ai3.init(), std::runtime_error);
}

TEST(AddrinfoTestSuite, get_unknown_node_address) {
    // Test Unit with unspecified ipv6 address
    CAddrinfo ai3("[::]", "0", AF_INET6, 0, AI_NUMERICHOST | AI_NUMERICSERV);
    ASSERT_NO_THROW(ai3.init());

    EXPECT_EQ(ai3->ai_family, AF_INET6);
    EXPECT_EQ(ai3->ai_socktype, 0);
    EXPECT_EQ(ai3->ai_protocol, 0);
    EXPECT_EQ(ai3->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai3.netaddr().str(), "[::]:0");

    // Test Unit with unspecified ipv4 address
    CAddrinfo ai4("0.0.0.0", "0", AF_INET, 0, AI_NUMERICHOST | AI_NUMERICSERV);
    ASSERT_NO_THROW(ai4.init());

    EXPECT_EQ(ai4->ai_family, AF_INET);
    EXPECT_EQ(ai4->ai_socktype, 0);
    EXPECT_EQ(ai4->ai_protocol, 0);
    EXPECT_EQ(ai4->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai4.netaddr().str(), "0.0.0.0:0");
}

TEST(AddrinfoTestSuite, get_active_empty_node_address) {
    // An empty node name will return information about the loopback interface.
    // This is specified by syscall ::getaddrinfo().

    { // Scoped to reduce memory usage for testing with node "".
        // Test Unit for AF_UNSPEC
        CAddrinfo ai1("", "50007");
        ASSERT_NO_THROW(ai1.init());

        // Address family set by syscal ::getaddrinfo(). Maybe interface with
        // index number 1 is used and its address_family is returned?
        EXPECT_EQ(ai1->ai_family, AF_INET6);
        EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
        EXPECT_EQ(ai1->ai_protocol, 0);
        EXPECT_EQ(ai1->ai_flags, 0);
        EXPECT_THAT(ai1.netaddr().str(), "[::1]:50007");

        // Test Unit for AF_UNSPEC
        CAddrinfo ai2("", "50099", AF_UNSPEC, 0, AI_NUMERICHOST);
        ASSERT_NO_THROW(ai2.init());

        EXPECT_EQ(ai2->ai_family, AF_INET6);
        EXPECT_EQ(ai2->ai_socktype, 0);
        EXPECT_EQ(ai2->ai_protocol, 0);
        EXPECT_EQ(ai2->ai_flags, AI_NUMERICHOST);
        EXPECT_THAT(ai2.netaddr().str(), "[::1]:50099");

        // Test Unit for AF_INET6
        CAddrinfo ai3("", "50084", AF_INET6, SOCK_STREAM);
        ASSERT_NO_THROW(ai3.init());

        EXPECT_EQ(ai3->ai_family, AF_INET6);
        EXPECT_EQ(ai3->ai_socktype, SOCK_STREAM);
        EXPECT_EQ(ai3->ai_protocol, 0);
        EXPECT_EQ(ai3->ai_flags, 0);
        EXPECT_EQ(ai3.netaddr().str(), "[::1]:50084");

        // Test Unit for AF_INET6
        CAddrinfo ai4("", "50058", AF_INET6, SOCK_STREAM,
                      AI_NUMERICHOST | AI_NUMERICSERV);
        ASSERT_NO_THROW(ai4.init());

        EXPECT_EQ(ai4->ai_family, AF_INET6);
        EXPECT_EQ(ai4->ai_socktype, SOCK_STREAM);
        EXPECT_EQ(ai4->ai_protocol, 0);
        EXPECT_EQ(ai4->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
        EXPECT_EQ(ai4.netaddr().str(), "[::1]:50058");

        // Test Unit for AF_INET
        CAddrinfo ai5("", "50057", AF_INET);
        ASSERT_NO_THROW(ai5.init());

        EXPECT_EQ(ai5->ai_family, AF_INET);
        EXPECT_EQ(ai5->ai_socktype, SOCK_STREAM);
        EXPECT_EQ(ai5->ai_protocol, 0);
        EXPECT_EQ(ai5->ai_flags, 0);
        EXPECT_EQ(ai5.netaddr().str(), "127.0.0.1:50057");

        // Test Unit for AF_INET
        CAddrinfo ai6("", "50100", AF_INET, 0, AI_NUMERICHOST);
        ASSERT_NO_THROW(ai6.init());

        EXPECT_EQ(ai6->ai_family, AF_INET);
        EXPECT_EQ(ai6->ai_socktype, 0);
        EXPECT_EQ(ai6->ai_protocol, 0);
        EXPECT_EQ(ai6->ai_flags, AI_NUMERICHOST);
        EXPECT_EQ(ai6.netaddr().str(), "127.0.0.1:50100");
    }

    { // Scoped to reduce memory usage for testing with node "[]".
        // Test Unit for AF_UNSPEC
        CAddrinfo ai2("[]", "50101", AF_UNSPEC, 0, AI_NUMERICHOST);
        EXPECT_THROW(ai2.init(), std::runtime_error);

        // Test Unit AF_INET6 to be numeric, important test of special case.
        CAddrinfo ai4("[]", "50103", AF_INET6, 0, AI_NUMERICHOST);
        EXPECT_THROW(ai4.init(), std::runtime_error);

        // Test Unit AF_INET
        CAddrinfo ai6("[]", "50105", AF_INET, 0, AI_NUMERICHOST);
        EXPECT_THROW(ai6.init(), std::runtime_error);
    }
}

TEST(AddrinfoTestSuite, get_passive_node_address) {
    // To get a passive address info, node must be empty ("") or an empty
    // IPv6 address ("[]") otherwise flag AI_PASSIVE is ignored.

    { // Scoped to reduce memory usage for testing with node "".
        // Test Unit for AF_UNSPEC
        CAddrinfo ai1("", "50106", AF_UNSPEC, SOCK_STREAM, AI_PASSIVE);
        ASSERT_NO_THROW(ai1.init());

        // Ubuntu returns AF_INET, MacOS and Microsoft Windows return AF_INET6
        EXPECT_THAT(ai1->ai_family, AnyOf(AF_INET6, AF_INET));
        EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
        EXPECT_EQ(ai1->ai_protocol, 0);
        EXPECT_EQ(ai1->ai_flags, AI_PASSIVE);
        EXPECT_THAT(ai1.netaddr().str(), AnyOf("[::]:50106", "0.0.0.0:50106"));

        // Test Unit for AF_UNSPEC
        CAddrinfo ai2("", "50107", AF_UNSPEC, 0, AI_PASSIVE | AI_NUMERICHOST);
        ASSERT_NO_THROW(ai2.init());

        // Ubuntu returns AF_INET, MacOS and Microsoft Windows return AF_INET6
        EXPECT_THAT(ai2->ai_family, AnyOf(AF_INET6, AF_INET));
        EXPECT_EQ(ai2->ai_socktype, 0);
        EXPECT_EQ(ai2->ai_protocol, 0);
        EXPECT_EQ(ai2->ai_flags, AI_PASSIVE | AI_NUMERICHOST);
        EXPECT_THAT(ai2.netaddr().str(), AnyOf("[::]:50107", "0.0.0.0:50107"));

        // Test Unit for AF_INET6
        CAddrinfo ai3("", "50108", AF_INET6, 0, AI_PASSIVE);
        ASSERT_NO_THROW(ai3.init());

        EXPECT_EQ(ai3->ai_family, AF_INET6);
        EXPECT_EQ(ai3->ai_socktype, 0);
        EXPECT_EQ(ai3->ai_protocol, 0);
        EXPECT_EQ(ai3->ai_flags, AI_PASSIVE);
        EXPECT_EQ(ai3.netaddr().str(), "[::]:50108");

        // Test Unit for AF_INET6
        CAddrinfo ai4("", "50109", AF_INET6, 0, AI_PASSIVE | AI_NUMERICHOST);
        ASSERT_NO_THROW(ai4.init());

        EXPECT_EQ(ai4->ai_family, AF_INET6);
        EXPECT_EQ(ai4->ai_socktype, 0);
        EXPECT_EQ(ai4->ai_protocol, 0);
        EXPECT_EQ(ai4->ai_flags, AI_PASSIVE | AI_NUMERICHOST);
        EXPECT_EQ(ai4.netaddr().str(), "[::]:50109");

        // Test Unit for AF_INET
        CAddrinfo ai5("", "50110", AF_INET, 0, AI_PASSIVE);
        ASSERT_NO_THROW(ai5.init());

        EXPECT_EQ(ai5->ai_family, AF_INET);
        EXPECT_EQ(ai5->ai_socktype, 0);
        EXPECT_EQ(ai5->ai_protocol, 0);
        EXPECT_EQ(ai5->ai_flags, AI_PASSIVE);
        EXPECT_EQ(ai5.netaddr().str(), "0.0.0.0:50110");

        // Test Unit for AF_INET
        CAddrinfo ai6("", "50111", AF_INET, 0, AI_PASSIVE | AI_NUMERICHOST);
        ASSERT_NO_THROW(ai6.init());

        EXPECT_EQ(ai6->ai_family, AF_INET);
        EXPECT_EQ(ai6->ai_socktype, 0);
        EXPECT_EQ(ai6->ai_protocol, 0);
        EXPECT_EQ(ai6->ai_flags, AI_PASSIVE | AI_NUMERICHOST);
        // This will listen on all local network interfaces.
        EXPECT_EQ(reinterpret_cast<sockaddr_in*>(ai6->ai_addr)->sin_addr.s_addr,
                  INADDR_ANY); // or
        EXPECT_EQ(ai6.netaddr().str(), "0.0.0.0:50111");
    }

    { // Scoped to reduce memory usage for testing with node "[]".
      // "[]" is undefined and if specified as numeric address
      // (AI_NUMERICHOST flag set) it is treated as invalid. As alphanumeric
      // node name it is given to syscall ::getaddrinfo() that triggers a DNS
      // name resolution. I think that will not find it.

        // Test Unit for AF_UNSPEC
        CAddrinfo ai2("[]", "50113", AF_UNSPEC, 0, AI_PASSIVE | AI_NUMERICHOST);
        EXPECT_THROW(ai2.init(), std::runtime_error);

        // Test Unit for AF_INET6
        CAddrinfo ai4("[]", "50115", AF_INET6, 0, AI_PASSIVE | AI_NUMERICHOST);
        EXPECT_THROW(ai4.init(), std::runtime_error);

        // Test Unit for AF_INET
        // Wrong address family
        CAddrinfo ai6("[]", "50117", AF_INET, 0, AI_PASSIVE | AI_NUMERICHOST);
        EXPECT_THROW(ai6.init(), std::runtime_error);
    }

    // Test Unit
    // Using explicit the unknown netaddress should definetly return the
    // in6addr_any passive listening socket info.
    CAddrinfo ai1("[::]", "50006", AF_INET6, SOCK_DGRAM,
                  AI_PASSIVE | AI_NUMERICHOST);
    ASSERT_NO_THROW(ai1.init());

    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_DGRAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_PASSIVE | AI_NUMERICHOST);
    // This will listen on all local network interfaces.
    EXPECT_EQ(ai1.netaddr().str(), "[::]:50006");

    // Test Unit
    // Using explicit the unknown netaddress should definetly return the
    // INADDR_ANY passive listening socket info.
    CAddrinfo ai2("0.0.0.0", "50032", AF_INET, SOCK_STREAM,
                  AI_PASSIVE | AI_NUMERICHOST);
    ASSERT_NO_THROW(ai2.init());

    EXPECT_EQ(ai2->ai_family, AF_INET);
    EXPECT_EQ(ai2->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai2->ai_protocol, 0);
    EXPECT_EQ(ai2->ai_flags, AI_PASSIVE | AI_NUMERICHOST);
    // This will listen on all local network interfaces.
    EXPECT_EQ(ai2.netaddr().str(), "0.0.0.0:50032");
}

TEST(AddrinfoTestSuite, get_two_brackets_alphanum_node_address) {
    // "[]" is undefined and internal always assumed to be AI_NUMERICHOST (flag
    // set) so get it as invalid but does not trigger a DNS server call.

    // Test Unit
    CAddrinfo ai7("[]", "50098");
    EXPECT_THROW(ai7.init(), std::runtime_error);

    // Test Unit for AF_UNSPEC
    CAddrinfo ai1("[]", "50112", AF_UNSPEC, SOCK_STREAM, AI_PASSIVE);
    EXPECT_THROW(ai1.init(), std::runtime_error);
    CAddrinfo ai2("[]", "50112", AF_UNSPEC, SOCK_STREAM);
    EXPECT_THROW(ai2.init(), std::runtime_error);

    // Test Unit for AF_INET6
    CAddrinfo ai3("[]", "50114", AF_INET6, 0, AI_PASSIVE);
    EXPECT_THROW(ai3.init(), std::runtime_error);
    CAddrinfo ai4("[]", "50114", AF_INET6);
    EXPECT_THROW(ai4.init(), std::runtime_error);

    // Test Unit for AF_INET
    // This alphanumeric address is never a valid IPv4 address but it
    // invokes an expensive DNS lookup.
    CAddrinfo ai5("[]", "50116", AF_INET, 0, AI_PASSIVE);
    EXPECT_THROW(ai5.init(), std::runtime_error);
    CAddrinfo ai6("[]", "50116", AF_INET);
    EXPECT_THROW(ai6.init(), std::runtime_error);
}

TEST(AddrinfoTestSuite, get_info_loopback_interface) {
    // If node is not empty AI_PASSIVE is ignored.

    // Test Unit with string port number
    CAddrinfo ai1("[::1]", "50001", AF_INET6, SOCK_STREAM,
                  AI_PASSIVE | AI_NUMERICHOST);
    ASSERT_NO_THROW(ai1.init());

    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_PASSIVE | AI_NUMERICHOST);
    EXPECT_EQ(ai1.netaddr().str(), "[::1]:50001");

    // Test Unit
    CAddrinfo ai5("[::1]", "50085", AF_INET6, SOCK_STREAM);
    ASSERT_NO_THROW(ai5.init());

    EXPECT_EQ(ai5->ai_family, AF_INET6);
    EXPECT_EQ(ai5->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai5->ai_protocol, 0);
    EXPECT_EQ(ai5->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai5.netaddr().str(), "[::1]:50085");

    // Test Unit
    CAddrinfo ai2("127.0.0.1", "50086", AF_INET, SOCK_DGRAM);
    ASSERT_NO_THROW(ai2.init());

    EXPECT_EQ(ai2->ai_family, AF_INET);
    EXPECT_EQ(ai2->ai_socktype, SOCK_DGRAM);
    EXPECT_EQ(ai2->ai_protocol, 0);
    EXPECT_EQ(ai2->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai2.netaddr().str(), "127.0.0.1:50086");

    // Test Unit
    CAddrinfo ai3("[::1]", "50087", AF_INET6);
    ASSERT_NO_THROW(ai3.init());

    EXPECT_EQ(ai3->ai_family, AF_INET6);
    EXPECT_EQ(ai3->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai3->ai_protocol, 0);
    EXPECT_EQ(ai3->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai3.netaddr().str(), "[::1]:50087");

    // Test Unit, does not trigger a DNS query
    CAddrinfo ai4("localhost", "50088");
    ASSERT_NO_THROW(ai4.init());

    EXPECT_EQ(ai4->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai4->ai_protocol, 0);
    EXPECT_EQ(ai4->ai_flags, 0);
    EXPECT_THAT(ai4.netaddr().str(), AnyOf("[::1]:50088", "127.0.0.1:50088"));
}

TEST(AddrinfoTestSuite, empty_service) {
    // With a node but an empty service the returned port number in the address
    // structure is set to 0.

    // Test Unit
    CAddrinfo ai1("[2001:db8::8]", "", AF_INET6, SOCK_STREAM, AI_NUMERICHOST);
    ASSERT_NO_THROW(ai1.init());

    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai1.netaddr().str(), "[2001:db8::8]:0");
}

TEST(AddrinfoTestSuite, get_fails) {
    // Test Unit. Address family does not match the numeric host address.
    // There are different error messages on different platforms.
    CAddrinfo ai1("192.168.155.15", "50003", AF_INET6, SOCK_STREAM,
                  AI_NUMERICHOST);
#if defined(__GNUC__) && !defined(__clang__)
    EXPECT_THROW(ai1.init(), std::invalid_argument);
#else
    EXPECT_THROW(ai1.init(), std::runtime_error);
#endif
}

} // namespace utest


int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include <utest/utest_main.inc>
    return gtest_return_code; // managed in gtest_main.inc
}
