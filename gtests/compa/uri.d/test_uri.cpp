// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-03-08

// Helpful link for ip address structures:
// https://stackoverflow.com/a/16010670/5014688

// Include source code for testing. So we have also direct access to static
// functions which need to be tested.
#include "pupnp/upnp/src/genlib/net/uri/uri.cpp"

#include "upnplib/uri.hpp"
#include "umock/netdb.hpp"

#include "gmock/gmock.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;

using ::upnplib::Curi;

namespace compa {

bool old_code{false}; // Managed in compa/gtest_main.inc
bool github_actions = ::std::getenv("GITHUB_ACTIONS");

//
// Mocking
// =======
class NetdbMock : public umock::NetdbInterface {
  public:
    virtual ~NetdbMock() override {}
    MOCK_METHOD(int, getaddrinfo,
                (const char* node, const char* service,
                 const struct addrinfo* hints, struct addrinfo** res),
                (override));
    MOCK_METHOD(void, freeaddrinfo, (struct addrinfo * res), (override));
};

class Mock_netv4info : public NetdbMock {
    // This is a derived class from mocking netdb to provide a structure for
    // addrinfo that can be given to the mocked program.
  private:
    // Provide structures to mock system call for network address
    struct sockaddr_in m_sa {};
    struct addrinfo m_res {};

  public:
    Mock_netv4info() { m_sa.sin_family = AF_INET; }

    addrinfo* set(const char* a_ipaddr, uint16_t a_port) {
        inet_pton(m_sa.sin_family, a_ipaddr, &m_sa.sin_addr);
        m_sa.sin_port = htons(a_port);

        m_res.ai_family = m_sa.sin_family;
        m_res.ai_addrlen = sizeof(struct sockaddr);
        m_res.ai_addr = (sockaddr*)&m_sa;

        return &m_res;
    }
};

//
// parse_hostport() function: tests from the uri module
// ====================================================
#if false
TEST(HostportIp4TestSuite, check_getaddrinfo) {
    // This test is for humans only to check what getaddrinfo() returns exactly
    // so we can correct mock it. Don't set '#if true' permanently because it
    // calls the real ::getaddrinfo() and will slow down this gtest
    // dramatically. It calls DNS internet server that may have a long delay.
    const struct ::addrinfo hints {
        {}, AF_INET, {}, {}, {}, {}, {}, {}
    };
    struct ::addrinfo* res{};

    EXPECT_EQ(::getaddrinfo("google.com", NULL, &hints, &res), 0);

    // Prefix "http://" is invalid
    int rc = ::getaddrinfo("http://google.com", NULL, &hints, &res);
    ::std::cout << "DEBUG: return info = " << ::gai_strerror(rc) << "(" << rc
                << ")\n"
                << "DEBUG: EAI_NONAME has number " << EAI_NONAME << '\n';

    EXPECT_EQ(::getaddrinfo("127.0.0.1", NULL, &hints, &res), 0);

    //  EAI_NONAME(-2) = Name or service not known
    EXPECT_EQ(::getaddrinfo("http://127.0.0.1", NULL, &hints, &res),
              EAI_NONAME);

    ::freeaddrinfo(res);
}

TEST(HostportIp4TestSuite, check_freeaddrinfo) {
    // This test is for humans only to check how freeaddrinfo() works so we can
    // correct mock it. Don't set '#if true' permanently because this does not
    // test production code.
    const struct ::addrinfo hints {
        {}, AF_INET, {}, {}, {}, {}, {}, {}
    };
    struct ::addrinfo* res;
    // Set pointer to invalid (uniitialized "random") value.
    ::memset(&res, 0xaa, sizeof(res));
    ::std::cout << "DEBUG: initial addrinfo* res = " << res << '\n';

    // This will segfault but a nullptr initialized res will not.
    // ::freeaddrinfo(res);

    // Failing getaddrinfo()
    EXPECT_EQ(::getaddrinfo("http://127.0.0.1", NULL, &hints, &res), EAI_NONAME);
    ::std::cout << "DEBUG: failed query, addrinfo* res = " << res << '\n';

    // Because res isn't touched with a failed getaddrinfo() this will also
    // segfault but a nullptr initialized res will not.
    // ::freeaddrinfo(res);

    EXPECT_EQ(::getaddrinfo("127.0.0.1", NULL, &hints, &res), 0);
    ::std::cout << "DEBUG: successful query, addrinfo* res = " << res << '\n';

    struct ::addrinfo* res2;
    ::memset(&res2, 0x55, sizeof(res));
    EXPECT_EQ(::getaddrinfo("127.0.0.1", NULL, &hints, &res2), 0);
    ::std::cout << "DEBUG: successful query, addrinfo* res2 = " << res2 << '\n';

    ::freeaddrinfo(res);
    ::std::cout << "DEBUG: after freeaddrinfo() addrinfo* res = " << res << '\n';

    ::freeaddrinfo(res2);
    ::std::cout << "DEBUG: after freeaddrinfo() addrinfo* res2 = " << res2 << '\n';

    ::std::cout
        << "\nDEBUG: As shown every successful getaddrinfo() should be freed "
           "with freeaddrinfo()\notherwise you risk memory leaks. "
           "On the other side you risk a segfault if you use\nan uninitalized "
           "addrinfo* pointer (not set to nullptr) and try to free it after a\n"
           "failed getaddrinfo().\n\n";
}
#endif

//
// parse_hostport() checks that getaddrinfo() isn't called.
// --------------------------------------------------------
class HostportIp4FTestSuite : public ::testing::Test {
  protected:
    NetdbMock m_mocked_netdb;
    hostport_type m_out;
    struct sockaddr_in* m_sai4 = (struct sockaddr_in*)&m_out.IPaddress;

    // Provide empty structures for mocking. Will be filled in the tests.
    struct sockaddr_in m_sa {};
    struct addrinfo m_res {};

    HostportIp4FTestSuite() {
        // Complete the addrinfo structure
        m_res.ai_addrlen = sizeof(struct sockaddr);
        m_res.ai_addr = (sockaddr*)&m_sa;

        // Set default return values for getaddrinfo in case we get an
        // unexpected call but it should not occur. We only use an ip address
        // here so name resolution should be called.
        ON_CALL(m_mocked_netdb, getaddrinfo(_, _, _, _))
            .WillByDefault(DoAll(SetArgPointee<3>(&m_res), Return(EAI_NONAME)));
        EXPECT_CALL(m_mocked_netdb, getaddrinfo(_, _, _, _)).Times(0);
        EXPECT_CALL(m_mocked_netdb, freeaddrinfo(_)).Times(0);
    }
};

TEST_F(HostportIp4FTestSuite, parse_ip_address_without_port) {
    // ip address without port
    EXPECT_EQ(parse_hostport("192.168.1.1", 80, &m_out), 11);
    EXPECT_STREQ(m_out.text.buff, "192.168.1.1");
    EXPECT_EQ(m_out.text.size, (size_t)11);
    EXPECT_EQ(m_sai4->sin_family, AF_INET);
    EXPECT_EQ(m_sai4->sin_port, htons(80));
    EXPECT_STREQ(inet_ntoa(m_sai4->sin_addr), "192.168.1.1");
}

TEST_F(HostportIp4FTestSuite, parse_ip_address_with_port) {
    // Ip address with port
    EXPECT_EQ(parse_hostport("192.168.1.2:443", 80, &m_out), 15);
    EXPECT_STREQ(m_out.text.buff, "192.168.1.2:443");
    EXPECT_EQ(m_out.text.size, (size_t)15);
    EXPECT_EQ(m_sai4->sin_family, AF_INET);
    EXPECT_EQ(m_sai4->sin_port, htons(443));
    EXPECT_STREQ(inet_ntoa(m_sai4->sin_addr), "192.168.1.2");
}

//
// parse_hostport() calls should fail
// ----------------------------------
class HostportFailIp4PTestSuite
    : public ::testing::TestWithParam<
          //           uristr,      ipaddr,      port
          ::std::tuple<const char*, const char*, uint16_t>> {};

TEST_P(HostportFailIp4PTestSuite, parse_name_with_scheme) {
    // Get parameter
    ::std::tuple params = GetParam();
    const char* uristr = ::std::get<0>(params);
    const char* ipaddr = ::std::get<1>(params);
    const uint16_t port = ::std::get<2>(params);

    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.set(ipaddr, port);

    // Mock for network address system call
    umock::Netdb netdb_injectObj(&netv4inf);
    EXPECT_CALL(netv4inf, getaddrinfo(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(res), Return(EAI_NONAME)));
    EXPECT_CALL(netv4inf, freeaddrinfo(_)).Times(0);

    // Provide arguments to execute the unit
    hostport_type out{};
    struct sockaddr_in* sai4 = (struct sockaddr_in*)&out.IPaddress;

    // Execute the unit
    EXPECT_EQ(parse_hostport(uristr, 80, &out), UPNP_E_INVALID_URL);
    EXPECT_STREQ(out.text.buff, nullptr);
    EXPECT_EQ(out.text.size, (size_t)0);
    EXPECT_EQ(sai4->sin_family, AF_UNSPEC);
    EXPECT_EQ(sai4->sin_port, 0);
    EXPECT_STREQ(inet_ntoa(sai4->sin_addr), "0.0.0.0");
}

INSTANTIATE_TEST_SUITE_P(
    uri, HostportFailIp4PTestSuite,
    ::testing::Values(
        //                 uristr,      ipaddr,     port
        ::std::make_tuple("http://192.168.1.3", "192.168.1.3", (uint16_t)80),
        ::std::make_tuple("/192.168.1.4:433", "192.168.1.4", (uint16_t)433),
        ::std::make_tuple("http://example.com/site", "192.168.1.5",
                          (uint16_t)80),
        ::std::make_tuple(":example.com:443/site", "192.168.1.6",
                          (uint16_t)433)));

//
// parse_hostport() calls should be successful
// -------------------------------------------
class HostportIp4PTestSuite
    : public ::testing::TestWithParam<
          //           uristr,      ipaddr,      port
          ::std::tuple<const char*, const char*, uint16_t>> {};

TEST_P(HostportIp4PTestSuite, parse_hostport_successful) {
    // Get parameter
    ::std::tuple params = GetParam();
    const char* uristr = ::std::get<0>(params);
    const char* ipaddr = ::std::get<1>(params);
    const uint16_t port = ::std::get<2>(params);
    const size_t size = ::strcspn(uristr, "/");

    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.set(ipaddr, port);

    // Mock for network address system call
    umock::Netdb netdb_injectObj(&netv4inf);
    EXPECT_CALL(netv4inf, getaddrinfo(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(res), Return(0)));
    EXPECT_CALL(netv4inf, freeaddrinfo(_)).Times(1);

    // Provide arguments to execute the unit
    hostport_type out{};
    struct sockaddr_in* sai4 = (struct sockaddr_in*)&out.IPaddress;

    // Execute the unit
    EXPECT_EQ((size_t)parse_hostport(uristr, 80, &out), size);
    EXPECT_STREQ(out.text.buff, uristr);
    EXPECT_EQ(out.text.size, size);
    EXPECT_EQ(sai4->sin_family, AF_INET);
    EXPECT_EQ(sai4->sin_port, htons(port));
    EXPECT_STREQ(inet_ntoa(sai4->sin_addr), ipaddr);
}

INSTANTIATE_TEST_SUITE_P(
    uri, HostportIp4PTestSuite,
    ::testing::Values(
        //                 uristr,      ipaddr,     port
        ::std::make_tuple("localhost", "127.0.0.1", (uint16_t)80),
        ::std::make_tuple("example.com", "192.168.1.3", (uint16_t)80),
        ::std::make_tuple("example.com:433", "192.168.1.4", (uint16_t)433),
        ::std::make_tuple("example-site.de/path?query#fragment", "192.168.1.5",
                          (uint16_t)80),
        ::std::make_tuple("example-site.de:433/path?query#fragment",
                          "192.168.1.5", (uint16_t)433)));

//
TEST(HostportIp6TestSuite, parse_hostport_without_name_resolution) {
    // TODO: Improve ip6 tests.
    hostport_type out;

    // Check IPv6 addresses
    struct sockaddr_in6* sai6 = (struct sockaddr_in6*)&out.IPaddress;
    char dst[128];

    EXPECT_EQ(
        parse_hostport("[2001:db8:85a3:8d3:1319:8a2e:370:7348]:443", 80, &out),
        42);
    EXPECT_STREQ(out.text.buff, "[2001:db8:85a3:8d3:1319:8a2e:370:7348]:443");
    EXPECT_EQ(out.text.size, (size_t)42);
    EXPECT_EQ(sai6->sin6_family, AF_INET6);
    EXPECT_EQ(sai6->sin6_port, htons(443));
    inet_ntop(AF_INET6, (const void*)sai6->sin6_addr.s6_addr, dst, sizeof(dst));
    EXPECT_STREQ(dst, "2001:db8:85a3:8d3:1319:8a2e:370:7348");
}

//
// token_cmp() functions: tests from the uri module
// ================================================
int token_string_cmp(const token* in1, const char* in2) {
    // This is a new function we use for tests. It will become part of new code.
    /*
     * \brief Compares buffer in the token object with the C string in in2 case
     * sensitive.
     *
     * \return
     *      \li < 0, if string1 is less than string2.
     *      \li == 0, if string1 is identical to string2 .
     *      \li > 0, if string1 is greater than string2.
     */
    if (in1 == nullptr && in2 == nullptr)
        return 0;
    if (in1 == nullptr && in2 != nullptr)
        return -1;
    if (in1 != nullptr && in2 == nullptr)
        return 1;
    // Here we have a valid in2 pointer to a C string, even if the string is
    // empty. If there is a nullptr in the token buffer then it is always
    // smaller.
    if (in1->buff == nullptr)
        return -1;

    const size_t in2_length = strlen(in2);
    if (in1->size != in2_length)
        return in1->size < in2_length ? -1 : 1;
    else
        return strncmp(in1->buff, in2, in1->size);
}

TEST(TokenCmpTestSuite, check_token_cmp) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    Curi uriObj;
    // == 0, if string1 is identical to string2.
    token inull{nullptr, 0};
    EXPECT_EQ(uriObj.token_cmp(&inull, &inull), 0);

    token in0{"", 0};
    EXPECT_EQ(uriObj.token_cmp(&in0, &in0), 0);

    token in1{"some entry", 10};
    EXPECT_EQ(uriObj.token_cmp(&in1, &in1), 0);

    // < 0, if string1 is less than string2.
    token in2{"some longer entry", 17};

    if (old_code) {
        ::std::cout
            << "  BUG! With string1 less than string2 it should return < 0 "
               "not > 0.\n";
        EXPECT_GT(uriObj.token_cmp(&in1, &in2), 0);

    } else {

        EXPECT_LT(uriObj.token_cmp(&in1, &in2), 0)
            << "  # With string1 less than string2 it should return < 0 not > "
               "0.";
    }

    // > 0, if string1 is greater than string2.
    token in3{"entry", 5};
    EXPECT_GT(uriObj.token_cmp(&in1, &in3), 0);
}

TEST(TokenCmpTestSuite, check_token_string_cmp) {
    // == 0, if string1 is identical to string2.
    EXPECT_EQ(token_string_cmp(nullptr, nullptr), 0);

    constexpr token in0{"", 0};
    constexpr char sinempty[]{""};
    EXPECT_EQ(token_string_cmp(&in0, sinempty), 0);

    constexpr token in1{"some entry", 10};
    constexpr char sin1[]{"some entry"};
    EXPECT_EQ(token_string_cmp(&in1, sin1), 0);

    // < 0, if string1 is less than string2.
    EXPECT_LT(token_string_cmp(nullptr, sinempty), 0);

    constexpr token inull{nullptr, 0};
    EXPECT_LT(token_string_cmp(&inull, sinempty), 0);

    constexpr char sin2[]{"some longer entry"};
    EXPECT_LT(token_string_cmp(&in1, sin2), 0);

    // > 0, if string1 is greater than string2.
    EXPECT_GT(token_string_cmp(&inull, nullptr), 0);

    constexpr char sin3[]{"entry"};
    EXPECT_GT(token_string_cmp(&in1, sin3), 0);
}

TEST(TokenCmpTestSuite, check_token_string_casecmp) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    Curi uriObj;
    // == 0, if string1 is identical to string2 case insensitive.
    token inull{nullptr, 0};
    constexpr char in5[]{""};

    if (old_code) {
        ::std::cout << "  BUG! A nullptr in the token structure, segfaults on "
                       "MS Windows (why only there?)\n";
#ifndef _WIN32
        EXPECT_EQ(uriObj.token_string_casecmp(&inull, in5), 0);
#endif
    } else {
        ASSERT_EXIT((uriObj.token_string_casecmp(&inull, in5), exit(0)),
                    ::testing::ExitedWithCode(0), ".*")
            << "  BUG! A nullptr in the token structure, segfaults on MS "
               "Windows (why only there?)\n";
    }

    token in0{"", 0};
    EXPECT_EQ(uriObj.token_string_casecmp(&in0, in5), 0);

    token in1{"some entry", 10};
    constexpr char in2[]{"SOME ENTRY"};
    EXPECT_EQ(uriObj.token_string_casecmp(&in1, in2), 0);

    // < 0, if string1 is less than string2.
    constexpr char in3[]{"some longer entry"};

    if (old_code) {
        ::std::cout
            << "  BUG! With string1 less than string2 it should return < 0 "
               "not > 0.\n";
        EXPECT_GT(uriObj.token_string_casecmp(&in1, in3), 0);

    } else {

        EXPECT_LT(uriObj.token_string_casecmp(&in1, in3), 0)
            << "  # With string1 less than string2 it should return < 0 not > "
               "0.";
    }

    // > 0, if string1 is greater than string2.
    constexpr char in4[]{"entry"};
    EXPECT_GT(uriObj.token_string_casecmp(&in1, in4), 0);
}

//
// replace_escaped() function: tests from the uri module
// =====================================================

TEST(UriIp4TestSuite, replace_escaped_check_buffer) {
    const char escstr[]{"%20"};
    size_t max = sizeof(escstr);
    char strbuf[sizeof(escstr)];
    memset(strbuf, 0xFF, max);
    EXPECT_EQ(strbuf[3], '\xFF');
    // Copies escstr with null terminator
    strcpy(strbuf, escstr);

    EXPECT_EQ(strbuf[0], '%');
    EXPECT_EQ(strbuf[1], '2');
    EXPECT_EQ(strbuf[2], '0');
    EXPECT_EQ(strbuf[3], '\0');
    EXPECT_EQ(strbuf[max - 1], '\0');

    // Execute unit; will fill trailing bytes with null
    Curi uriObj;
    ASSERT_EQ(uriObj.replace_escaped(strbuf, 0, &max), 1);
    EXPECT_EQ(strbuf[0], ' ');
    EXPECT_EQ(strbuf[1], '\0');
    EXPECT_EQ(strbuf[2], '\0');
    EXPECT_EQ(strbuf[3], '\0');
    EXPECT_EQ(max, sizeof(escstr) - 2);
}

TEST(UriIp4TestSuite, replace_escaped) {
    const char escstr[]{"Hello%20%0AWorld%G0!%0x"};
    size_t max = sizeof(escstr);
    char strbuf[sizeof(escstr)];
    memset(strbuf, 0xFF, max);
    strcpy(strbuf, escstr);
    EXPECT_EQ(strbuf[max - 1], '\0');

    // Execute the unit.
    // The function converts only one escaped character and the index must
    // exactly point to its '%'.
    Curi uriObj;
    ASSERT_EQ(uriObj.replace_escaped(strbuf, 5, &max), 1);
    EXPECT_STREQ(strbuf, "Hello %0AWorld%G0!%0x");
    // max buffer length is reduced by two characters (redudce '%xx' to ' ').
    EXPECT_EQ(max, sizeof(escstr) - 2);
    EXPECT_EQ(max, strlen(strbuf) + 1);
    // The two trailing free characters are filled with '\0'.
    EXPECT_EQ(strbuf[max - 2], 'x');
    EXPECT_EQ(strbuf[max - 1], '\0'); // regular new delimiter
    EXPECT_EQ(strbuf[max], '\0');     // filled
    EXPECT_EQ(strbuf[max + 1], '\0'); // filled
    EXPECT_EQ(max + 2, sizeof(strbuf));

    // Not pointing to an escaped character
    EXPECT_EQ(uriObj.replace_escaped(strbuf, 0, &max), 0);
    // No hex values after escape character
    EXPECT_EQ(uriObj.replace_escaped(strbuf, 16, &max), 0);
    EXPECT_EQ(uriObj.replace_escaped(strbuf, 20, &max), 0);
    // Failures should not modify output.
    EXPECT_STREQ(strbuf, "Hello %0AWorld%G0!%0x");
    EXPECT_EQ(max, sizeof(escstr) - 2);
}

//
// remove_escaped_chars() function: tests from the uri module
// ==========================================================
class RemoveEscCharsIp4PTestSuite
    : public ::testing::TestWithParam<
          //           escstr,      resultstr,   resultsize
          ::std::tuple<const char*, const char*, size_t>> {};

TEST_P(RemoveEscCharsIp4PTestSuite, remove_escaped_chars) {
    // Get parameter
    ::std::tuple params = GetParam();
    const char* escstr = ::std::get<0>(params);
    size_t size{strlen(escstr)}; // without '\0'

    // string buffer; we have to set it with a constant because we cannot get
    // the string size from pointer escstr. But there is a guard (ASSERT_GT).
    // strbuf must be one byte greater for '\0' than the strlen of escstr.
    char strbuf[32];
    ASSERT_GT(sizeof(strbuf), size)
        << "Error: string buffer too small for testing. You must increase it "
           "in this test.\n";
    strcpy(strbuf, escstr); // with '\0'

    // Prozess the unit.
    Curi uriObj;
    EXPECT_EQ(uriObj.remove_escaped_chars(strbuf, &size), UPNP_E_SUCCESS);
    EXPECT_STREQ(strbuf, ::std::get<1>(params)); // new result string
    EXPECT_EQ(size, ::std::get<2>(params));      // new result size
}

INSTANTIATE_TEST_SUITE_P(
    uri, RemoveEscCharsIp4PTestSuite,
    ::testing::Values(
        //                 escstr,                   resultstr,       resultsize
        ::std::make_tuple("%3CHello%20world%21%3E", "<Hello world!>", 14),
        ::std::make_tuple("", "", 0), ::std::make_tuple("%20", " ", 1),
        ::std::make_tuple("%3C%3E", "<>", 2),
        ::std::make_tuple("hello", "hello", 5)));

//
TEST(UriIp4TestSuite, remove_escaped_chars_edge_conditions) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    if (old_code) {
        ::std::cout << "  BUG! Calling remove_escaped_chars() with nullptr "
                       "should not segfault.\n";
    } else {

        char strbuf[32]{};
        size_t size{strlen(strbuf)};
        Curi uriObj;
        ASSERT_EXIT((uriObj.remove_escaped_chars(nullptr, nullptr), exit(0)),
                    ::testing::ExitedWithCode(0), ".*")
            << "  BUG! Calling remove_escaped_chars() with nullptr should not "
               "segfault.";

        EXPECT_EQ(uriObj.remove_escaped_chars(nullptr, nullptr),
                  UPNP_E_SUCCESS);

        strcpy(strbuf, "hello"); // with '\0'
        size = strlen(strbuf);
        ASSERT_EXIT((uriObj.remove_escaped_chars(nullptr, &size), exit(0)),
                    ::testing::ExitedWithCode(0), ".*")
            << "  BUG! Calling remove_escaped_chars() with nullptr should not "
               "segfault.";

        EXPECT_EQ(uriObj.remove_escaped_chars(nullptr, &size), UPNP_E_SUCCESS);
        EXPECT_EQ(size, (size_t)5);
        ASSERT_EXIT((uriObj.remove_escaped_chars(strbuf, nullptr), exit(0)),
                    ::testing::ExitedWithCode(0), ".*")
            << "  BUG! Calling remove_escaped_chars() with nullptr should not "
               "segfault.";

        EXPECT_EQ(uriObj.remove_escaped_chars(strbuf, nullptr), UPNP_E_SUCCESS);
        EXPECT_STREQ(strbuf, "hello");
    }
}

//
// remove_dots() function: tests from the uri module
// =================================================
class RemoveDotsIp4PTestSuite
    : public ::testing::TestWithParam<
          //           path,        size,   result,      retval
          ::std::tuple<const char*, size_t, const char*, int>> {};

TEST_P(RemoveDotsIp4PTestSuite, remove_dots) {
    // Get parameter
    ::std::tuple params = GetParam();
    const char* path = ::std::get<0>(params);
    size_t size = ::std::get<1>(params);
    const char* result = ::std::get<2>(params);
    int retval = ::std::get<3>(params);

    // string buffer; we have to set it with a constant because we cannot get
    // the string size from pointer path. But there is a guard (ASSERT_GT).
    // strbuf must be one byte greater for '\0' than the strlen of path.
    char strbuf[32];
    ASSERT_GT(sizeof(strbuf), strlen(path))
        << "Error: string buffer too small for testing. You must increase it "
           "in this test.\n";
    strcpy(strbuf, path); // with '\0'

    // Prozess the unit.
    Curi uriObj;
    EXPECT_EQ(uriObj.remove_dots(strbuf, size), retval);
    EXPECT_STREQ(strbuf, result);
}

INSTANTIATE_TEST_SUITE_P(
    uri, RemoveDotsIp4PTestSuite,
    ::testing::Values(
        //                 path,    size, result,  retval
        ::std::make_tuple("/./hello", 8, "/hello", UPNP_E_SUCCESS),
        ::std::make_tuple("./hello", 7, "hello", UPNP_E_SUCCESS),
        //::std::make_tuple("/../hello", 9, "/../hello", UPNP_E_INVALID_URL),
        ::std::make_tuple("/../hello", 9, "/hello", UPNP_E_SUCCESS), // BUG!
        //::std::make_tuple("../hello", 8, "../hello", UPNP_E_INVALID_URL),
        ::std::make_tuple("../hello", 8, "hello", UPNP_E_SUCCESS), // BUG!
        ::std::make_tuple("hello/", 6, "hello/", UPNP_E_SUCCESS),
        ::std::make_tuple("/hello/./", 9, "/hello/", UPNP_E_SUCCESS),
        ::std::make_tuple("/./hello/./world", 16, "/hello/world",
                          UPNP_E_SUCCESS),
        ::std::make_tuple("/hello/../world", 15, "/world", UPNP_E_SUCCESS),
        ::std::make_tuple("hello/../world", 14, "/world", UPNP_E_SUCCESS),
        ::std::make_tuple("/hello/world/../", 16, "/hello/", UPNP_E_SUCCESS),
        ::std::make_tuple("/hello/world/..", 15, "/hello/", UPNP_E_SUCCESS),
        ::std::make_tuple("/hello/../dear/../world", 23, "/world",
                          UPNP_E_SUCCESS),
        ::std::make_tuple("/./hello/foo/../goodbye", 23, "/hello/goodbye",
                          UPNP_E_SUCCESS),
        ::std::make_tuple(".", 1, "", UPNP_E_SUCCESS),
        ::std::make_tuple("..", 2, "", UPNP_E_SUCCESS),
        ::std::make_tuple("/", 1, "/", UPNP_E_SUCCESS),
        //::std::make_tuple("ab", 1, "a", UPNP_E_SUCCESS), // BUG! Does not
        // finish
        ::std::make_tuple("ab", 2, "ab", UPNP_E_SUCCESS)));

TEST(UriIp4TestSuite, remove_dots_report_bugs) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    Curi uriObj;
    // Have attention to the buffer size for this test to be greater than the
    // longest tested string.
    char strbuf[31 + 1];

    // Prozess the unit.

    if (old_code) {
        strcpy(strbuf, "/../hello"); // with '\0'
        ::std::cout << "  BUG! path beginning with '/../<segment>' or "
                       "'../<segment>' must fail.\n";
        EXPECT_EQ(uriObj.remove_dots(strbuf, strlen(strbuf)), UPNP_E_SUCCESS);
        EXPECT_STREQ(strbuf, "/hello");
        strcpy(strbuf, "../hello"); // with '\0'
        EXPECT_EQ(uriObj.remove_dots(strbuf, strlen(strbuf)), UPNP_E_SUCCESS);
        EXPECT_STREQ(strbuf, "hello");

        ::std::cout
            << "  BUG! Argument 'size' shorter than strlen should not hang "
               "the function.\n";
        // see below

    } else {

        ::std::cout
            << "  BUG! Argument 'size' shorter than strlen should not hang "
               "the function...\n";
        strcpy(strbuf, "ab"); // with '\0'
        // EXPECT_EQ(uriObj.remove_dots(strbuf, 1), UPNP_E_SUCCESS);
        EXPECT_STREQ(strbuf, "a");

        strcpy(strbuf, "/../hello"); // with '\0'
        EXPECT_EQ(uriObj.remove_dots(strbuf, strlen(strbuf)),
                  UPNP_E_INVALID_URL)
            << "  # path beginning with '/../<segment>' must fail with "
               "UPNP_E_INVALID_URL(-108).";
        EXPECT_STREQ(strbuf, "/../hello");
        strcpy(strbuf, "../hello"); // with '\0'
        EXPECT_EQ(uriObj.remove_dots(strbuf, strlen(strbuf)),
                  UPNP_E_INVALID_URL)
            << "  # path beginning with '../<segment>' must fail with "
               "UPNP_E_INVALID_URL(-108).";
        EXPECT_STREQ(strbuf, "../hello");
    }
}

//
// resolve_rel_url() function: tests from the uri module
// =====================================================

TEST(ResolveRelUrlIp4TestSuite, resolve_successful) {
    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.set("192.168.186.186", 443);

    // Mock for network address system call
    umock::Netdb netdb_injectObj(&netv4inf);
    EXPECT_CALL(netv4inf, getaddrinfo(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(res), Return(0)));
    EXPECT_CALL(netv4inf, freeaddrinfo(_)).Times(1);

    // Provide arguments to execute the unit
    char base_url[]{"https://example.com:443"};
    char rel_url[]{"homepage#this-fragment"};

    // Test Unit
    Curi uriObj;
    char* abs_url = uriObj.resolve_rel_url(*&base_url, *&rel_url);
    EXPECT_STREQ(abs_url, "https://example.com:443/homepage#this-fragment");
}

TEST(ResolveRelUrlIp4TestSuite, null_base_url_should_return_rel_url) {
    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.set("0.0.0.0", 0);

    // Mock for network address system call
    umock::Netdb netdb_injectObj(&netv4inf);
    ON_CALL(netv4inf, getaddrinfo(_, _, _, _))
        .WillByDefault(DoAll(SetArgPointee<3>(res), Return(EAI_NONAME)));
    EXPECT_CALL(netv4inf, getaddrinfo(_, _, _, _)).Times(0);
    EXPECT_CALL(netv4inf, freeaddrinfo(_)).Times(0);

    // Provide arguments to execute the unit
    char rel_url[]{"homepage#this-fragment"};

    // Execute the unit
    Curi uriObj;
    char* abs_url = uriObj.resolve_rel_url(NULL, *&rel_url);
    EXPECT_STREQ(abs_url, "homepage#this-fragment");
}

TEST(ResolveRelUrlIp4TestSuite, empty_base_url_should_return_rel_url) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.set("0.0.0.0", 0);

    // Mock for network address system call
    umock::Netdb netdb_injectObj(&netv4inf);
    ON_CALL(netv4inf, getaddrinfo(_, _, _, _))
        .WillByDefault(DoAll(SetArgPointee<3>(res), Return(EAI_NONAME)));
    EXPECT_CALL(netv4inf, getaddrinfo(_, _, _, _)).Times(0);
    EXPECT_CALL(netv4inf, freeaddrinfo(_)).Times(0);

    // Provide arguments to execute the unit
    char base_url[]{""};
    char rel_url[]{"homepage#this-fragment"};

    // Test Unit
    Curi uriObj;
    char* abs_url = uriObj.resolve_rel_url(*&base_url, *&rel_url);

    if (old_code) {
        ::std::cout
            << "  BUG! Empty base url should return rel url, not NULL.\n";
        EXPECT_EQ(abs_url, nullptr);
    } else {
        EXPECT_STREQ(abs_url, "homepage#this-fragment");
    }
}

TEST(ResolveRelUrlIp4TestSuite, null_rel_url_should_return_base_url) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    if (old_code) {
        ::std::cout << "  BUG! Segfault if rel_url is NULL.\n";

    } else {

        Mock_netv4info netv4inf;
        addrinfo* res = netv4inf.set("192.168.168.168", 80);

        // Mock for network address system call
        umock::Netdb netdb_injectObj(&netv4inf);
        EXPECT_CALL(netv4inf, getaddrinfo(_, _, _, _))
            .WillOnce(DoAll(SetArgPointee<3>(res), Return(0)));
        EXPECT_CALL(netv4inf, freeaddrinfo(_)).Times(1);

        // Provide arguments to execute the unit
        char base_url[]{"http://example.com"};

        // Execute the unit
        Curi uriObj;
        ASSERT_EXIT((uriObj.resolve_rel_url(*&base_url, NULL), exit(0)),
                    ::testing::ExitedWithCode(0), ".*")
            << "  BUG! Calling resolve_rel_url() with nullptr should not "
               "segfault.";

        char* abs_url = uriObj.resolve_rel_url(*&base_url, NULL);
        EXPECT_STREQ(abs_url, "http://example.com");
    }
}

TEST(ResolveRelUrlIp4TestSuite, empty_rel_url_should_return_base_url) {
    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.set("192.168.168.168", 80);

    // Mock for network address system call
    umock::Netdb netdb_injectObj(&netv4inf);
    EXPECT_CALL(netv4inf, getaddrinfo(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(res), Return(0)));
    EXPECT_CALL(netv4inf, freeaddrinfo(_)).Times(1);

    // Provide arguments to execute the unit
    char base_url[]{"http://example.com"};
    char rel_url[]{""};

    // Execute the unit
    Curi uriObj;
    char* abs_url = uriObj.resolve_rel_url(*&base_url, *&rel_url);
    EXPECT_STREQ(abs_url, "http://example.com");
}

TEST(ResolveRelUrlIp4TestSuite, null_base_and_rel_url_should_return_null) {
    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.set("0.0.0.0", 0);

    // Mock for network address system call
    umock::Netdb netdb_injectObj(&netv4inf);
    ON_CALL(netv4inf, getaddrinfo(_, _, _, _))
        .WillByDefault(DoAll(SetArgPointee<3>(res), Return(EAI_NONAME)));
    EXPECT_CALL(netv4inf, getaddrinfo(_, _, _, _)).Times(0);
    EXPECT_CALL(netv4inf, freeaddrinfo(_)).Times(0);

    // Execute the unit
    Curi uriObj;
    char* abs_url = uriObj.resolve_rel_url(NULL, NULL);
    EXPECT_EQ(abs_url, nullptr);
}

TEST(ResolveRelUrlIp4TestSuite, empty_base_and_rel_url_should_return_null) {
    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.set("0.0.0.0", 0);

    // Mock for network address system call
    umock::Netdb netdb_injectObj(&netv4inf);
    ON_CALL(netv4inf, getaddrinfo(_, _, _, _))
        .WillByDefault(DoAll(SetArgPointee<3>(res), Return(EAI_NONAME)));
    EXPECT_CALL(netv4inf, getaddrinfo(_, _, _, _)).Times(0);
    EXPECT_CALL(netv4inf, freeaddrinfo(_)).Times(0);

    // Provide arguments to execute the unit
    char base_url[]{""};
    char rel_url[]{""};

    // Execute the unit
    Curi uriObj;
    char* abs_url = uriObj.resolve_rel_url(*&base_url, *&rel_url);
    EXPECT_EQ(abs_url, nullptr);
}

TEST(ResolveRelUrlIp4TestSuite, absolute_rel_url_should_return_a_copy_of_it) {
    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.set("192.168.168.168", 443);

    // Mock for network address system call
    umock::Netdb netdb_injectObj(&netv4inf);
    EXPECT_CALL(netv4inf, getaddrinfo(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(res), Return(0)));
    EXPECT_CALL(netv4inf, freeaddrinfo(_)).Times(1);

    // Provide arguments to execute the unit
    char base_url[]{"http://example.com"};
    char rel_url[]{"https://absolute.net:443/home-page#fragment"};

    // Execute the unit
    Curi uriObj;
    char* abs_url = uriObj.resolve_rel_url(*&base_url, *&rel_url);
    EXPECT_STREQ(abs_url, "https://absolute.net:443/home-page#fragment");
}

TEST(ResolveRelUrlIp4TestSuite, base_and_rel_url_not_absolute_should_ret_null) {
    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.set("0.0.0.0", 0);

    // Mock for network address system call
    umock::Netdb netdb_injectObj(&netv4inf);
    ON_CALL(netv4inf, getaddrinfo(_, _, _, _))
        .WillByDefault(DoAll(SetArgPointee<3>(res), Return(EAI_NONAME)));
    EXPECT_CALL(netv4inf, getaddrinfo(_, _, _, _)).Times(0);
    EXPECT_CALL(netv4inf, freeaddrinfo(_)).Times(0);

    // Provide arguments to execute the unit
    char base_url[]{"/example.com"};
    char rel_url[]{"home-page#fragment"};

    // Execute the unit
    Curi uriObj;
    char* abs_url = uriObj.resolve_rel_url(*&base_url, *&rel_url);
    EXPECT_EQ(abs_url, nullptr);
}

TEST(UriIp4TestSuite, copy_URL_list) {
    GTEST_SKIP() << "Format of the URL_list unclear so far. Test must be "
                    "created later.";
}

TEST(UriIp4TestSuite, free_URL_list) {
    GTEST_SKIP()
        << "free_URL_list frees memory of copy_URL_list and test must be "
           "created together with it.";
}

} // namespace compa

int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "compa/gtest_main.inc"
    return gtest_return_code; // Managed in compa/gtest_main.inc
}
