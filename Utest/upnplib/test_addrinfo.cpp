// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: r024-07-25

// I test different address infos that we get from system function
// ::getaddrinfo(). This function does not ensure always the same order of same
// returned address infos in subsequent calls. So I have to test independent
// from the sequence of the test calls. This is why you find following several
// algorithm with a do{} while() loop to ensure this.

// Include source code for testing. So we have also direct access to static
// functions which need to be tested.
#include <Upnplib/src/net/addrinfo.cpp>

#include <upnplib/socket.hpp>
#include <utest/utest.hpp>
#include <umock/netdb_mock.hpp>

namespace utest {

using ::testing::_;
using ::testing::AnyOf;
using ::testing::Conditional;
using ::testing::DoAll;
using ::testing::Field;
using ::testing::Pointee;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::StrictMock;

using ::upnplib::CAddrinfo;
using ::upnplib::SSockaddr;
using ::upnplib::to_netaddrp;

// Alternative proof of runtime select of the platform instead of conditional
// compiling.
enum class Co { // Possible compiler
    unknown,
    gnuc,
    clang,
    msc
};
// Current used compiler
#if defined(__GNUC__) && !defined(__clang__)
Co co = Co::gnuc;
#elif defined(__clang__)
Co co = Co::clang;
#elif defined(_MSC_VER)
Co co = Co::msc;
#else
Co co = Co::unknown;
#endif


class NetaddrAssignTest
    : public ::testing::TestWithParam<
          std::tuple<const std::string, const std::string>> {};

TEST_P(NetaddrAssignTest, netaddress_assign) {
    // Get parameter
    std::tuple params = GetParam();

    // Test Unit
    CAddrinfo aiObj(std::get<0>(params));
    if (std::get<1>(params) == "") {
        EXPECT_THROW(aiObj.load(), std::runtime_error);
    } else {
        aiObj.load();
        EXPECT_EQ(aiObj->ai_next, nullptr);
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
        std::make_tuple(".", ""),
        std::make_tuple(".:", ""),
        std::make_tuple(":.", ""),
        std::make_tuple("::", ""),
        std::make_tuple(":::", ""),
        std::make_tuple("[::", ""),
        std::make_tuple("::]", ""),
        // std::make_tuple("[::1", ""), // tested later
        // std::make_tuple("::1]", ""), // tested later
        // std::make_tuple("", "[::1]:0"), // multiple results, tested later
        // std::make_tuple(":50987", "[::1]:50987"), // multiple results, tested later
        /*20*/ std::make_tuple("[::1]", "[::1]:0"),
        std::make_tuple("[::1]:", "[::1]:0"),
        std::make_tuple("[::1]:0", "[::1]:0"),
        // std::make_tuple("[::1].4", ""), // dot for colon, takes long time, mocked later
        std::make_tuple("127.0.0.1", "127.0.0.1:0"),
        std::make_tuple("127.0.0.1:", "127.0.0.1:0"),
        std::make_tuple("127.0.0.1:0", "127.0.0.1:0"),
        // std::make_tuple("127.0.0.1.5", ""), // dot for colon, takes long time, mocked later
        std::make_tuple("[2001:db8::43]:", "[2001:db8::43]:0"),
        std::make_tuple("2001:db8::41:59897", ""), // no brackets
        std::make_tuple("[2001:db8::fg]", ""),
        std::make_tuple("[2001:db8::fg]:59877", ""),
        // std::make_tuple("[2001:db8::42]:65535", "[2001:db8::42]:65535"), // tested later
        /*30*/ std::make_tuple("[2001:db8::51]:65536", ""), // invalid port
        std::make_tuple("[2001:db8::52]:9999999999", ""), // invalid port
        std::make_tuple("[2001:db8::52::53]", ""), // double double colon
        std::make_tuple("[2001:db8::52::54]:65535", ""), // double double colon
        std::make_tuple("[12.168.88.95]", ""), // IPv4 address with brackets
        std::make_tuple("[12.168.88.96]:", ""),
        std::make_tuple("[12.168.88.97]:9876", ""),
        // std::make_tuple("192.168.88.98:59876", "192.168.88.98:59876"), // tested later
        std::make_tuple("192.168.88.99:65537", ""), // invalid port
        // std::make_tuple("192.168.88.256:59866", ""), // tested later
        // std::make_tuple("192.168.88.91", "192.168.88.91:0"), // tested later
        // std::make_tuple("garbage:49493", ""), // triggers DNS lookup
        std::make_tuple("[garbage]:49494", ""),
        std::make_tuple("[2001:db8::44]:https", "[2001:db8::44]:443"),
        // std::make_tuple("[2001:db8::44]:httpx", ""), // takes long time, mocked later
        /*40*/ std::make_tuple("192.168.88.98:http", "192.168.88.98:80"),
        std::make_tuple("192.168.71.73%1:44:https", ""),
        std::make_tuple("192.168.71.74%lo:44:https", "")
        // std::make_tuple("192.168.88.98:httpy", ""), // takes long time, mocked later
        // std::make_tuple("[fe80::5054:ff:fe7f:c021]", "[fe80::5054:ff:fe7f:c021%2]:0"), // fails, not portable
        // std::make_tuple("[fe80::5054:ff:fe7f:c021%ens1]", "[fe80::5054:ff:fe7f:c021%2]:0"), // succeeds, not portable
        // std::make_tuple("[2003:d5:270b:9000:5054:ff:fe7f:c021%3]", "[2003:d5:270b:9000:5054:ff:fe7f:c021%3]:0"), // succeeds, not porable
        // std::make_tuple("[2003:d5:270b:9000:5054:ff:fe7f:c021%2]", "[2003:d5:270b:9000:5054:ff:fe7f:c021%2]:0"), // succeeds, not porable
        // std::make_tuple("[2003:d5:270b:9000:5054:ff:fe7f:c021%ens1]", "[2003:d5:270b:9000:5054:ff:fe7f:c021%2]:0") // fails, not porable
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
    CAddrinfo ai1("[2001:db8::8%2]");

    // Check the initialized object. This is what we have given with the
    // constructor. We get just the initialized hints.
    EXPECT_EQ(ai1->ai_family, AF_UNSPEC);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, 0);
    EXPECT_EQ(ai1->ai_addrlen, 0);
    EXPECT_EQ(ai1->ai_addr, nullptr);
    EXPECT_EQ(ai1->ai_canonname, nullptr);
    // There is no address information
    EXPECT_EQ(ai1.netaddr().str(), "");
    EXPECT_EQ(ai1->ai_next, nullptr);

    ASSERT_NO_THROW(ai1.load());

    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai1->ai_addrlen, 28);
    EXPECT_NE(ai1->ai_addr, nullptr);
    EXPECT_EQ(ai1->ai_canonname, nullptr);
    EXPECT_EQ(ai1.netaddr().str(), "[2001:db8::8%2]:0");
    EXPECT_EQ(ai1->ai_next, nullptr);
}

TEST_F(AddrinfoMockFTestSuite, query_ipv6_addrinfo_successful) {
    SSockaddr saddr;
    saddr = "[2001:db8::9]";

    ::addrinfo res;
    res.ai_flags = AI_NUMERICHOST;
    res.ai_family = AF_INET6;
    res.ai_socktype = SOCK_STREAM;
    res.ai_protocol = 0;
    res.ai_addrlen = sizeof(saddr.sin6);
    res.ai_addr = &saddr.sa;
    res.ai_canonname = nullptr;
    res.ai_next = nullptr;

    // Mock 'is_netaddr()'.
    EXPECT_CALL(m_netdbObj,
                getaddrinfo(Pointee(*"2001:db8::9"), nullptr,
                            Field(&addrinfo::ai_flags, AI_NUMERICHOST), _))
        .WillOnce(DoAll(SetArgPointee<3>(&res), Return(0)));
    // Mock 'CAddrinfo::load()'
    EXPECT_CALL(m_netdbObj, getaddrinfo(Pointee(*"2001:db8::9"), Pointee(*"0"),
                                        Field(&addrinfo::ai_flags,
                                              AI_NUMERICHOST | AI_NUMERICSERV),
                                        _))
        .WillOnce(DoAll(SetArgPointee<3>(&res), Return(0)));
    // Mock 'freeaddrinfo()'
    EXPECT_CALL(m_netdbObj, freeaddrinfo(&res)).Times(2);

    // Test Unit
    CAddrinfo ai("[2001:db8::9]");
    ASSERT_NO_THROW(ai.load());
}

TEST_F(AddrinfoMockFTestSuite, query_addrinfo_url_with_service_successful) {
    ::addrinfo res1;
    res1.ai_family = AF_UNSPEC;

    ::addrinfo res2;
    res2.ai_family = AF_INET6;

    SSockaddr saddr;
    saddr = "[2001:db8::10]:443";

    ::addrinfo res3;
    res3.ai_flags = 0;
    res3.ai_family = AF_UNSPEC;
    res3.ai_socktype = SOCK_STREAM;
    res3.ai_protocol = 0;
    res3.ai_addrlen = sizeof(saddr.sin6);
    res3.ai_addr = &saddr.sa;
    res3.ai_canonname = nullptr;
    res3.ai_next = nullptr;

    // Mock 'is_netaddr()'.
    EXPECT_CALL(m_netdbObj,
                getaddrinfo(Pointee(*"www.example.com"), nullptr,
                            Field(&addrinfo::ai_flags, AI_NUMERICHOST), _))
        .WillOnce(DoAll(SetArgPointee<3>(&res1), Return(EAI_NONAME)))
        .WillOnce(DoAll(SetArgPointee<3>(&res2), Return(EAI_NONAME)));
    // Mock 'CAddrinfo::load()'
    EXPECT_CALL(m_netdbObj,
                getaddrinfo(Pointee(*"www.excample.com"), Pointee(*"https"),
                            AllOf(Field(&addrinfo::ai_family, AF_UNSPEC),
                                  Field(&addrinfo::ai_flags, 0)),
                            _))
        .WillOnce(DoAll(SetArgPointee<3>(&res3), Return(0)));
    // Mock 'freeaddrinfo()'
    EXPECT_CALL(m_netdbObj, freeaddrinfo(_)).Times(3);

    // Test Unit
    CAddrinfo ai("www.example.com:https");
    ASSERT_NO_THROW(ai.load());

    EXPECT_EQ(ai->ai_family, AF_UNSPEC);
    EXPECT_EQ(ai->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai->ai_protocol, 0);
    EXPECT_EQ(ai->ai_flags, 0);
    EXPECT_EQ(ai->ai_addrlen, 28);
    EXPECT_EQ(ai->ai_addr, &saddr.sa);
    EXPECT_EQ(ai->ai_canonname, nullptr);
    EXPECT_EQ(ai->ai_next, nullptr);
    EXPECT_EQ(ai.netaddr().str(), "[2001:db8::10]:443");
}

TEST(AddrinfoTestSuite, load_ipv6_addrinfo_and_port_successful) {
    CAddrinfo ai1("[2001:db8::14]", "https");

    ASSERT_NO_THROW(ai1.load());

    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai1->ai_addrlen, 28);
    EXPECT_NE(ai1->ai_addr, nullptr);
    EXPECT_EQ(ai1->ai_canonname, nullptr);
    EXPECT_EQ(ai1->ai_next, nullptr);
    EXPECT_EQ(ai1.netaddr().str(), "[2001:db8::14]:443");
}

TEST(AddrinfoTestSuite, load_ipv6_addrinfo_with_port_successful) {
    CAddrinfo ai1("[2001:db8::15]:59877");

    ASSERT_NO_THROW(ai1.load());

    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai1->ai_addrlen, 28);
    EXPECT_NE(ai1->ai_addr, nullptr);
    EXPECT_EQ(ai1->ai_canonname, nullptr);
    EXPECT_EQ(ai1->ai_next, nullptr);
    EXPECT_EQ(ai1.netaddr().str(), "[2001:db8::15]:59877");
}

TEST(AddrinfoTestSuite, query_ipv4_addrinfo_successful) {
    CAddrinfo ai1("192.168.200.201");

    // Check the initialized object. This is what we have given with the
    // constructor. We get just the initialized hints.
    EXPECT_EQ(ai1->ai_family, AF_UNSPEC);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, 0);
    EXPECT_EQ(ai1->ai_addrlen, 0);
    EXPECT_EQ(ai1->ai_addr, nullptr);
    EXPECT_EQ(ai1->ai_canonname, nullptr);
    EXPECT_EQ(ai1->ai_next, nullptr);
    // There is no address information
    EXPECT_EQ(ai1.netaddr().str(), "");

    ASSERT_NO_THROW(ai1.load());

    EXPECT_EQ(ai1->ai_family, AF_INET);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai1->ai_addrlen, 16);
    EXPECT_NE(ai1->ai_addr, nullptr);
    EXPECT_EQ(ai1->ai_canonname, nullptr);
    EXPECT_EQ(ai1->ai_next, nullptr);
    EXPECT_EQ(ai1.netaddr().str(), "192.168.200.201:0");
}

TEST(AddrinfoTestSuite, load_ipv4_addrinfo_and_port_successful) {
    CAddrinfo ai1("192.168.200.202", "54544");

    ASSERT_NO_THROW(ai1.load());

    EXPECT_EQ(ai1->ai_family, AF_INET);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai1->ai_addrlen, 16);
    EXPECT_NE(ai1->ai_addr, nullptr);
    EXPECT_EQ(ai1->ai_canonname, nullptr);
    EXPECT_EQ(ai1->ai_next, nullptr);
    EXPECT_EQ(ai1.netaddr().str(), "192.168.200.202:54544");
}

TEST(AddrinfoTestSuite, load_ipv4_addrinfo_with_port_successful) {
    CAddrinfo ai1("192.168.200.203:http");

    ASSERT_NO_THROW(ai1.load());

    EXPECT_EQ(ai1->ai_family, AF_INET);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai1->ai_addrlen, 16);
    EXPECT_NE(ai1->ai_addr, nullptr);
    EXPECT_EQ(ai1->ai_canonname, nullptr);
    EXPECT_EQ(ai1->ai_next, nullptr);
    EXPECT_EQ(ai1.netaddr().str(), "192.168.200.203:80");
}

TEST(AddrinfoTestSuite, double_set_addrinfo_successful) {
    // If node is not empty AI_PASSIVE is ignored.
    // Flag AI_NUMERICHOST should be set if possible to avoid expensive name
    // resolution from external DNS server.

    // Test Unit with numeric port number
    CAddrinfo ai2("[2001:db8::2]", "50048", AF_INET6, SOCK_STREAM,
                  AI_PASSIVE | AI_NUMERICHOST);
    ASSERT_NO_THROW(ai2.load());

    // Returns what ::getaddrinfo() returns.
    EXPECT_EQ(ai2->ai_family, AF_INET6);
    // Returns what ::getaddrinfo() returns.
    EXPECT_EQ(ai2->ai_socktype, SOCK_STREAM);
    // Different on platforms: Ubuntu & MacOS return 6, win32 returns 0.
    // We just return that what was requested by the user.
    EXPECT_EQ(ai2->ai_protocol, 0);
    // Different on platforms: Ubuntu returns 1025, MacOS & win32 return 0.
    // We just return that what was requested by the user.
    EXPECT_EQ(ai2->ai_flags, AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);
    // Returns what ::getaddrinfo() returns.
    EXPECT_EQ(ai2.netaddr().str(), "[2001:db8::2]:50048");
    // Returns what ::getaddrinfo() returns.
    EXPECT_EQ(ai2->ai_next, nullptr);

    // Test Unit
    // Getting the address information again is possible but not very useful.
    // Because the same node, service and hints are used the result is exactly
    // the same as before.
    int* old_res{&ai2->ai_flags};
    ASSERT_NO_THROW(ai2.load());

    EXPECT_NE(old_res, &ai2->ai_flags);
    EXPECT_EQ(ai2->ai_family, AF_INET6);
    EXPECT_EQ(ai2->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai2->ai_protocol, 0);
    EXPECT_EQ(ai2->ai_flags, AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai2.netaddr().str(), "[2001:db8::2]:50048");
    EXPECT_EQ(ai2->ai_next, nullptr);
}

TEST(AddrinfoTestSuite, instantiate_not_load_numeric_host_successful) {
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
    ASSERT_NO_THROW(ai1.load());

    EXPECT_EQ(ai1->ai_family, AF_INET6); // set by syscal ::getaddrinfo
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai1->ai_next, nullptr);
    EXPECT_EQ(ai1.netaddr().str(), "[2001:db8::5]:50051");

    // Test Unit
    CAddrinfo ai2("192.168.9.10", "50096");
    ASSERT_NO_THROW(ai2.load());

    EXPECT_EQ(ai2->ai_family, AF_INET); // set by syscal ::getaddrinfo
    EXPECT_EQ(ai2->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai2->ai_protocol, 0);
    EXPECT_EQ(ai2->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai2->ai_next, nullptr);
    EXPECT_EQ(ai2.netaddr().str(), "192.168.9.10:50096");
}

TEST(AddrinfoTestSuite, get_unknown_numeric_host_fails) {
    // With AI_NUMERICHOST "localhost" is unknown. Name resolving does not
    // trigger a DNS query.

    // Test Unit
    CAddrinfo ai1("localhost", "50031", AF_INET, SOCK_STREAM, AI_NUMERICHOST);
    EXPECT_THROW(ai1.load(), std::runtime_error);

    // Does not call ::getaddrinfo(), because invalid numeric IPv6 is detected
    // before.
    CAddrinfo ai2("localhost:50052", AF_INET6, SOCK_DGRAM, AI_NUMERICHOST);
    EXPECT_THROW(ai2.load(), std::runtime_error);

    CAddrinfo ai3("localhost", "50053", AF_UNSPEC, SOCK_STREAM, AI_NUMERICHOST);
    EXPECT_THROW(ai3.load(), std::runtime_error);
}

TEST(AddrinfoTestSuite, get_unknown_alphanumeric_host_fails) {
    CAddrinfo ai1("[localhost]", "50055", AF_UNSPEC, SOCK_DGRAM);
    EXPECT_THROW(ai1.load(), std::runtime_error);

    CAddrinfo ai2("[localhost]:50005", AF_UNSPEC, SOCK_DGRAM);
    EXPECT_THROW(ai2.load(), std::runtime_error);
}

TEST_F(AddrinfoMockFTestSuite, get_addrinfo_out_of_memory) {
    EXPECT_CALL(m_netdbObj,
                getaddrinfo(Pointee(*"localhost"), nullptr,
                            Field(&addrinfo::ai_flags, AI_NUMERICHOST), _))
        .Times(2)
        .WillRepeatedly(Return(EAI_MEMORY));
    // Mock CAddrinfo::load()
    EXPECT_CALL(m_netdbObj,
                getaddrinfo(Pointee(*"localhost"), Pointee(*"50118"),
                            Field(&addrinfo::ai_flags, AI_NUMERICSERV), _))
        .WillOnce(Return(EAI_MEMORY));
    // Mock 'freeaddrinfo()'
    EXPECT_CALL(m_netdbObj, freeaddrinfo(_)).Times(2);

    // Test Unit
    CAddrinfo ai("localhost", "50118", AF_UNSPEC, SOCK_STREAM);
    EXPECT_THROW(ai.load(), std::invalid_argument);
}

TEST_F(AddrinfoMockFTestSuite, get_addrinfo_invalid_ipv4_address) {
    // This test triggers a DNS lookup, so I mock it.
    // Mock 'is_netaddr()' that is called two times.
    EXPECT_CALL(m_netdbObj,
                getaddrinfo(Pointee(*"192.168.88.256"), nullptr,
                            Field(&addrinfo::ai_flags, AI_NUMERICHOST), _))
        .Times(2)
        .WillRepeatedly(Return(EAI_NONAME));
    // Mock 'CAddrinfo::load()'
    EXPECT_CALL(m_netdbObj,
                getaddrinfo(Pointee(*"192.168.88.256"), Pointee(*"59866"),
                            Field(&addrinfo::ai_flags, AI_NUMERICSERV), _))
        .WillOnce(Return(EAI_NONAME));
    // Mock 'freeaddrinfo()'
    EXPECT_CALL(m_netdbObj, freeaddrinfo(_)).Times(2);

    // Test Unit
    CAddrinfo ai("192.168.88.256:59866");
    EXPECT_THROW(ai.load(), std::runtime_error);
}

TEST_F(AddrinfoMockFTestSuite, get_addrinfo_ipv6_service_dot_for_colon) {
    // Looking for mistaken service tooks long time, so I mock it.
    ::addrinfo res{};
    res.ai_family = AF_INET6;

    // Mock 'is_netaddr()'.
    EXPECT_CALL(m_netdbObj,
                getaddrinfo(Pointee(*"::1"), nullptr,
                            Field(&addrinfo::ai_flags, AI_NUMERICHOST), _))
        .WillOnce(DoAll(SetArgPointee<3>(&res), Return(0)));
    // Mock 'CAddrinfo::load()'
    EXPECT_CALL(m_netdbObj,
                getaddrinfo(Pointee(*"::1"), Pointee(*".4"),
                            Field(&addrinfo::ai_flags, AI_NUMERICHOST), _))
        .WillOnce(Return(EAI_NONAME));
    // Mock 'freeaddrinfo()'
    EXPECT_CALL(m_netdbObj, freeaddrinfo(&res)).Times(1);

    // Test Unit
    CAddrinfo ai("[::1].4");
    EXPECT_THROW(ai.load(), std::runtime_error);
}

TEST_F(AddrinfoMockFTestSuite, get_addrinfo_ipv4_service_dot_for_colon) {
    // Looking for mistaken service tooks long time, so I mock it.
    ::addrinfo res{};

    // Mock 'is_netaddr()'.
    EXPECT_CALL(m_netdbObj,
                getaddrinfo(Pointee(*"127.0.0.1.5"), nullptr,
                            Field(&addrinfo::ai_flags, AI_NUMERICHOST), _))
        .Times(2)
        .WillRepeatedly(DoAll(SetArgPointee<3>(&res), Return(EAI_NONAME)));
    // Mock 'CAddrinfo::load()'
    EXPECT_CALL(m_netdbObj,
                getaddrinfo(Pointee(*"127.0.0.1.5"), Pointee(*"0"),
                            Field(&addrinfo::ai_flags, AI_NUMERICSERV), _))
        .WillOnce(Return(EAI_NONAME));
    // Mock 'freeaddrinfo()'
    EXPECT_CALL(m_netdbObj, freeaddrinfo(&res)).Times(2);

    // Test Unit
    CAddrinfo ai("127.0.0.1.5");
    EXPECT_THROW(ai.load(), std::runtime_error);
}

TEST_F(AddrinfoMockFTestSuite, get_addrinfo_ipv6_service_undefined_alpha_name) {
    // Looking for undefined service tooks long time, so I mock it.
    ::addrinfo res{};
    res.ai_family = AF_INET6;

    // Mock 'is_netaddr()'.
    EXPECT_CALL(m_netdbObj,
                getaddrinfo(Pointee(*"2001:db8::44"), nullptr,
                            Field(&addrinfo::ai_flags, AI_NUMERICHOST), _))
        .WillOnce(DoAll(SetArgPointee<3>(&res), Return(0)));
    // Mock 'CAddrinfo::load()'
    EXPECT_CALL(m_netdbObj,
                getaddrinfo(Pointee(*"2001:db8::44"), Pointee(*"httpx"),
                            Field(&addrinfo::ai_flags, AI_NUMERICHOST), _))
        .WillOnce(Return(EAI_SERVICE));
    // Mock 'freeaddrinfo()'
    EXPECT_CALL(m_netdbObj, freeaddrinfo(&res)).Times(1);

    // Test Unit
    CAddrinfo ai("[2001:db8::44]:httpx");
    EXPECT_THROW(ai.load(), std::runtime_error);
}

TEST_F(AddrinfoMockFTestSuite, get_addrinfo_ipv4_service_undefined_alpha_name) {
    // Looking for undefined service tooks long time, so I mock it.
    ::addrinfo res{};
    res.ai_family = AF_INET;

    // Mock 'is_netaddr()'.
    EXPECT_CALL(m_netdbObj,
                getaddrinfo(Pointee(*"192.168.88.98"), nullptr,
                            Field(&addrinfo::ai_flags, AI_NUMERICHOST), _))
        .WillOnce(DoAll(SetArgPointee<3>(&res), Return(0)));
    // Mock 'CAddrinfo::load()'
    EXPECT_CALL(m_netdbObj,
                getaddrinfo(Pointee(*"192.168.88.98"), Pointee(*"httpy"),
                            Field(&addrinfo::ai_flags, AI_NUMERICHOST), _))
        .WillOnce(Return(EAI_SERVICE));
    // Mock 'freeaddrinfo()'
    EXPECT_CALL(m_netdbObj, freeaddrinfo(&res)).Times(1);

    // Test Unit
    CAddrinfo ai("192.168.88.98:httpy");
    EXPECT_THROW(ai.load(), std::runtime_error);
}

TEST(AddrinfoTestSuite, invalid_ipv6_node_address) {
    // Test Unit. Node address must be surounded with brackets.
    CAddrinfo ai1("::1", "", AF_INET6, 0, AI_NUMERICHOST);
    EXPECT_THROW(ai1.load(), std::runtime_error);

    CAddrinfo ai2("[::1", "", AF_INET6, 0, AI_NUMERICHOST);
    EXPECT_THROW(ai2.load(), std::runtime_error);

    CAddrinfo ai3("::1]", "", AF_INET6, 0, AI_NUMERICHOST);
    EXPECT_THROW(ai3.load(), std::runtime_error);
}

TEST(AddrinfoTestSuite, get_unknown_ipv6_node_address) {
    CAddrinfo ai3("[::]", "0", AF_INET6, 0, AI_NUMERICHOST | AI_NUMERICSERV);

    EXPECT_EQ(ai3->ai_family, AF_INET6);
    EXPECT_EQ(ai3->ai_socktype, 0);
    EXPECT_EQ(ai3->ai_protocol, 0);
    EXPECT_EQ(ai3->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai3->ai_addrlen, 0);
    EXPECT_EQ(ai3->ai_addr, nullptr);
    EXPECT_EQ(ai3->ai_canonname, nullptr);
    EXPECT_EQ(ai3->ai_next, nullptr);
    EXPECT_EQ(ai3.netaddr().str(), "");

    ASSERT_NO_THROW(ai3.load());

    bool double_res1{false};
    [[maybe_unused]] bool double_res2{false};
    [[maybe_unused]] bool double_res3{false};
    do {
        EXPECT_EQ(ai3->ai_protocol, 0);
        EXPECT_EQ(ai3->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
        EXPECT_NE(ai3->ai_addr, nullptr); // not equal nullptr
        EXPECT_EQ(ai3->ai_canonname, nullptr);
#if !defined(_MSC_VER)
        if (ai3->ai_family == AF_INET6 && ai3->ai_socktype == SOCK_STREAM) {
            ASSERT_FALSE(double_res1);
            double_res1 = true;
            EXPECT_EQ(ai3->ai_addrlen, 28);
            EXPECT_EQ(ai3.netaddr().str(), "[::]:0");
        } else if (ai3->ai_family == AF_INET6 &&
                   ai3->ai_socktype == SOCK_DGRAM) {
            ASSERT_FALSE(double_res2);
            double_res2 = true;
            EXPECT_EQ(ai3->ai_addrlen, 28);
            EXPECT_EQ(ai3.netaddr().str(), "[::]:0");
#if !defined(__APPLE__)
        } else if (ai3->ai_family == AF_INET6 && ai3->ai_socktype == SOCK_RAW) {
            ASSERT_FALSE(double_res3);
            double_res3 = true;
            EXPECT_EQ(ai3->ai_addrlen, 28);
            EXPECT_EQ(ai3.netaddr().str(), "[::]:0");
#endif
#else // _MSC_VER
        if (ai3->ai_family == AF_INET6 && ai3->ai_socktype == 0) {
            ASSERT_FALSE(double_res1);
            double_res1 = true;
            EXPECT_EQ(ai3->ai_addrlen, 28);
            EXPECT_EQ(ai3.netaddr().str(), "[::]:0");
#endif
        } else {
            GTEST_FAIL()
                << "  Unexpected address information: address family = "
                << ai3->ai_family << ", socket type = " << ai3->ai_socktype
                << "\n";
        }
    } while (ai3.get_next());

    // Call another one for testing.
    EXPECT_FALSE(ai3.get_next());
}

TEST(AddrinfoTestSuite, get_unknown_ipv4_node_address) {
    CAddrinfo ai4("0.0.0.0", "0", AF_INET, 0, AI_NUMERICHOST | AI_NUMERICSERV);

    EXPECT_EQ(ai4->ai_family, AF_INET);
    EXPECT_EQ(ai4->ai_socktype, 0);
    EXPECT_EQ(ai4->ai_protocol, 0);
    EXPECT_EQ(ai4->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai4->ai_addrlen, 0);
    EXPECT_EQ(ai4->ai_addr, nullptr);
    EXPECT_EQ(ai4->ai_canonname, nullptr);
    EXPECT_EQ(ai4->ai_next, nullptr);
    EXPECT_EQ(ai4.netaddr().str(), "");

    ASSERT_NO_THROW(ai4.load());

    bool double_res1{false};
    [[maybe_unused]] bool double_res2{false};
    [[maybe_unused]] bool double_res3{false};

    do {
        EXPECT_EQ(ai4->ai_protocol, 0);
        EXPECT_EQ(ai4->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
        EXPECT_NE(ai4->ai_addr, nullptr); // not equal nullptr
        EXPECT_EQ(ai4->ai_canonname, nullptr);
#if !defined(_MSC_VER)
        if (ai4->ai_family == AF_INET && ai4->ai_socktype == SOCK_STREAM) {
            ASSERT_FALSE(double_res1);
            double_res1 = true;
            EXPECT_EQ(ai4->ai_addrlen, 16);
            EXPECT_EQ(ai4.netaddr().str(), "0.0.0.0:0");
        } else if (ai4->ai_family == AF_INET &&
                   ai4->ai_socktype == SOCK_DGRAM) {
            ASSERT_FALSE(double_res2);
            double_res2 = true;
            EXPECT_EQ(ai4->ai_addrlen, 16);
            EXPECT_EQ(ai4.netaddr().str(), "0.0.0.0:0");
#if !defined(__APPLE__)
        } else if (ai4->ai_family == AF_INET && ai4->ai_socktype == SOCK_RAW) {
            ASSERT_FALSE(double_res3);
            double_res3 = true;
            EXPECT_EQ(ai4->ai_addrlen, 16);
            EXPECT_EQ(ai4.netaddr().str(), "0.0.0.0:0");
#endif
#else // _MSC_VER
        if (ai4->ai_family == AF_INET && ai4->ai_socktype == 0) {
            ASSERT_FALSE(double_res1);
            double_res1 = true;
            EXPECT_EQ(ai4->ai_addrlen, 16);
            EXPECT_EQ(ai4.netaddr().str(), "0.0.0.0:0");
#endif
        } else {
            GTEST_FAIL()
                << "  Unexpected address information: address family = "
                << ai4->ai_family << ", socket type = " << ai4->ai_socktype
                << "\n";
        }
    } while (ai4.get_next());

    // Call another one for testing.
    EXPECT_FALSE(ai4.get_next());
}

TEST(AddrinfoTestSuite, get_active_empty_node_address) {
    // An empty node name will return information about the loopback interface.
    // This is specified by syscall ::getaddrinfo().

    { // Scoped to reduce memory usage for testing with node "".
        // Test Unit for AF_UNSPEC
        CAddrinfo ai1("", "50007");
        ASSERT_NO_THROW(ai1.load());

        // Address family set by syscal ::getaddrinfo().
        // There should be two results
        bool double_res1{false};
        bool double_res2{false};
        do {
            EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
            EXPECT_EQ(ai1->ai_protocol, 0);
            EXPECT_EQ(ai1->ai_flags, AI_NUMERICSERV);
            EXPECT_NE(ai1->ai_addr, nullptr); // not equal nullptr
            EXPECT_EQ(ai1->ai_canonname, nullptr);
            if (ai1->ai_family == AF_INET6) {
                ASSERT_FALSE(double_res1);
                double_res1 = true;
                EXPECT_EQ(ai1->ai_addrlen, 28);
                EXPECT_EQ(ai1.netaddr().str(), "[::1]:50007");
            } else if (ai1->ai_family == AF_INET) {
                ASSERT_FALSE(double_res2);
                double_res2 = true;
                EXPECT_EQ(ai1->ai_addrlen, 16);
                EXPECT_EQ(ai1.netaddr().str(), "127.0.0.1:50007");
            } else {
                GTEST_FAIL()
                    << "  Unexpected address information: address family = "
                    << ai1->ai_family << ", socket type = " << ai1->ai_socktype
                    << "\n";
            }
        } while (ai1.get_next());

        // Test Unit for AF_INET6
        CAddrinfo ai3("", "50084", AF_INET6, SOCK_STREAM);
        ASSERT_NO_THROW(ai3.load());

        EXPECT_EQ(ai3->ai_family, AF_INET6);
        EXPECT_EQ(ai3->ai_socktype, SOCK_STREAM);
        EXPECT_EQ(ai3->ai_protocol, 0);
        EXPECT_EQ(ai3->ai_flags, AI_NUMERICSERV);
        EXPECT_EQ(ai3.netaddr().str(), "[::1]:50084");
        EXPECT_EQ(ai3->ai_next, nullptr);

        // Test Unit for AF_INET6
        CAddrinfo ai4("", "50058", AF_INET6, SOCK_STREAM,
                      AI_NUMERICHOST | AI_NUMERICSERV);
        ASSERT_NO_THROW(ai4.load());

        EXPECT_EQ(ai4->ai_family, AF_INET6);
        EXPECT_EQ(ai4->ai_socktype, SOCK_STREAM);
        EXPECT_EQ(ai4->ai_protocol, 0);
        EXPECT_EQ(ai4->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
        EXPECT_EQ(ai4.netaddr().str(), "[::1]:50058");
        EXPECT_EQ(ai4->ai_next, nullptr);

        // Test Unit for AF_INET
        CAddrinfo ai5("", "50057", AF_INET);
        ASSERT_NO_THROW(ai5.load());

        EXPECT_EQ(ai5->ai_family, AF_INET);
        EXPECT_EQ(ai5->ai_socktype, SOCK_STREAM);
        EXPECT_EQ(ai5->ai_protocol, 0);
        EXPECT_EQ(ai5->ai_flags, AI_NUMERICSERV);
        EXPECT_EQ(ai5.netaddr().str(), "127.0.0.1:50057");
        EXPECT_EQ(ai5->ai_next, nullptr);
    }

    { // Scoped to reduce memory usage for testing with node "[]".
        // Test Unit for AF_UNSPEC
        CAddrinfo ai2("[]", "50101", AF_UNSPEC, 0, AI_NUMERICHOST);
        EXPECT_THROW(ai2.load(), std::runtime_error);

        // Test Unit AF_INET6 to be numeric, important test of special case.
        CAddrinfo ai4("[]", "50103", AF_INET6, 0, AI_NUMERICHOST);
        EXPECT_THROW(ai4.load(), std::runtime_error);

        // Test Unit AF_INET
        CAddrinfo ai6("[]", "50105", AF_INET, 0, AI_NUMERICHOST);
        EXPECT_THROW(ai6.load(), std::runtime_error);
    }
}

#if !defined(__APPLE__) && !defined(_MSC_VER)
TEST(AddrinfoTestSuite, get_active_empty_node_socktype_0) {
    // Test Unit for AF_INET
    CAddrinfo ai6("", "50100", AF_INET, 0, AI_NUMERICHOST);
    ASSERT_NO_THROW(ai6.load());

    EXPECT_EQ(ai6->ai_family, AF_INET);
    // ai_socktype specifies  the preferred socket type, for example
    // SOCK_STREAM or SOCK_DGRAM. Specifying 0 in this field indicates that
    // socket addresses of any type can be returned by getaddrinfo().
    EXPECT_EQ(ai6->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai6->ai_protocol, 0);
    EXPECT_EQ(ai6->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai6.netaddr().str(), "127.0.0.1:50100");

    // There are more entries
    ASSERT_TRUE(ai6.get_next());

    EXPECT_EQ(ai6->ai_family, AF_INET);
    EXPECT_EQ(ai6->ai_socktype, SOCK_DGRAM);
    EXPECT_EQ(ai6->ai_protocol, 0);
    EXPECT_EQ(ai6->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai6.netaddr().str(), "127.0.0.1:50100");

    // There are more entries
    ASSERT_TRUE(ai6.get_next());

    EXPECT_EQ(ai6->ai_family, AF_INET);
    EXPECT_EQ(ai6->ai_socktype, SOCK_RAW);
    EXPECT_EQ(ai6->ai_protocol, 0);
    EXPECT_EQ(ai6->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai6.netaddr().str(), "127.0.0.1:50100");
    // There are no more entries
    EXPECT_FALSE(ai6.get_next());
}
#endif

#ifdef __APPLE__
TEST(AddrinfoTestSuite, get_active_empty_node_socktype_0) {
    // Test Unit for AF_INET
    CAddrinfo ai6("", "50100", AF_INET, 0, AI_NUMERICHOST);
    ASSERT_NO_THROW(ai6.load());

    EXPECT_EQ(ai6->ai_family, AF_INET);
    // ai_socktype specifies  the preferred socket type, for example
    // SOCK_STREAM or SOCK_DGRAM. Specifying 0 in this field indicates that
    // socket addresses of any type can be returned by getaddrinfo().
    EXPECT_EQ(ai6->ai_socktype, SOCK_DGRAM);
    EXPECT_EQ(ai6->ai_protocol, 0);
    EXPECT_EQ(ai6->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai6.netaddr().str(), "127.0.0.1:50100");

    // There are more entries
    ASSERT_TRUE(ai6.get_next());

    EXPECT_EQ(ai6->ai_family, AF_INET);
    EXPECT_EQ(ai6->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai6->ai_protocol, 0);
    EXPECT_EQ(ai6->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai6.netaddr().str(), "127.0.0.1:50100");

    // There are no more entries
    EXPECT_FALSE(ai6.get_next());
}
#endif

#ifdef _MSC_VER
TEST(AddrinfoTestSuite, get_active_empty_node_socktype_0) {
    // Test Unit for AF_INET
    CAddrinfo ai6("", "50100", AF_INET, 0, AI_NUMERICHOST);
    ASSERT_NO_THROW(ai6.load());

    EXPECT_EQ(ai6->ai_family, AF_INET);
    // ai_socktype specifies  the preferred socket type, for example
    // SOCK_STREAM or SOCK_DGRAM. Specifying 0 in this field indicates that
    // socket addresses of any type can be returned by getaddrinfo().
    EXPECT_EQ(ai6->ai_socktype, 0);
    EXPECT_EQ(ai6->ai_protocol, 0);
    EXPECT_EQ(ai6->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai6.netaddr().str(), "127.0.0.1:50100");

    ASSERT_FALSE(ai6.get_next());
}
#endif

TEST(AddrinfoTestSuite, get_active_empty_node_undef_socktype) {
    // Due to "man getaddrinfo" an empty node returns information of the
    // loopback interface. With setting everything unspecified we get all
    // available combinations but different on platforms.
    CAddrinfo ai1("", AF_UNSPEC, 0); /* same as
    CAddrinfo ai1("", "", AF_UNSPEC, 0); */

    EXPECT_EQ(ai1->ai_family, AF_UNSPEC);
    EXPECT_EQ(ai1->ai_socktype, 0);
    EXPECT_EQ(ai1->ai_protocol, 0); // ip
    EXPECT_EQ(ai1->ai_flags, 0);
    EXPECT_EQ(ai1->ai_addrlen, 0);
    EXPECT_EQ(ai1->ai_addr, nullptr);
    EXPECT_EQ(ai1->ai_canonname, nullptr);
    EXPECT_EQ(ai1->ai_next, nullptr);
    EXPECT_EQ(ai1.netaddr().str(), "");

    ASSERT_NO_THROW(ai1.load());

    bool double_res1{false};
    bool double_res2{false};
    [[maybe_unused]] bool double_res3{false};
    [[maybe_unused]] bool double_res4{false};
    [[maybe_unused]] bool double_res5{false};
    [[maybe_unused]] bool double_res6{false};

    do {
        EXPECT_EQ(ai1->ai_protocol, 0);
        EXPECT_EQ(ai1->ai_flags, AI_NUMERICSERV);
        EXPECT_NE(ai1->ai_addr, nullptr); // not equal nullptr
        EXPECT_EQ(ai1->ai_canonname, nullptr);
#if !defined(_MSC_VER)
        if (ai1->ai_family == AF_INET6 && ai1->ai_socktype == SOCK_STREAM) {
            ASSERT_FALSE(double_res1);
            double_res1 = true;
            EXPECT_EQ(ai1->ai_addrlen, 28);
            EXPECT_EQ(ai1.netaddr().str(), "[::1]:0");
        } else if (ai1->ai_family == AF_INET6 &&
                   ai1->ai_socktype == SOCK_DGRAM) {
            ASSERT_FALSE(double_res2);
            double_res2 = true;
            EXPECT_EQ(ai1->ai_addrlen, 28);
            EXPECT_EQ(ai1.netaddr().str(), "[::1]:0");
        } else if (ai1->ai_family == AF_INET &&
                   ai1->ai_socktype == SOCK_STREAM) {
            ASSERT_FALSE(double_res3);
            double_res3 = true;
            EXPECT_EQ(ai1->ai_addrlen, 16);
            EXPECT_EQ(ai1.netaddr().str(), "127.0.0.1:0");
        } else if (ai1->ai_family == AF_INET &&
                   ai1->ai_socktype == SOCK_DGRAM) {
            ASSERT_FALSE(double_res4);
            double_res4 = true;
            EXPECT_EQ(ai1->ai_addrlen, 16);
            EXPECT_EQ(ai1.netaddr().str(), "127.0.0.1:0");
#if !defined(__APPLE__)
        } else if (ai1->ai_family == AF_INET6 && ai1->ai_socktype == SOCK_RAW) {
            ASSERT_FALSE(double_res5);
            double_res5 = true;
            EXPECT_EQ(ai1->ai_addrlen, 28);
            EXPECT_EQ(ai1.netaddr().str(), "[::1]:0");
        } else if (ai1->ai_family == AF_INET && ai1->ai_socktype == SOCK_RAW) {
            ASSERT_FALSE(double_res6);
            double_res6 = true;
            EXPECT_EQ(ai1->ai_addrlen, 16);
            EXPECT_EQ(ai1.netaddr().str(), "127.0.0.1:0");
#endif
#else // _MSC_VER
        if (ai1->ai_family == AF_INET6 && ai1->ai_socktype == 0) {
            ASSERT_FALSE(double_res1);
            double_res1 = true;
            EXPECT_EQ(ai1->ai_addrlen, 28);
            EXPECT_EQ(ai1.netaddr().str(), "[::1]:0");
        } else if (ai1->ai_family == AF_INET && ai1->ai_socktype == 0) {
            ASSERT_FALSE(double_res2);
            double_res2 = true;
            EXPECT_EQ(ai1->ai_addrlen, 16);
            EXPECT_EQ(ai1.netaddr().str(), "127.0.0.1:0");
#endif
        } else {
            GTEST_FAIL()
                << "  Unexpected address information: address family = "
                << ai1->ai_family << ", socket type = " << ai1->ai_socktype
                << "\n";
        }
    } while (ai1.get_next());
}

TEST(AddrinfoTestSuite, get_multiple_address_infos) {
    CAddrinfo ai1("localhost:50686");
    ASSERT_NO_THROW(ai1.load());

    constexpr int expected_addrinfos{2}; // Value of expected address infos
    int found_addrinfos{};
    do {
        EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
        EXPECT_EQ(ai1->ai_protocol, 0);
        EXPECT_EQ(ai1->ai_flags, AI_NUMERICSERV);
        EXPECT_NE(ai1->ai_addr, nullptr);
        EXPECT_EQ(ai1->ai_canonname, nullptr);

        switch (ai1->ai_family) {
        case AF_INET6:
            EXPECT_EQ(ai1->ai_addrlen, 28);
            EXPECT_EQ(ai1.netaddr().str(), "[::1]:50686");
            break;
        case AF_INET:
            EXPECT_EQ(ai1->ai_addrlen, 16);
            EXPECT_EQ(ai1.netaddr().str(), "127.0.0.1:50686");
            break;
        default:
            GTEST_FAIL() << "  Unexpected address family = " << ai1->ai_family
                         << "\n";
        }
        found_addrinfos++;
    } while (ai1.get_next());

    ASSERT_EQ(found_addrinfos, expected_addrinfos);
}

TEST(AddrinfoTestSuite, get_passive_node_address) {
    // To get a passive address info, node must be empty ("") otherwise flag
    // AI_PASSIVE is ignored.

    { // Scoped to reduce memory usage for testing with node "".
        // Test Unit for AF_UNSPEC
        CAddrinfo ai1("", "50106", AF_UNSPEC, SOCK_STREAM, AI_PASSIVE);
        ASSERT_NO_THROW(ai1.load());

        // Ubuntu returns AF_INET, MacOS and Microsoft Windows return AF_INET6
        EXPECT_THAT(ai1->ai_family, AnyOf(AF_INET6, AF_INET));
        EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
        EXPECT_EQ(ai1->ai_protocol, 0);
        EXPECT_EQ(ai1->ai_flags, AI_PASSIVE | AI_NUMERICSERV);
        EXPECT_THAT(ai1.netaddr().str(), AnyOf("[::]:50106", "0.0.0.0:50106"));

        // Test Unit for AF_UNSPEC
        CAddrinfo ai2("", "50107", AF_UNSPEC, 0, AI_PASSIVE | AI_NUMERICHOST);
        ASSERT_NO_THROW(ai2.load());

        // Ubuntu returns AF_INET, MacOS and Microsoft Windows return AF_INET6
        EXPECT_THAT(ai2->ai_family, AnyOf(AF_INET6, AF_INET));
#ifdef __APPLE__
        EXPECT_EQ(ai2->ai_socktype, SOCK_DGRAM);
#elif defined(_MSC_VER)
        EXPECT_EQ(ai2->ai_socktype, 0);
#else
        EXPECT_EQ(ai2->ai_socktype, SOCK_STREAM);
#endif
        EXPECT_EQ(ai2->ai_protocol, 0);
        EXPECT_EQ(ai2->ai_flags, AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);
        EXPECT_THAT(ai2.netaddr().str(), AnyOf("[::]:50107", "0.0.0.0:50107"));

        // Test Unit for AF_INET6
        CAddrinfo ai3("", "50108", AF_INET6, 0, AI_PASSIVE);
        ASSERT_NO_THROW(ai3.load());

        EXPECT_EQ(ai3->ai_family, AF_INET6);
#ifdef __APPLE__
        EXPECT_EQ(ai3->ai_socktype, SOCK_DGRAM);
#elif defined(_MSC_VER)
        EXPECT_EQ(ai3->ai_socktype, 0);
#else
        EXPECT_EQ(ai3->ai_socktype, SOCK_STREAM);
#endif
        EXPECT_EQ(ai3->ai_protocol, 0);
        EXPECT_EQ(ai3->ai_flags, AI_PASSIVE | AI_NUMERICSERV);
        EXPECT_EQ(ai3.netaddr().str(), "[::]:50108");

        // Test Unit for AF_INET6
        CAddrinfo ai4("", "50109", AF_INET6, 0, AI_PASSIVE | AI_NUMERICHOST);
        ASSERT_NO_THROW(ai4.load());

        EXPECT_EQ(ai4->ai_family, AF_INET6);
#ifdef __APPLE__
        EXPECT_EQ(ai4->ai_socktype, SOCK_DGRAM);
#elif defined(_MSC_VER)
        EXPECT_EQ(ai4->ai_socktype, 0);
#else
        EXPECT_EQ(ai4->ai_socktype, SOCK_STREAM);
#endif
        EXPECT_EQ(ai4->ai_protocol, 0);
        EXPECT_EQ(ai4->ai_flags, AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);
        EXPECT_EQ(ai4.netaddr().str(), "[::]:50109");

        // Test Unit for AF_INET
        CAddrinfo ai5("", "50110", AF_INET, 0, AI_PASSIVE);
        ASSERT_NO_THROW(ai5.load());

        EXPECT_EQ(ai5->ai_family, AF_INET);
#ifdef __APPLE__
        EXPECT_EQ(ai5->ai_socktype, SOCK_DGRAM);
#elif defined(_MSC_VER)
        EXPECT_EQ(ai5->ai_socktype, 0);
#else
        EXPECT_EQ(ai5->ai_socktype, SOCK_STREAM);
#endif
        EXPECT_EQ(ai5->ai_protocol, 0);
        EXPECT_EQ(ai5->ai_flags, AI_PASSIVE | AI_NUMERICSERV);
        EXPECT_EQ(ai5.netaddr().str(), "0.0.0.0:50110");

        // Test Unit for AF_INET
        CAddrinfo ai6("", "50111", AF_INET, 0, AI_PASSIVE | AI_NUMERICHOST);
        ASSERT_NO_THROW(ai6.load());

        EXPECT_EQ(ai6->ai_family, AF_INET);
#ifdef __APPLE__
        EXPECT_EQ(ai6->ai_socktype, SOCK_DGRAM);
#elif defined(_MSC_VER)
        EXPECT_EQ(ai6->ai_socktype, 0);
#else
        EXPECT_EQ(ai6->ai_socktype, SOCK_STREAM);
#endif
        EXPECT_EQ(ai6->ai_protocol, 0);
        EXPECT_EQ(ai6->ai_flags, AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);
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
        EXPECT_THROW(ai2.load(), std::runtime_error);

        // Test Unit for AF_INET6
        CAddrinfo ai4("[]", "50115", AF_INET6, 0, AI_PASSIVE | AI_NUMERICHOST);
        EXPECT_THROW(ai4.load(), std::runtime_error);

        // Test Unit for AF_INET
        // Wrong address family
        CAddrinfo ai6("[]", "50117", AF_INET, 0, AI_PASSIVE | AI_NUMERICHOST);
        EXPECT_THROW(ai6.load(), std::runtime_error);
    }

    // Test Unit
    // Using explicit the unknown netaddress should definetly return the
    // in6addr_any passive listening socket info.
    CAddrinfo ai1("[::]", "50006", AF_INET6, SOCK_DGRAM,
                  AI_PASSIVE | AI_NUMERICHOST);
    ASSERT_NO_THROW(ai1.load());

    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_DGRAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);
    // This will listen on all local network interfaces.
    EXPECT_EQ(ai1.netaddr().str(), "[::]:50006");

    // Test Unit
    // Using explicit the unknown netaddress should definetly return the
    // INADDR_ANY passive listening socket info.
    CAddrinfo ai2("0.0.0.0", "50032", AF_INET, SOCK_STREAM,
                  AI_PASSIVE | AI_NUMERICHOST);
    ASSERT_NO_THROW(ai2.load());

    EXPECT_EQ(ai2->ai_family, AF_INET);
    EXPECT_EQ(ai2->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai2->ai_protocol, 0);
    EXPECT_EQ(ai2->ai_flags, AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);
    // This will listen on all local network interfaces.
    EXPECT_EQ(ai2.netaddr().str(), "0.0.0.0:50032");
}

TEST(AddrinfoTestSuite, get_two_brackets_alphanum_node_address) {
    // "[]" is undefined and internal always assumed to be AI_NUMERICHOST (flag
    // set) so get it as invalid but does not trigger a DNS server call.

    // Test Unit
    CAddrinfo ai7("[]", "50098");
    EXPECT_THROW(ai7.load(), std::runtime_error);

    // Test Unit for AF_UNSPEC
    CAddrinfo ai1("[]", "50112", AF_UNSPEC, SOCK_STREAM, AI_PASSIVE);
    EXPECT_THROW(ai1.load(), std::runtime_error);
    CAddrinfo ai2("[]", "50112", AF_UNSPEC, SOCK_STREAM);
    EXPECT_THROW(ai2.load(), std::runtime_error);

    // Test Unit for AF_INET6
    CAddrinfo ai3("[]", "50114", AF_INET6, 0, AI_PASSIVE);
    EXPECT_THROW(ai3.load(), std::runtime_error);
    CAddrinfo ai4("[]", "50114", AF_INET6);
    EXPECT_THROW(ai4.load(), std::runtime_error);

    // Test Unit for AF_INET
    // This alphanumeric address is never a valid IPv4 address but it
    // invokes an expensive DNS lookup.
    CAddrinfo ai5("[]", "50116", AF_INET, 0, AI_PASSIVE);
    EXPECT_THROW(ai5.load(), std::runtime_error);
    CAddrinfo ai6("[]", "50116", AF_INET);
    EXPECT_THROW(ai6.load(), std::runtime_error);
}

TEST(AddrinfoTestSuite, get_info_loopback_interface) {
    // If node is not empty AI_PASSIVE is ignored.

    // Test Unit with string port number
    CAddrinfo ai1("[::1]", "50001", AF_INET6, SOCK_STREAM,
                  AI_PASSIVE | AI_NUMERICHOST);
    ASSERT_NO_THROW(ai1.load());

    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai1.netaddr().str(), "[::1]:50001");

    // Test Unit
    CAddrinfo ai5("[::1]", "50085", AF_INET6, SOCK_STREAM);
    ASSERT_NO_THROW(ai5.load());

    EXPECT_EQ(ai5->ai_family, AF_INET6);
    EXPECT_EQ(ai5->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai5->ai_protocol, 0);
    EXPECT_EQ(ai5->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai5.netaddr().str(), "[::1]:50085");

    // Test Unit
    CAddrinfo ai2("127.0.0.1", "50086", AF_INET, SOCK_DGRAM);
    ASSERT_NO_THROW(ai2.load());

    EXPECT_EQ(ai2->ai_family, AF_INET);
    EXPECT_EQ(ai2->ai_socktype, SOCK_DGRAM);
    EXPECT_EQ(ai2->ai_protocol, 0);
    EXPECT_EQ(ai2->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai2.netaddr().str(), "127.0.0.1:50086");

    // Test Unit
    CAddrinfo ai3("[::1]", "50087", AF_INET6);
    ASSERT_NO_THROW(ai3.load());

    EXPECT_EQ(ai3->ai_family, AF_INET6);
    EXPECT_EQ(ai3->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai3->ai_protocol, 0);
    EXPECT_EQ(ai3->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai3.netaddr().str(), "[::1]:50087");

    // Test Unit, does not trigger a DNS query
    CAddrinfo ai4("localhost", "50088");
    ASSERT_NO_THROW(ai4.load());

    EXPECT_THAT(ai4->ai_family, AnyOf(AF_INET6, AF_INET));
    EXPECT_EQ(ai4->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai4->ai_protocol, 0);
    EXPECT_EQ(ai4->ai_flags, AI_NUMERICSERV);
    EXPECT_THAT(ai4.netaddr().str(), AnyOf("[::1]:50088", "127.0.0.1:50088"));
}

TEST(AddrinfoTestSuite, empty_service) {
    // With a node but an empty service the returned port number in the address
    // structure is set to 0.

    // Test Unit
    CAddrinfo ai1("[2001:db8::8]", "", AF_INET6, SOCK_STREAM, AI_NUMERICHOST);
    ASSERT_NO_THROW(ai1.load());

    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai1.netaddr().str(), "[2001:db8::8]:0");
}

TEST(AddrinfoTestSuite, load_wrong_address_family) {
    // Test Unit. Address family does not match the numeric host address.
    CAddrinfo ai1("192.168.155.15", "50003", AF_INET6, SOCK_STREAM,
                  AI_NUMERICHOST);
    EXPECT_THROW(ai1.load(), std::runtime_error);
}

TEST(AddrinfoTestSuite, load_loopback_addr_with_scope_id) {
    CAddrinfo ai1("[::1%1]");
    ASSERT_NO_THROW(ai1.load());

    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai1->ai_addrlen, 28);
    EXPECT_NE(ai1->ai_addr, nullptr);
    EXPECT_EQ(ai1->ai_canonname, nullptr);
    EXPECT_EQ(ai1.netaddr().str(), "[::1%1]:0");
    EXPECT_EQ(ai1->ai_next, nullptr);

    CAddrinfo ai2("[::1%lo]");
#ifdef __APPLE__
    ASSERT_NO_THROW(ai2.load());

    EXPECT_EQ(ai2->ai_family, AF_INET6);
    EXPECT_EQ(ai2->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai2->ai_protocol, 0);
    EXPECT_EQ(ai2->ai_flags, AI_NUMERICHOST | AI_NUMERICSERV);
    EXPECT_EQ(ai2->ai_addrlen, 28);
    EXPECT_NE(ai2->ai_addr, nullptr);
    EXPECT_EQ(ai2->ai_canonname, nullptr);
    EXPECT_EQ(ai2.netaddr().str(), "[::1]:0");
    EXPECT_EQ(ai2->ai_next, nullptr);
#else
    EXPECT_THROW(ai2.load(), std::runtime_error);
#endif
}

TEST(AddrinfoTestSuite, load_lla_with_scope_id) {
    CAddrinfo ai1("[fe80::acd%1]:ssh");
    ASSERT_NO_THROW(ai1.load());

    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai1->ai_addrlen, 28);
    EXPECT_NE(ai1->ai_addr, nullptr);
    EXPECT_EQ(ai1->ai_canonname, nullptr);
    EXPECT_EQ(ai1.netaddr().str(), "[fe80::acd%1]:22");
    EXPECT_EQ(ai1->ai_next, nullptr);

    CAddrinfo ai2("[fe80::acd%lo]:ssh");
#ifdef _MSC_VER
    EXPECT_THROW(ai2.load(), std::runtime_error);
#else
    ASSERT_NO_THROW(ai2.load());

    EXPECT_EQ(ai2->ai_family, AF_INET6);
    EXPECT_EQ(ai2->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai2->ai_protocol, 0);
    EXPECT_EQ(ai2->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai2->ai_addrlen, 28);
    EXPECT_NE(ai2->ai_addr, nullptr);
    EXPECT_EQ(ai2->ai_canonname, nullptr);
    EXPECT_THAT(
        ai2.netaddr().str(),
        Conditional(co == Co::clang, "[fe80::acd]:22", "[fe80::acd%1]:22"));
    EXPECT_EQ(ai2->ai_next, nullptr);
#endif // _MSC_VER
}

TEST(AddrinfoTestSuite, load_uad_with_scope_id) {
    CAddrinfo ai1("[2001:db8::55%1]:https");
    ASSERT_NO_THROW(ai1.load());

    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai1->ai_addrlen, 28);
    EXPECT_NE(ai1->ai_addr, nullptr);
    EXPECT_EQ(ai1->ai_canonname, nullptr);
    EXPECT_EQ(ai1.netaddr().str(), "[2001:db8::55%1]:443");
    EXPECT_EQ(ai1->ai_next, nullptr);

    CAddrinfo ai2("[2001:db8::55%lo]:https");
#if __APPLE__
    ASSERT_NO_THROW(ai1.load());

    EXPECT_EQ(ai1->ai_family, AF_INET6);
    EXPECT_EQ(ai1->ai_socktype, SOCK_STREAM);
    EXPECT_EQ(ai1->ai_protocol, 0);
    EXPECT_EQ(ai1->ai_flags, AI_NUMERICHOST);
    EXPECT_EQ(ai1->ai_addrlen, 28);
    EXPECT_NE(ai1->ai_addr, nullptr);
    EXPECT_EQ(ai1->ai_canonname, nullptr);
    EXPECT_EQ(ai1.netaddr().str(), "[2001:db8::55%1]:443");
    EXPECT_EQ(ai1->ai_next, nullptr);
#else
    EXPECT_THROW(ai2.load(), std::runtime_error);
#endif
}

} // namespace utest


int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include <utest/utest_main.inc>
    return gtest_return_code; // managed in gtest_main.inc
}
