// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-12-13

// Helpful link for ip address structures:
// https://stackoverflow.com/a/16010670/5014688

#include "gmock/gmock.h"
#include "upnpmock/netdb.hpp"

#include "genlib/net/uri/uri.cpp"

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;

namespace upnp {

//
// Interface for the uri module
// ----------------------------
// TODO: complete the interface
// clang-format off

class Iuri {
  public:
    virtual ~Iuri() {}

    virtual int token_string_casecmp(
            token* in1, const char* in2) = 0;
    virtual int parse_uri(
            const char *in, size_t max, uri_type *out) = 0;
};

class Curi : Iuri {
  public:
    virtual ~Curi() override {}

    int token_string_casecmp(token* in1, const char* in2) override {
        return ::token_string_casecmp(in1, in2); }
    int parse_uri(const char *in, size_t max, uri_type *out) override {
        return ::parse_uri(in, max, out); }
};
// clang-format on

//
// Mocking
// -------
class Mock_netdb : public Bnetdb {
    // Class to mock the free system functions.
    Bnetdb* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_netdb() {
        m_oldptr = netdb_h;
        netdb_h = this;
    }
    virtual ~Mock_netdb() { netdb_h = m_oldptr; }

    MOCK_METHOD(int, getaddrinfo,
                (const char* node, const char* service,
                 const struct addrinfo* hints, struct addrinfo** res),
                (override));
    MOCK_METHOD(void, freeaddrinfo, (struct addrinfo * res), (override));
};

//
// testsuite for uri
//------------------
TEST(UriTestSuite, parse_scheme_of_uri) {
    ::token out;

    EXPECT_EQ(::parse_scheme("https://dummy.net:80/page", 25, &out), 5);
    EXPECT_EQ(out.size, 5);
#ifdef OLD_TEST
    ::std::cerr << "  BUG! Only the http scheme (e.g. https) should be "
                   "returned, not the whole URI.\n";
    EXPECT_STREQ(out.buff, "https://dummy.net:80/page");
#else
    EXPECT_STREQ(out.buff, "https") << "  # Only the http scheme (e.g. https) "
                                       "should be returned, not the whole URI.";
#endif
    EXPECT_EQ(::parse_scheme("https://dummy.net:80/page", 6, &out), 5);
    EXPECT_EQ(out.size, 5);
#ifdef OLD_TEST
    // same as above
    EXPECT_STREQ(out.buff, "https://dummy.net:80/page");
#else
    EXPECT_STREQ(out.buff, "https");
#endif
    EXPECT_EQ(::parse_scheme("h:tps://dummy.net:80/page", 32, &out), 1);
    EXPECT_EQ(out.size, 1);
#ifdef OLD_TEST
    // same as above
    EXPECT_STREQ(out.buff, "h:tps://dummy.net:80/page");
#else
    EXPECT_STREQ(out.buff, "h");
#endif
    EXPECT_EQ(::parse_scheme("https://dummy.net:80/page", 5, &out), 0);
    EXPECT_EQ(out.size, 0);
    EXPECT_STREQ(out.buff, nullptr);
    EXPECT_EQ(::parse_scheme("1ttps://dummy.net:80/page", 32, &out), 0);
    EXPECT_EQ(out.size, 0);
    EXPECT_STREQ(out.buff, nullptr);
    EXPECT_EQ(::parse_scheme("h§tps://dummy.net:80/page", 32, &out), 0);
    EXPECT_EQ(out.size, 0);
    EXPECT_STREQ(out.buff, nullptr);
    EXPECT_EQ(::parse_scheme(":ttps://dummy.net:80/page", 32, &out), 0);
    EXPECT_EQ(out.size, 0);
    EXPECT_STREQ(out.buff, nullptr);
    EXPECT_EQ(::parse_scheme("h*tps://dummy.net:80/page", 32, &out), 0);
    EXPECT_EQ(out.size, 0);
    EXPECT_STREQ(out.buff, nullptr);
}

TEST(UriTestSuite, check_token_string_casecmp) {
    Curi uriObj;

    token in1{"some entry", 10};

    const char in2[] = "some entry";
    EXPECT_EQ(uriObj.token_string_casecmp(&in1, in2), 0);

    // < 0, if string1 is less than string2.
    const char in3[] = "some longer entry";
#ifdef OLD_TEST
    ::std::cerr << "  BUG! With string1 less than string2 it should return < 0 "
                   "not > 0.\n";
    EXPECT_GT(uriObj.token_string_casecmp(&in1, in3), 0);
#else
    EXPECT_LT(uriObj.token_string_casecmp(&in1, in3), 0)
        << "  # With string1 less than string2 it should return < 0 not > 0.";
#endif

    // > 0, if string1 is greater than string2.
    const char in4[] = "entry";
    EXPECT_GT(uriObj.token_string_casecmp(&in1, in4), 0);
}

#if false
// This test is only for humans to get an idea what's going on. If you need it,
// just enable it temporary. It is not intended to be permanent part of the
// tests. It doesn't really test things.

TEST(UriTestSuite, parse_uri) {
    // const char*
    // uri_str{"https://example-site.de:80/home-page?query-something#section1"};
    const char* uri_str{"scheme://example-site.de:80/path?query#fragment"};
    uri_type out;
    Curi uriObj;

    // Execute unit
    EXPECT_EQ(uriObj.parse_uri(uri_str, 64, &out), HTTP_SUCCESS);
    ::std::cerr << "DEBUG: out.scheme.buff = " << out.scheme.buff
                << ::std::endl;
    ::std::cerr << "DEBUG: out.type ABSOLUTE(0), RELATIVE(1) = " << out.type
                << ::std::endl;
    ::std::cerr
        << "DEBUG: out.path_type ABS_PATH(0), REL_PATH(1), OPAQUE_PART(2) = "
        << out.path_type << ::std::endl;
    ::std::cerr << "DEBUG: out.hostport.text.buff = " << out.hostport.text.buff
                << ::std::endl;
    ::std::cerr << "DEBUG: out.pathquery.buff = " << out.pathquery.buff
                << ::std::endl;
    ::std::cerr << "DEBUG: out.fragment.buff = " << out.fragment.buff
                << ::std::endl;
    ::std::cerr << "DEBUG: out.fragment.size = " << (signed)out.fragment.size
                << ::std::endl;
}
#endif

class UriIp4FTestSuite : public ::testing::Test {
  protected:
    Mock_netdb m_mocked_netdb;
    hostport_type m_out;
    struct sockaddr_in* m_sai4 = (struct sockaddr_in*)&m_out.IPaddress;

    // Provide empty structures for mocking. Will be filled in the tests.
    struct sockaddr_in m_sa {};
    struct addrinfo m_res {};

    UriIp4FTestSuite() {
        // Complete the addrinfo structure
        m_res.ai_addrlen = sizeof(struct sockaddr);
        m_res.ai_addr = (sockaddr*)&m_sa;

        // Set default return values for getaddrinfo in case we get an
        // unexpected call but it should not occur. An ip address should not be
        // asked for name resolution.
        ON_CALL(m_mocked_netdb, getaddrinfo(_, _, _, _))
            .WillByDefault(
                DoAll(SetArgPointee<3>(&m_res), Return(EAI_FAIL)));
        EXPECT_CALL(m_mocked_netdb, getaddrinfo(_, _, _, _)).Times(0);
        EXPECT_CALL(m_mocked_netdb, freeaddrinfo(_)).Times(0);
    }
};

TEST_F(UriIp4FTestSuite, parse_hostport_ip_address_without_port) {
    // ip address without port
    EXPECT_EQ(parse_hostport("192.168.1.1", 80, &m_out), 11);
    EXPECT_STREQ(m_out.text.buff, "192.168.1.1");
    EXPECT_EQ(m_out.text.size, 11);
    EXPECT_EQ(m_sai4->sin_family, AF_INET);
    EXPECT_EQ(m_sai4->sin_port, htons(80));
    EXPECT_STREQ(inet_ntoa(m_sai4->sin_addr), "192.168.1.1");
}

TEST_F(UriIp4FTestSuite, parse_hostport_ip_address_with_port) {
    // Ip address with port
    EXPECT_EQ(parse_hostport("192.168.1.2:443", 80, &m_out), 15);
    EXPECT_STREQ(m_out.text.buff, "192.168.1.2:443");
    EXPECT_EQ(m_out.text.size, 15);
    EXPECT_EQ(m_sai4->sin_family, AF_INET);
    EXPECT_EQ(m_sai4->sin_port, htons(443));
    EXPECT_STREQ(inet_ntoa(m_sai4->sin_addr), "192.168.1.2");
}

TEST(UriIp4TestSuite, parse_hostport_ip_address_with_scheme_will_fail) {
    Mock_netdb mocked_netdb;

    // Provide structures to mock system call for network address
    struct sockaddr_in sa {};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(80);
    inet_pton(AF_INET, "192.168.1.3", &sa.sin_addr);

    struct addrinfo res {};
    res.ai_family = sa.sin_family;
    res.ai_addrlen = sizeof(struct sockaddr);
    res.ai_addr = (sockaddr*)&sa;

    // Mock for network address system call
    EXPECT_CALL(mocked_netdb, getaddrinfo(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(&res), Return(0)));
    EXPECT_CALL(mocked_netdb, freeaddrinfo(_)).Times(1);

    // Call unit for ip address with scheme
    hostport_type out{};
    struct sockaddr_in* sai4 = (struct sockaddr_in*)&out.IPaddress;

    // Execute unit without port should fail
    EXPECT_EQ(parse_hostport("http://192.168.1.3", 80, &out),
              UPNP_E_INVALID_URL);
    EXPECT_EQ(out.text.buff, nullptr);
    EXPECT_EQ(out.text.size, 0);
    EXPECT_EQ(sai4->sin_family, AF_INET);
    EXPECT_EQ(sai4->sin_port, htons(80));
    EXPECT_STREQ(inet_ntoa(sai4->sin_addr), "192.168.1.3");
}

TEST(UriIp4TestSuite,
     parse_hostport_ip_address_with_scheme_and_port_will_fail) {
    Mock_netdb mocked_netdb;

    // Provide structures to mock system call for network address
    struct sockaddr_in sa {};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(443);
    inet_pton(AF_INET, "192.168.1.4", &sa.sin_addr);

    struct addrinfo res {};
    res.ai_family = sa.sin_family;
    res.ai_addrlen = sizeof(struct sockaddr);
    res.ai_addr = (sockaddr*)&sa;

    // Mock for network address system call
    EXPECT_CALL(mocked_netdb, getaddrinfo(_, _, _, _))
        .WillRepeatedly(DoAll(SetArgPointee<3>(&res), Return(0)));
    EXPECT_CALL(mocked_netdb, freeaddrinfo(_)).Times(1);

    // Call unit for ip address with scheme
    hostport_type out{};
    struct sockaddr_in* sai4 = (struct sockaddr_in*)&out.IPaddress;

    // Execute unit with port should fail
    EXPECT_EQ(parse_hostport("http://192.168.1.4:443", 80, &out),
              UPNP_E_INVALID_URL);
    EXPECT_EQ(out.text.buff, nullptr);
    EXPECT_EQ(out.text.size, 0);
    EXPECT_EQ(sai4->sin_family, AF_INET);
    EXPECT_EQ(sai4->sin_port, htons(443));
    EXPECT_STREQ(inet_ntoa(sai4->sin_addr), "192.168.1.4");
}

//
// Ip4 Parameterized test suite
// ############################
//
class UriIp4PTestSuite : public ::testing::TestWithParam<
                             ::std::tuple<const char*, const char*, int>> {};

TEST_P(UriIp4PTestSuite, parse_hostport_successful) {
    Mock_netdb mocked_netdb;
    const char* uristr = ::std::get<0>(GetParam());
    const char* ipaddr = ::std::get<1>(GetParam());
    int port = ::std::get<2>(GetParam());

    // Provide structures to mock system call for network address
    struct sockaddr_in sa {};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    inet_pton(AF_INET, ipaddr, &sa.sin_addr);

    struct addrinfo res {};
    res.ai_family = sa.sin_family;
    res.ai_addrlen = sizeof(struct sockaddr);
    res.ai_addr = (sockaddr*)&sa;

    // Mock for network address system call
    EXPECT_CALL(mocked_netdb, getaddrinfo(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(&res), Return(0)));
    EXPECT_CALL(mocked_netdb, freeaddrinfo(_)).Times(1);

    // Call unit for address name without port
    hostport_type out{};
    struct sockaddr_in* sai4 = (struct sockaddr_in*)&out.IPaddress;

    EXPECT_EQ(parse_hostport(uristr, 80, &out), ::strcspn(uristr, "/"));
    EXPECT_STREQ(out.text.buff, uristr);
    EXPECT_EQ(out.text.size, ::strcspn(uristr, "/"));
    EXPECT_EQ(sai4->sin_family, AF_INET);
    EXPECT_EQ(sai4->sin_port, htons(port));
    EXPECT_STREQ(inet_ntoa(sai4->sin_addr), ipaddr);
}

INSTANTIATE_TEST_SUITE_P(
    uri, UriIp4PTestSuite,
    ::testing::Values(
        //                 uristr,       ipaddr,      port
        ::std::make_tuple("localhost", "127.0.0.1", 80),
        ::std::make_tuple("example.com", "192.168.1.3", 80),
        ::std::make_tuple("example.com:433", "192.168.1.4", 433),
        ::std::make_tuple("example-site.de/path?query#fragment", "192.168.1.5",
                          80),
        ::std::make_tuple("example-site.de:433/path?query#fragment",
                          "192.168.1.5", 433)));

//
TEST(UriIp6TestSuite, parse_hostport_without_name_resolution)
// TODO: Improve ip6 tests.
{
    hostport_type out;

    // Check IPv6 addresses
    struct sockaddr_in6* sai6 = (struct sockaddr_in6*)&out.IPaddress;
    char dst[128];

    EXPECT_EQ(
        parse_hostport("[2001:db8:85a3:8d3:1319:8a2e:370:7348]:443", 80, &out),
        42);
    EXPECT_STREQ(out.text.buff, "[2001:db8:85a3:8d3:1319:8a2e:370:7348]:443");
    EXPECT_EQ(out.text.size, 42);
    EXPECT_EQ(sai6->sin6_family, AF_INET6);
    EXPECT_EQ(sai6->sin6_port, htons(443));
    inet_ntop(AF_INET6, (const void*)sai6->sin6_addr.s6_addr, dst, sizeof(dst));
    EXPECT_STREQ(dst, "2001:db8:85a3:8d3:1319:8a2e:370:7348");
}

} // namespace upnp

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
