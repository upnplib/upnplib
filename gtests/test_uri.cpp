// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-12-21

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
// ============================
// clang-format off

class Iuri {
  public:
    virtual ~Iuri() {}

    virtual int replace_escaped(
            char* in, size_t index, size_t* max) = 0;
    virtual int copy_URL_list(
            URL_list* in, URL_list* out) = 0;
    virtual void free_URL_list(
            URL_list* list) = 0;
    virtual int token_string_casecmp(
            token* in1, const char* in2) = 0;
    virtual int token_cmp(
            token* in1, token* in2) = 0;
    virtual int remove_escaped_chars(
            char* in, size_t* size) = 0;
    virtual int remove_dots(
            char* buf, size_t size) = 0;
    virtual char* resolve_rel_url(
            char* base_url, char* rel_url) = 0;
    virtual int parse_uri(
            const char *in, size_t max, uri_type *out) = 0;
};

class Curi : Iuri {
  public:
    virtual ~Curi() override {}

    int replace_escaped(char* in, size_t index, size_t* max) override {
        return ::replace_escaped(in, index, max); }
    int copy_URL_list(URL_list* in, URL_list* out) override {
        return ::copy_URL_list(in, out); }
    void free_URL_list(URL_list* list) override {
        return ::free_URL_list(list); }
    int token_string_casecmp(token* in1, const char* in2) override {
        return ::token_string_casecmp(in1, in2); }
    int token_cmp(token* in1, token* in2) override {
        return ::token_cmp(in1, in2); }
    int remove_escaped_chars(char* in, size_t* size) override {
        return ::remove_escaped_chars(in, size); }
    int remove_dots(char* buf, size_t size) override {
        return ::remove_dots(buf, size); }
    char* resolve_rel_url(char* base_url, char* rel_url) override {
        return ::resolve_rel_url(base_url, rel_url); }
    int parse_uri(const char *in, size_t max, uri_type *out) override {
        return ::parse_uri(in, max, out); }
};
// clang-format on

//
// Mocking
// =======
class Mock_netdb : public Bnetdb
// Class to mock the free system functions.
{
  private:
    Bnetdb* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_netdb() {
        m_oldptr = netdb_h;
        netdb_h = this;
    }
    virtual ~Mock_netdb() override { netdb_h = m_oldptr; }

    MOCK_METHOD(int, getaddrinfo,
                (const char* node, const char* service,
                 const struct addrinfo* hints, struct addrinfo** res),
                (override));
    MOCK_METHOD(void, freeaddrinfo, (struct addrinfo * res), (override));
};

class Mock_netv4info : public Mock_netdb
// This is a derived class from mocking netdb to provide a structure for
// addrinfo that can be given to the mocked program.
{
  private:
    Bnetdb* m_oldptr;

    // Provide structures to mock system call for network address
    struct sockaddr_in m_sa {};
    struct addrinfo m_res {};

  public:
    // Save and restore the old pointer to the production function
    Mock_netv4info() {
        m_oldptr = netdb_h;
        netdb_h = this;

        m_sa.sin_family = AF_INET;
    }

    virtual ~Mock_netv4info() override { netdb_h = m_oldptr; }

    addrinfo* set(const char* a_ipaddr, short int a_port) {
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
TEST(HostportIp4TestSuite, check_getaddrinfo)
// This is for humans only to check what getaddrinfo() returns exactly so we can
// correct mock it. Don't set '#if true' permanently because it calls the real
// ::getaddrinfo() and will slow down this gtest dramatically. It calls DNS
// internet server that may have a long delay.
{
    const struct addrinfo hints {
        {}, AF_INET, {}, {}, {}, {}, {}, {}
    };
    struct addrinfo* res{};

    int rc = ::getaddrinfo("http://google.com", NULL, &hints, &res);
    ::std::cout << "DEBUG: return info = " << ::gai_strerror(rc) << "(" << rc
                << ")\n"
                << "DEBUG: EAI_NONAME has number " << EAI_NONAME << '\n';

    EXPECT_EQ(::getaddrinfo("127.0.0.1", NULL, &hints, &res), 0);
    EXPECT_EQ(::getaddrinfo("google.com", NULL, &hints, &res), 0);
    //  EAI_NONAME(-2) = Name or service not known
    EXPECT_EQ(::getaddrinfo("http://127.0.0.1", NULL, &hints, &res), EAI_NONAME);
    EXPECT_EQ(::getaddrinfo("http://google.com", NULL, &hints, &res), EAI_NONAME);
}
#endif

//
// parse_hostport() checks that getaddrinfo() isn't called.
// --------------------------------------------------------
class HostportIp4FTestSuite : public ::testing::Test {
  protected:
    Mock_netdb m_mocked_netdb;
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
        // unexpected call but it should not occur. An ip address should not be
        // asked for name resolution.
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
          ::std::tuple<const char*, const char*, int>> {};

TEST_P(HostportFailIp4PTestSuite, parse_name_with_scheme) {
    // Get parameter
    ::std::tuple params = GetParam();
    const char* uristr = ::std::get<0>(params);
    const char* ipaddr = ::std::get<1>(params);
    const int port = ::std::get<2>(params);

    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.set(ipaddr, port);

    // Mock for network address system call
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
        ::std::make_tuple("http://192.168.1.3", "192.168.1.3", 80),
        ::std::make_tuple("/192.168.1.4:433", "192.168.1.4", 433),
        ::std::make_tuple("http://example.com/site", "192.168.1.5", 80),
        ::std::make_tuple(":example.com:443/site", "192.168.1.6", 433)));

//
// parse_hostport() calls should be successful
// -------------------------------------------
class HostportIp4PTestSuite : public ::testing::TestWithParam<
                                  //           uristr,      ipaddr,      port
                                  ::std::tuple<const char*, const char*, int>> {
};

TEST_P(HostportIp4PTestSuite, parse_hostport_successful) {
    // Get parameter
    ::std::tuple params = GetParam();
    const char* uristr = ::std::get<0>(params);
    const char* ipaddr = ::std::get<1>(params);
    const int port = ::std::get<2>(params);
    const size_t size = ::strcspn(uristr, "/");

    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.set(ipaddr, port);

    // Mock for network address system call
    EXPECT_CALL(netv4inf, getaddrinfo(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(res), Return(0)));
    EXPECT_CALL(netv4inf, freeaddrinfo(_)).Times(1);

    // Provide arguments to execute the unit
    hostport_type out{};
    struct sockaddr_in* sai4 = (struct sockaddr_in*)&out.IPaddress;

    // Execute the unit
    EXPECT_EQ((const size_t)parse_hostport(uristr, 80, &out), size);
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
        ::std::make_tuple("localhost", "127.0.0.1", 80),
        ::std::make_tuple("example.com", "192.168.1.3", 80),
        ::std::make_tuple("example.com:433", "192.168.1.4", 433),
        ::std::make_tuple("example-site.de/path?query#fragment", "192.168.1.5",
                          80),
        ::std::make_tuple("example-site.de:433/path?query#fragment",
                          "192.168.1.5", 433)));

//
TEST(HostportIp6TestSuite, parse_hostport_without_name_resolution)
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
    EXPECT_EQ(out.text.size, (size_t)42);
    EXPECT_EQ(sai6->sin6_family, AF_INET6);
    EXPECT_EQ(sai6->sin6_port, htons(443));
    inet_ntop(AF_INET6, (const void*)sai6->sin6_addr.s6_addr, dst, sizeof(dst));
    EXPECT_STREQ(dst, "2001:db8:85a3:8d3:1319:8a2e:370:7348");
}

//
// token_string_casecmp() function: tests from the uri module
// ==========================================================
TEST(UriIp4TestSuite, check_token_string_casecmp) {
    Curi uriObj;

    // == 0, if string1 is identical to string2 case insensitive.
    token inull{nullptr, 0};
    const char in5[]{""};
#ifdef OLD_TEST
    ::std::cout
        << "  BUG! A nullptr in the token structure, segfaults on MS Windows "
           "(why only there?)\n";
#endif
#ifndef _WIN32
    EXPECT_EQ(uriObj.token_string_casecmp(&inull, in5), 0);
#endif
    token in0{"", 0};
    EXPECT_EQ(uriObj.token_string_casecmp(&in0, in5), 0);

    token in1{"some entry", 10};
    const char in2[]{"SOME ENTRY"};
    EXPECT_EQ(uriObj.token_string_casecmp(&in1, in2), 0);

    // < 0, if string1 is less than string2.
    const char in3[]{"some longer entry"};
#ifdef OLD_TEST
    ::std::cout << "  BUG! With string1 less than string2 it should return < 0 "
                   "not > 0.\n";
    EXPECT_GT(uriObj.token_string_casecmp(&in1, in3), 0);
#else
    EXPECT_LT(uriObj.token_string_casecmp(&in1, in3), 0)
        << "  # With string1 less than string2 it should return < 0 not > 0.";
#endif

    // > 0, if string1 is greater than string2.
    const char in4[]{"entry"};
    EXPECT_GT(uriObj.token_string_casecmp(&in1, in4), 0);
}

//
// token_cmp() function: tests from the uri module
// ===============================================

TEST(UriIp4TestSuite, check_token_cmp) {
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
#ifdef OLD_TEST
    ::std::cout << "  BUG! With string1 less than string2 it should return < 0 "
                   "not > 0.\n";
    EXPECT_GT(uriObj.token_cmp(&in1, &in2), 0);
#else
    EXPECT_LT(uriObj.token_cmp(&in1, &in2), 0)
        << "  # With string1 less than string2 it should return < 0 not > 0.";
#endif

    // > 0, if string1 is greater than string2.
    token in3{"entry", 5};
    EXPECT_GT(uriObj.token_cmp(&in1, &in3), 0);
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
#ifdef OLD_TEST
    ::std::cout << "  BUG! Calling remove_escaped_chars() with nullptr should "
                   "not segfault.\n";
#else
    char strbuf[32]{};
    size_t size{strlen(strbuf)};
    Curi uriObj;
    EXPECT_EQ(uriObj.remove_escaped_chars(nullptr, nullptr), UPNP_E_SUCCESS);

    strcpy(strbuf, "hello"); // with '\0'
    size = strlen(strbuf);
    EXPECT_EQ(uriObj.remove_escaped_chars(nullptr, &size), UPNP_E_SUCCESS);
    EXPECT_EQ(size, 5);

    EXPECT_EQ(uriObj.remove_escaped_chars(strbuf, nullptr), UPNP_E_SUCCESS);
    EXPECT_STREQ(strbuf, "hello");
#endif
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
    Curi uriObj;
    // Have attention to the buffer size for this test to be greater than the
    // longest tested string.
    char strbuf[31 + 1];

    // Prozess the unit.
#ifdef OLD_TEST
    strcpy(strbuf, "/../hello"); // with '\0'
    ::std::cout << "  BUG! path beginning with '/../<segment>' or "
                   "'../<segment>' must fail.\n";
    EXPECT_EQ(uriObj.remove_dots(strbuf, strlen(strbuf)), UPNP_E_SUCCESS);
    EXPECT_STREQ(strbuf, "/hello");
    strcpy(strbuf, "../hello"); // with '\0'
    EXPECT_EQ(uriObj.remove_dots(strbuf, strlen(strbuf)), UPNP_E_SUCCESS);
    EXPECT_STREQ(strbuf, "hello");

    ::std::cout << "  BUG! Argument 'size' shorter than strlen should not hang "
                   "the function.\n";
    // see below
#else
    ::std::cout << "  BUG! Argument 'size' shorter than strlen should not hang "
                   "the function...\n";
    strcpy(strbuf, "ab"); // with '\0'
    // EXPECT_EQ(uriObj.remove_dots(strbuf, 1), UPNP_E_SUCCESS);
    EXPECT_STREQ(strbuf, "a");

    strcpy(strbuf, "/../hello"); // with '\0'
    EXPECT_EQ(uriObj.remove_dots(strbuf, strlen(strbuf)), UPNP_E_INVALID_URL)
        << "  # path beginning with '/../<segment>' must fail with "
           "UPNP_E_INVALID_URL(-108).";
    EXPECT_STREQ(strbuf, "/../hello");
    strcpy(strbuf, "../hello"); // with '\0'
    EXPECT_EQ(uriObj.remove_dots(strbuf, strlen(strbuf)), UPNP_E_INVALID_URL)
        << "  # path beginning with '../<segment>' must fail with "
           "UPNP_E_INVALID_URL(-108).";
    EXPECT_STREQ(strbuf, "../hello");
#endif
}

//
// parse_uri() function: tests from the uri module
// ===============================================
#if false
TEST(UriIp4TestSuite, parse_uri_simple_call)
// This test is only for humans to get an idea what's going on. If you need it,
// set '#if true' only temporary. It is not intended to be permanent part of
// the tests. It doesn't really test things and because unmocked, it queries
// DNS server on the internet that may have long delays.
{
    const char* uri_str{"scheme://example-site.de:80/uripath?uriquery#urifragment"};
    // const char* uri_str{"mailto:a@b.com"};
    uri_type out;
    Curi uriObj;

    // Execute unit
    EXPECT_EQ(uriObj.parse_uri(uri_str, 64, &out), HTTP_SUCCESS);
    ::std::cout << "DEBUG: out.scheme.buff = " << out.scheme.buff
                << ::std::endl;
    ::std::cout << "DEBUG: out.type ABSOLUTE(0), RELATIVE(1) = " << out.type
                << ::std::endl;
    ::std::cout
        << "DEBUG: out.path_type ABS_PATH(0), REL_PATH(1), OPAQUE_PART(2) = "
        << out.path_type << ::std::endl;
    ::std::cout << "DEBUG: out.hostport.text.buff = " << out.hostport.text.buff
                << ::std::endl;
    ::std::cout << "DEBUG: out.pathquery.buff = " << out.pathquery.buff
                << ::std::endl;
    ::std::cout << "DEBUG: out.fragment.buff = " << out.fragment.buff
                << ::std::endl;
    ::std::cout << "DEBUG: out.fragment.size = " << (signed)out.fragment.size
                << ::std::endl;
}
#endif

TEST(UriIp4TestSuite, parse_scheme_of_uri) {
    ::token out;

    EXPECT_EQ(::parse_scheme("https://dummy.net:80/page", 6, &out), (size_t)5);
    EXPECT_EQ(out.size, (size_t)5);
    EXPECT_STREQ(out.buff, "https://dummy.net:80/page");
    EXPECT_EQ(::parse_scheme("https://dummy.net:80/page", 5, &out), (size_t)0);
    EXPECT_EQ(out.size, (size_t)0);
    EXPECT_STREQ(out.buff, nullptr);
    EXPECT_EQ(::parse_scheme("h:tps://dummy.net:80/page", 32, &out), (size_t)1);
    EXPECT_EQ(out.size, (size_t)1);
    EXPECT_STREQ(out.buff, "h:tps://dummy.net:80/page");
    EXPECT_EQ(::parse_scheme("1ttps://dummy.net:80/page", 32, &out), (size_t)0);
    EXPECT_EQ(out.size, (size_t)0);
    EXPECT_STREQ(out.buff, nullptr);
    EXPECT_EQ(::parse_scheme("h§tps://dummy.net:80/page", 32, &out), (size_t)0);
    EXPECT_EQ(out.size, (size_t)0);
    EXPECT_STREQ(out.buff, nullptr);
    EXPECT_EQ(::parse_scheme(":ttps://dummy.net:80/page", 32, &out), (size_t)0);
    EXPECT_EQ(out.size, (size_t)0);
    EXPECT_STREQ(out.buff, nullptr);
    EXPECT_EQ(::parse_scheme("h*tps://dummy.net:80/page", 32, &out), (size_t)0);
    EXPECT_EQ(out.size, (size_t)0);
    EXPECT_STREQ(out.buff, nullptr);
    EXPECT_EQ(::parse_scheme("mailto:a@b.com", 7, &out), (size_t)6);
    EXPECT_EQ(out.size, (size_t)6);
    EXPECT_STREQ(out.buff, "mailto:a@b.com");
    EXPECT_EQ(::parse_scheme("mailto:a@b.com", 6, &out), (size_t)0);
    EXPECT_EQ(out.size, (size_t)0);
    EXPECT_STREQ(out.buff, nullptr);
}

TEST(UriIp4TestSuite, parse_absolute_uri) {
    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.set("192.168.10.10", 80);

    // Mock for network address system call
    EXPECT_CALL(netv4inf, getaddrinfo(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(res), Return(0)));
    EXPECT_CALL(netv4inf, freeaddrinfo(_)).Times(1);

    uri_type out;
    struct sockaddr_in* sai4 = (struct sockaddr_in*)&out.hostport.IPaddress;

    // Execute the unit
    Curi uriObj;
#ifdef OLD_TEST
    ::std::cout << "  BUG! parse_uri must fail with too short max size instead "
                   "of returning wrong values!\n";
    EXPECT_EQ(uriObj.parse_uri(
                  "https://example-site.de:80/uri/path?uriquery#urifragment",
                  128, &out),
              HTTP_SUCCESS);
#else
    EXPECT_EQ(uriObj.parse_uri(
                  "https://example-site.de:80/uri/path?uriquery#urifragment", 7,
                  &out),
              UPNP_E_INVALID_URL);
#endif
    // Check the uri-parts scheme, hostport, pathquery and fragment
    EXPECT_STREQ(out.scheme.buff,
                 "https://example-site.de:80/uri/path?uriquery#urifragment");
    EXPECT_STREQ(out.hostport.text.buff,
                 "example-site.de:80/uri/path?uriquery#urifragment");
    EXPECT_STREQ(out.pathquery.buff, "/uri/path?uriquery#urifragment");
    EXPECT_STREQ(out.fragment.buff, "urifragment");

    ::std::cout << "  BUG! Wrong uri type on MS Windows due to conflicting "
                   "ABSOLUTE constant name. See issue #3.\n";
#ifdef _WIN32
    EXPECT_EQ(out.type, 1); // What ever this means.
#else
    EXPECT_EQ(out.type, ABSOLUTE);
#endif
    EXPECT_EQ(out.path_type, ABS_PATH);
    EXPECT_EQ(sai4->sin_family, AF_INET);
    EXPECT_EQ(sai4->sin_port, htons(80));
    EXPECT_STREQ(inet_ntoa(sai4->sin_addr), "192.168.10.10");
}

TEST(UriIp4TestSuite, parse_relative_uri_with_authority_and_absolute_path) {
    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.set("192.168.10.10", 80);

    // Mock for network address system call
    EXPECT_CALL(netv4inf, getaddrinfo(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(res), Return(0)));
    EXPECT_CALL(netv4inf, freeaddrinfo(_)).Times(1);

    uri_type out;
    struct sockaddr_in* sai4 = (struct sockaddr_in*)&out.hostport.IPaddress;

    // Execute the unit
    Curi uriObj;
    EXPECT_EQ(
        uriObj.parse_uri("//example-site.de:80/uri/path?uriquery#urifragment",
                         51, &out),
        HTTP_SUCCESS);
    // Check the uri-parts scheme, hostport, pathquery and fragment
    EXPECT_STREQ(out.scheme.buff, nullptr);
    EXPECT_STREQ(out.hostport.text.buff,
                 "example-site.de:80/uri/path?uriquery#urifragment");
    EXPECT_STREQ(out.pathquery.buff, "/uri/path?uriquery#urifragment");
    EXPECT_STREQ(out.fragment.buff, "urifragment");

    ::std::cout << "  BUG! Wrong uri type on MS Windows due to conflicting "
                   "ABSOLUTE constant name. See issue #3.\n";
#ifdef _WIN32
    EXPECT_EQ(out.type, 2); // What ever this means
#else
    EXPECT_EQ(out.type, RELATIVE);
#endif
    EXPECT_EQ(out.path_type, ABS_PATH);
    EXPECT_EQ(sai4->sin_family, AF_INET);
    EXPECT_EQ(sai4->sin_port, htons(80));
    EXPECT_STREQ(inet_ntoa(sai4->sin_addr), "192.168.10.10");
}

TEST(UriIp4TestSuite, parse_relative_uri_with_absolute_path) {
    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.set("0.0.0.0", 0);

    // Set default return values for network address system call in case we get
    // an unexpected call but it should not occur. An ip address should not be
    // asked for name resolution because we do not have one.
    ON_CALL(netv4inf, getaddrinfo(_, _, _, _))
        .WillByDefault(DoAll(SetArgPointee<3>(res), Return(EAI_NONAME)));
    EXPECT_CALL(netv4inf, getaddrinfo(_, _, _, _)).Times(0);
    EXPECT_CALL(netv4inf, freeaddrinfo(_)).Times(0);

    uri_type out;
    struct sockaddr_in* sai4 = (struct sockaddr_in*)&out.hostport.IPaddress;

    // Execute the unit
    Curi uriObj;
    EXPECT_EQ(uriObj.parse_uri("/uri/path?uriquery#urifragment", 31, &out),
              HTTP_SUCCESS);
    // Check the uri-parts scheme, hostport, pathquery and fragment
    EXPECT_STREQ(out.scheme.buff, nullptr);
    EXPECT_STREQ(out.hostport.text.buff, nullptr);
    EXPECT_STREQ(out.pathquery.buff, "/uri/path?uriquery#urifragment");
    EXPECT_STREQ(out.fragment.buff, "urifragment");

    ::std::cout << "  BUG! Wrong uri type on MS Windows due to conflicting "
                   "ABSOLUTE constant name. See issue #3.\n";
#ifdef _WIN32
    EXPECT_EQ(out.type, 2); // What ever this means.
#else
    EXPECT_EQ(out.type, RELATIVE);
#endif
    EXPECT_EQ(out.path_type, ABS_PATH);
    EXPECT_EQ(sai4->sin_family, AF_UNSPEC);
    EXPECT_EQ(sai4->sin_port, htons(0));
    EXPECT_STREQ(inet_ntoa(sai4->sin_addr), "0.0.0.0");
}

TEST(UriIp4TestSuite, parse_relative_uri_with_relative_path) {
    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.set("0.0.0.0", 0);

    // Set default return values for network address system call in case we get
    // an unexpected call but it should not occur. An ip address should not be
    // asked for name resolution because we do not have one.
    ON_CALL(netv4inf, getaddrinfo(_, _, _, _))
        .WillByDefault(DoAll(SetArgPointee<3>(res), Return(EAI_NONAME)));
    EXPECT_CALL(netv4inf, getaddrinfo(_, _, _, _)).Times(0);
    EXPECT_CALL(netv4inf, freeaddrinfo(_)).Times(0);

    uri_type out;
    struct sockaddr_in* sai4 = (struct sockaddr_in*)&out.hostport.IPaddress;

    // Execute the unit
    // The relative path does not have a leading /
    Curi uriObj;
    EXPECT_EQ(uriObj.parse_uri("uri/path?uriquery#urifragment", 30, &out),
              HTTP_SUCCESS);
    // Check the uri-parts scheme, hostport, pathquery and fragment
    EXPECT_STREQ(out.scheme.buff, nullptr);
    EXPECT_STREQ(out.hostport.text.buff, nullptr);
    EXPECT_STREQ(out.pathquery.buff, "uri/path?uriquery#urifragment");
    EXPECT_STREQ(out.fragment.buff, "urifragment");

    ::std::cout << "  BUG! Wrong uri type on MS Windows due to conflicting "
                   "ABSOLUTE constant name. See issue #3.\n";
#ifdef _WIN32
    EXPECT_EQ(out.type, 2); // What ever this means.
#else
    EXPECT_EQ(out.type, RELATIVE);
#endif
    EXPECT_EQ(out.path_type, REL_PATH);
    EXPECT_EQ(sai4->sin_family, AF_UNSPEC);
    EXPECT_EQ(sai4->sin_port, htons(0));
    EXPECT_STREQ(inet_ntoa(sai4->sin_addr), "0.0.0.0");
}

TEST(UriIp4TestSuite, parse_uri_with_opaque_part) {
    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.set("0.0.0.0", 0);

    // Set default return values for network address system call in case we get
    // an unexpected call but it should not occur. An ip address should not be
    // asked for name resolution because we do not have one.
    ON_CALL(netv4inf, getaddrinfo(_, _, _, _))
        .WillByDefault(DoAll(SetArgPointee<3>(res), Return(EAI_NONAME)));
    EXPECT_CALL(netv4inf, getaddrinfo(_, _, _, _)).Times(0);
    EXPECT_CALL(netv4inf, freeaddrinfo(_)).Times(0);

    uri_type out;
    struct sockaddr_in* sai4 = (struct sockaddr_in*)&out.hostport.IPaddress;

    // Execute the unit
    // The relative path does not have a leading /
    Curi uriObj;
    EXPECT_EQ(uriObj.parse_uri("mailto:a@b.com", 15, &out), HTTP_SUCCESS);
    // Check the uri-parts scheme, hostport, pathquery and fragment
    EXPECT_STREQ(out.scheme.buff, "mailto:a@b.com");
    EXPECT_STREQ(out.hostport.text.buff, nullptr);
    EXPECT_STREQ(out.pathquery.buff, "a@b.com");
    EXPECT_STREQ(out.fragment.buff, nullptr);

    ::std::cout << "  BUG! Wrong uri type on MS Windows due to conflicting "
                   "ABSOLUTE constant name. See issue #3.\n";
#ifdef _WIN32
    EXPECT_EQ(out.type, 1); // What ever this means.
#else
    EXPECT_EQ(out.type, ABSOLUTE);
#endif
    EXPECT_EQ(out.path_type, OPAQUE_PART);
    EXPECT_EQ(sai4->sin_family, AF_UNSPEC);
    EXPECT_EQ(sai4->sin_port, htons(0));
    EXPECT_STREQ(inet_ntoa(sai4->sin_addr), "0.0.0.0");
}

//
// resolve_rel_url() function: tests from the uri module
// =====================================================

TEST(ResolveRelUrlIp4TestSuite, resolve_successful) {
    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.set("192.168.186.186", 443);

    // Mock for network address system call
    EXPECT_CALL(netv4inf, getaddrinfo(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(res), Return(0)));
    EXPECT_CALL(netv4inf, freeaddrinfo(_)).Times(1);

    // Provide arguments to execute the unit
    char base_url[]{"https://example.com:443"};
    char rel_url[]{"homepage#this-fragment"};

    // Execute the unit
    Curi uriObj;
    char* abs_url = uriObj.resolve_rel_url(*&base_url, *&rel_url);
    EXPECT_STREQ(abs_url, "https://example.com:443/homepage#this-fragment");
}

TEST(ResolveRelUrlIp4TestSuite, null_base_url_should_return_rel_url) {
    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.set("0.0.0.0", 0);

    // Mock for network address system call
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
    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.set("0.0.0.0", 0);

    // Mock for network address system call
    ON_CALL(netv4inf, getaddrinfo(_, _, _, _))
        .WillByDefault(DoAll(SetArgPointee<3>(res), Return(EAI_NONAME)));
    EXPECT_CALL(netv4inf, getaddrinfo(_, _, _, _)).Times(0);
    EXPECT_CALL(netv4inf, freeaddrinfo(_)).Times(0);

    // Provide arguments to execute the unit
    char base_url[]{""};
    char rel_url[]{"homepage#this-fragment"};

    // Execute the unit
    Curi uriObj;
    char* abs_url = uriObj.resolve_rel_url(*&base_url, *&rel_url);
#ifdef OLD_TEST
    ::std::cout << "  BUG! Empty base url should return rel url, not NULL.\n";
    EXPECT_EQ(abs_url, nullptr);
#else
    EXPECT_STREQ(abs_url, "homepage#this-fragment");
#endif
}

TEST(ResolveRelUrlIp4TestSuite, null_rel_url_should_return_base_url) {
#ifdef OLD_TEST
    ::std::cout << "  BUG! Segfault if rel_url is NULL.\n";
#else
    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.set("192.168.168.168", 80);

    // Mock for network address system call
    EXPECT_CALL(netv4inf, getaddrinfo(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(res), Return(0)));
    EXPECT_CALL(netv4inf, freeaddrinfo(_)).Times(1);

    // Provide arguments to execute the unit
    char base_url[]{"http://example.com"};

    // Execute the unit
    Curi uriObj;
    char* abs_url = uriObj.resolve_rel_url(*&base_url, NULL);
    EXPECT_STREQ(abs_url, "http://example.com");
#endif
}

TEST(ResolveRelUrlIp4TestSuite, empty_rel_url_should_return_base_url) {
    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.set("192.168.168.168", 80);

    // Mock for network address system call
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

TEST(ResolveRelUrlIp4TestSuite,
     base_and_rel_url_not_absolute_should_return_null) {
    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.set("0.0.0.0", 0);

    // Mock for network address system call
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

} // namespace upnp

int main(int argc, char** argv) {
    // ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
