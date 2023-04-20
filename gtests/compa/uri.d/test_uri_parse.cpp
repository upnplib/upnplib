// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-04-28

// Helpful link for ip address structures:
// https://stackoverflow.com/a/16010670/5014688

// Include source code for testing. So we have also direct access to static
// functions which need to be tested.
#include "pupnp/upnp/src/genlib/net/uri/uri.cpp"

#include "upnplib/upnptools.hpp"
#include "upnplib/uri.hpp"
#include "umock/netdb_mock.hpp"

using ::testing::_;
using ::testing::AnyOf;
using ::testing::DoAll;
using ::testing::Eq;
using ::testing::Return;
using ::testing::SetArgPointee;

using ::upnplib::Curi;
using ::upnplib::errStrEx;

namespace compa {

bool old_code{false}; // Managed in compa/gtest_main.inc
bool github_actions = ::std::getenv("GITHUB_ACTIONS");

//
// Mocking
// =======
class Mock_netv4info : public umock::NetdbMock {
    // This is a derived class from mocking netdb to provide a structure for
    // addrinfo that can be given to the mocked program.
  private:
    // Provide structures to mock system call for network address
    struct sockaddr_in m_sa {};
    struct addrinfo m_res {};

  public:
    Mock_netv4info() { m_sa.sin_family = AF_INET; }

    addrinfo* get(const char* a_ipaddr, uint16_t a_port) {
        inet_pton(m_sa.sin_family, a_ipaddr, &m_sa.sin_addr);
        m_sa.sin_port = htons(a_port);

        m_res.ai_family = m_sa.sin_family;
        m_res.ai_addrlen = sizeof(struct sockaddr);
        m_res.ai_addr = (sockaddr*)&m_sa;

        return &m_res;
    }
};

//
// parse_uri() function: tests from the uri module
// ===============================================
#if 0
TEST(ParseUriIp4TestSuite, simple_call) {
    // This test is only for humans to get an idea what's going on. If you need
    // it, set '#if true' only temporary. It is not intended to be permanent
    // part of the tests. It doesn't really test things and because unmocked, it
    // queries DNS server on the internet that may have long delays.

    const char* uri_str{"scheme://example-site.de:80/uripath?uriquery#urifragment"};
    // const char* uri_str{"mailto:a@b.com"};
    uri_type out;
    Curi uriObj;

    // Test Unit
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

TEST(ParseUriIp4TestSuite, absolute_uri_successful) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.get("192.168.10.10", 80);

    // Mock for network address system calls. parse_uri() asks the DNS server.
    umock::Netdb netdb_injectObj(&netv4inf);
    EXPECT_CALL(netv4inf, getaddrinfo(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(res), Return(0)));
    EXPECT_CALL(netv4inf, freeaddrinfo(_)).Times(1);

    uri_type out;
    memset(&out, 0xaa, sizeof(out));

    // Test Unit
    char url_str[]{"https://example-site.de:80/uri/path?uriquery#urifragment"};
    Curi uriObj;
    int returned{UPNP_E_INTERNAL_ERROR};
    EXPECT_EQ(returned = uriObj.parse_uri(url_str, strlen(url_str), &out),
              HTTP_SUCCESS)
        << errStrEx(returned, HTTP_SUCCESS);

    // Check the uri-parts scheme, hostport, pathquery and fragment. Please note
    // that the last part of the buffer content is garbage. The valid character
    // chain is determined by its size. But it's no problem to compare the whole
    // buffer because it's defined to contain a C string.
    EXPECT_STREQ(out.scheme.buff,
                 "https://example-site.de:80/uri/path?uriquery#urifragment");
    EXPECT_EQ(out.scheme.size, (size_t)5);

    // The token.buff pointer just only point into the original url_str:
    url_str[8] = 'X';
    EXPECT_STREQ(out.scheme.buff,
                 "https://Xxample-site.de:80/uri/path?uriquery#urifragment");
    //                    ^ there is a 'X' now

    EXPECT_STREQ(out.hostport.text.buff,
                 "Xxample-site.de:80/uri/path?uriquery#urifragment");
    EXPECT_EQ(out.hostport.text.size, (size_t)18);

    EXPECT_STREQ(out.pathquery.buff, "/uri/path?uriquery#urifragment");
    EXPECT_EQ(out.pathquery.size, (size_t)18);

    EXPECT_STREQ(out.fragment.buff, "urifragment");
    EXPECT_EQ(out.fragment.size, (size_t)11);

    ::std::cout << "  BUG! Wrong uri type 'relative' on MS Windows due to "
                   "conflicting 'ABSOLUTE' constant name. See issue #3.\n";
#ifdef _WIN32
    if (old_code) {
        EXPECT_EQ(out.type, relative);
    } else {
        EXPECT_EQ(out.type, ABSOLUTE);
    }
#else
    EXPECT_EQ(out.type, ABSOLUTE);
#endif
    EXPECT_EQ(out.path_type, ABS_PATH);

    const sockaddr_in* sai4 = (struct sockaddr_in*)&out.hostport.IPaddress;
    EXPECT_EQ(sai4->sin_family, AF_INET);
    EXPECT_EQ(sai4->sin_port, htons(80));
    EXPECT_STREQ(::inet_ntoa(sai4->sin_addr), "192.168.10.10");
}

TEST(ParseUriIp4TestSuite, absolute_uri_with_shorter_max_size) {
    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.get("192.168.10.10", 80);

    // Mock for network address system calls. parse_uri() asks the DNS server.
    umock::Netdb netdb_injectObj(&netv4inf);
    EXPECT_CALL(netv4inf, getaddrinfo(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(res), Return(0)));
    EXPECT_CALL(netv4inf, freeaddrinfo(_)).Times(1);

    uri_type out;
    memset(&out, 0xaa, sizeof(out));
    struct sockaddr_in* sai4 = (struct sockaddr_in*)&out.hostport.IPaddress;

    // Test Unit
    constexpr char url_str[57]{
        "https://example-site.de:80/uri/path?uriquery#urifragment"};

    // This is by 17 chars too short for the whole url (without '\0'). It will
    // split pathquery.
    constexpr size_t max_size = 57 - 1 - 17;

    Curi uriObj;
    int returned{UPNP_E_INTERNAL_ERROR};
    EXPECT_EQ(returned = uriObj.parse_uri(url_str, max_size, &out),
              HTTP_SUCCESS)
        << errStrEx(returned, HTTP_SUCCESS);

    // Check the uri-parts scheme, hostport, pathquery and fragment. Please note
    // that the last part of the buffer content is garbage. The valid character
    // chain is determined by its size. But it's no problem to compare the whole
    // buffer because it's defined to contain a C string.
    EXPECT_STREQ(out.scheme.buff,
                 "https://example-site.de:80/uri/path?uriquery#urifragment");
    EXPECT_EQ(out.scheme.size, (size_t)5);

    EXPECT_STREQ(out.hostport.text.buff,
                 "example-site.de:80/uri/path?uriquery#urifragment");
    EXPECT_EQ(out.hostport.text.size, (size_t)18);

    EXPECT_STREQ(out.pathquery.buff, "/uri/path?uriquery#urifragment");
    EXPECT_EQ(out.pathquery.size, (size_t)13);

    EXPECT_STREQ(out.fragment.buff, nullptr);
    EXPECT_EQ(out.fragment.size, (size_t)0);
#ifdef _WIN32
    EXPECT_EQ(out.type, relative); // This bug is reported by another gtest.
#else
    EXPECT_EQ(out.type, ABSOLUTE);
#endif
    EXPECT_EQ(out.path_type, ABS_PATH);
    EXPECT_EQ(sai4->sin_family, AF_INET);
    EXPECT_EQ(sai4->sin_port, htons(80));
    EXPECT_STREQ(inet_ntoa(sai4->sin_addr), "192.168.10.10");
}

TEST(ParseUriIp4TestSuite, ip_address_with_greater_max_size) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    // Mock for network address system calls. Url with ip address does not need
    // to query for a network address.
    Mock_netv4info netv4inf;
    umock::Netdb netdb_injectObj(&netv4inf);
    EXPECT_CALL(netv4inf, getaddrinfo(_, _, _, _)).Times(0);
    EXPECT_CALL(netv4inf, freeaddrinfo(_)).Times(0);

    uri_type out;
    memset(&out, 0xaa, sizeof(out));

    // Test Unit
    constexpr char url_str[57]{
        "https://192.168.168.168:80/uri/path?uriquery#urifragment"};

    // This is by 10 chars longer than the whole url (without '\0'). It will
    // increase the fragment with garbage at the end.
    constexpr size_t max_size = 57 - 1 + 10;

    Curi uriObj;
    int returned{UPNP_E_INTERNAL_ERROR};
    EXPECT_EQ(returned = uriObj.parse_uri(url_str, max_size, &out),
              HTTP_SUCCESS)
        << errStrEx(returned, HTTP_SUCCESS);

    // Check the uri-parts scheme, hostport, pathquery and fragment. Please note
    // that the last part of the buffer content is garbage. The valid character
    // chain is determined by its size. But it's no problem to compare the whole
    // buffer because it's defined to contain a C string.
    EXPECT_STREQ(out.scheme.buff,
                 "https://192.168.168.168:80/uri/path?uriquery#urifragment");
    EXPECT_EQ(out.scheme.size, (size_t)5);

    EXPECT_STREQ(out.hostport.text.buff,
                 "192.168.168.168:80/uri/path?uriquery#urifragment");
    EXPECT_EQ(out.hostport.text.size, (size_t)18);

    EXPECT_STREQ(out.pathquery.buff, "/uri/path?uriquery#urifragment");
    EXPECT_EQ(out.pathquery.size, (size_t)18);
#ifdef _WIN32
    EXPECT_EQ(out.type, relative); // This bug is reported by another gtest.
#else
    EXPECT_EQ(out.type, ABSOLUTE);
#endif
    EXPECT_EQ(out.path_type, ABS_PATH);

    struct sockaddr_in* sai4 = (struct sockaddr_in*)&out.hostport.IPaddress;
    EXPECT_EQ(sai4->sin_family, AF_INET);
    EXPECT_EQ(sai4->sin_port, htons(80));
    EXPECT_STREQ(inet_ntoa(sai4->sin_addr), "192.168.168.168");

    EXPECT_STREQ(out.fragment.buff, "urifragment");

    // The fragment size differs on different operating systems and seems to
    // be undefined with max_size > strlen(url_str). That is a reason to
    // only use strlen(url_str) as max_size as workaround so far.
    if (old_code) {
        ::std::cout << "  BUG! Fragment size should be 11 (current="
                    << out.fragment.size
                    << ") and not different on other OS.\n";
        EXPECT_GE(out.fragment.size, (size_t)11);

    } else {

        ASSERT_EQ(out.fragment.size, (size_t)11)
            << "  # Fragment size should be 11 and not different on other OS.";
        GTEST_FAIL()
            << "  # Fragment size of 11 should not differ on different OS.";
    }
}

TEST(ParseUriIp4TestSuite, uri_without_valid_host_and_port) {
    // This test will not segfault with a null initialized 'url' structure for
    // the output of 'parse_uri()'.

    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    constexpr char url_str[] = "http://upnplib.net:80/path/?key=value#fragment";
    uri_type url;
    memset(&url, 0xaa, sizeof(uri_type));

    // Mock for network address system calls, parse_uri() ask DNS server.
    Mock_netv4info netv4inf;
    umock::Netdb netdb_injectObj(&netv4inf);
    EXPECT_CALL(netv4inf, getaddrinfo(_, _, _, _)).WillOnce(Return(EAI_NONAME));
    EXPECT_CALL(netv4inf, freeaddrinfo(_)).Times(0);

    // Get a uri structure with parse_uri(). It fails with NONAME to get a valid
    // host & port.
    Curi uriObj;
    int returned{UPNP_E_INTERNAL_ERROR};
    EXPECT_EQ(returned = uriObj.parse_uri(url_str, strlen(url_str), &url),
              UPNP_E_INVALID_URL)
        << errStrEx(returned, HTTP_SUCCESS);

#ifdef _WIN32
    EXPECT_EQ(url.type, relative); // This bug is reported by another gtest.
#else
    EXPECT_EQ(url.type, ABSOLUTE);
#endif
    EXPECT_EQ(url.path_type, OPAQUE_PART);

    EXPECT_STREQ(url.scheme.buff,
                 "http://upnplib.net:80/path/?key=value#fragment");
    EXPECT_EQ(url.scheme.size, (size_t)4);

    EXPECT_STREQ(url.hostport.text.buff, nullptr);
    EXPECT_EQ(url.hostport.text.size, (size_t)0);

    if (old_code) {
        ::std::cout << "  BUG! Fail to get a valid host and port must not "
                       "segfault on reading url struct.\n";

    } else {

        GTEST_FAIL() << "  # Fail to get a valid host and port must not "
                        "segfault on reading url struct.";

        EXPECT_STREQ(url.pathquery.buff, "/path/?key=value#fragment");
        EXPECT_EQ(url.pathquery.size, (size_t)16);

        EXPECT_STREQ(url.fragment.buff, "fragment");
        EXPECT_EQ(url.fragment.size, (size_t)8);
    }

    const sockaddr_in* sai4 = (struct sockaddr_in*)&url.hostport.IPaddress;
    EXPECT_EQ(sai4->sin_family, 0);
    EXPECT_EQ(sai4->sin_port, 0);
    EXPECT_STREQ(inet_ntoa(sai4->sin_addr), "0.0.0.0");
}

TEST(ParseUriIp4TestSuite, ip_address_without_pathquery) {
    uri_type out;
    memset(&out, 0xaa, sizeof(out));
    struct sockaddr_in* sai4 = (struct sockaddr_in*)&out.hostport.IPaddress;

    // Test Unit
    constexpr char url_str[]{"https://192.168.168.168:80#urifragment"};

    Curi uriObj;
    int returned{UPNP_E_INTERNAL_ERROR};
    EXPECT_EQ(returned = uriObj.parse_uri(url_str, strlen(url_str), &out),
              HTTP_SUCCESS)
        << errStrEx(returned, HTTP_SUCCESS);

    // Check the uri-parts scheme, hostport, pathquery and fragment. Please note
    // that the last part of the buffer content is garbage. The valid character
    // chain is determined by its size. But it's no problem to compare the whole
    // buffer because it's defined to contain a C string.
    EXPECT_STREQ(out.scheme.buff, "https://192.168.168.168:80#urifragment");
    EXPECT_EQ(out.scheme.size, (size_t)5);

    EXPECT_STREQ(out.hostport.text.buff, "192.168.168.168:80#urifragment");
    EXPECT_EQ(out.hostport.text.size, (size_t)18);

    EXPECT_STREQ(out.pathquery.buff, "#urifragment");
    EXPECT_EQ(out.pathquery.size, (size_t)0);

    EXPECT_STREQ(out.fragment.buff, "urifragment");
    EXPECT_EQ(out.fragment.size, (size_t)11);
#ifdef _WIN32
    EXPECT_EQ(out.type, relative); // This bug is reported by another gtest.
#else
    EXPECT_EQ(out.type, ABSOLUTE);
#endif
    EXPECT_EQ(out.path_type, OPAQUE_PART);
    EXPECT_EQ(sai4->sin_family, AF_INET);
    EXPECT_EQ(sai4->sin_port, htons(80));
    EXPECT_STREQ(inet_ntoa(sai4->sin_addr), "192.168.168.168");
}

TEST(ParseUriIp4TestSuite, ip_address_without_fragment) {
    constexpr char url_str[] = "http://192.168.167.166:80/path/?key=value";
    uri_type out;
    memset(&out, 0xaa, sizeof(out));

    // Test Unit
    Curi uriObj;
    int returned{UPNP_E_INTERNAL_ERROR};
    EXPECT_EQ(returned = uriObj.parse_uri(url_str, strlen(url_str), &out),
              HTTP_SUCCESS)
        << errStrEx(returned, HTTP_SUCCESS);

    // Check the uri-parts scheme, hostport, pathquery and fragment. Please note
    // that the last part of the buffer content is garbage. The valid character
    // chain is determined by its size. But it's no problem to compare the whole
    // buffer because it's defined to contain a C string.
    EXPECT_STREQ(out.scheme.buff, "http://192.168.167.166:80/path/?key=value");
    EXPECT_EQ(out.scheme.size, (size_t)4);

    EXPECT_STREQ(out.hostport.text.buff, "192.168.167.166:80/path/?key=value");
    EXPECT_EQ(out.hostport.text.size, (size_t)18);

    EXPECT_STREQ(out.pathquery.buff, "/path/?key=value");
    EXPECT_EQ(out.pathquery.size, (size_t)16);

    EXPECT_STREQ(out.fragment.buff, nullptr);
    EXPECT_EQ(out.fragment.size, (size_t)0);
#ifdef _WIN32
    EXPECT_EQ(out.type, relative); // This bug is reported by another gtest.
#else
    EXPECT_EQ(out.type, ABSOLUTE);
#endif
    EXPECT_EQ(out.path_type, ABS_PATH);

    struct sockaddr_in* sai4 = (struct sockaddr_in*)&out.hostport.IPaddress;
    EXPECT_EQ(sai4->sin_family, AF_INET);
    EXPECT_EQ(sai4->sin_port, htons(80));
    EXPECT_STREQ(inet_ntoa(sai4->sin_addr), "192.168.167.166");
}

TEST(ParseUriIp4TestSuite, parse_scheme_of_uri) {
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

TEST(ParseUriIp4TestSuite, relative_uri_with_authority_and_absolute_path) {
    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.get("192.168.10.10", 80);

    // Mock for network address system call
    umock::Netdb netdb_injectObj(&netv4inf);
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

TEST(ParseUriIp4TestSuite, relative_uri_with_absolute_path) {
    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.get("0.0.0.0", 0);

    // Set default return values for network address system call in case we get
    // an unexpected call but it should not occur. An ip address should not be
    // asked for name resolution because we do not have one.
    umock::Netdb netdb_injectObj(&netv4inf);
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

TEST(ParseUriIp4TestSuite, relative_uri_with_relative_path) {
    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.get("0.0.0.0", 0);

    // Set default return values for network address system call in case we get
    // an unexpected call but it should not occur. An ip address should not be
    // asked for name resolution because we do not have one.
    umock::Netdb netdb_injectObj(&netv4inf);
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

TEST(ParseUriIp4TestSuite, uri_with_opaque_part) {
    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.get("0.0.0.0", 0);

    // Set default return values for network address system call in case we get
    // an unexpected call but it should not occur. An ip address should not be
    // asked for name resolution because we do not have one.
    umock::Netdb netdb_injectObj(&netv4inf);
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

} // namespace compa

int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "compa/gtest_main.inc"
    return gtest_return_code; // managed in compa/gtest_main.inc
}
