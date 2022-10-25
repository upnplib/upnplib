// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-10-25
//
// If you need more information how the Url class works you can temporary
// uncomment #define DEBUG_URL and run the tests with
// ./build/gtests/test_url_class --gtest_filter=UrlClass*
// #define DEBUG_URL

//
// To have it available for tests: the following figure displays example URIs
// and their component parts.
// clang-format off
// [syntax diagram](https://en.wikipedia.org/wiki/Uniform_Resource_Identifier#Syntax)
/*
[Example URIs](https://en.wikipedia.org/wiki/Uniform_Resource_Identifier#Example_URIs)

             userinfo           host      port
        ┌───────┴───────┐ ┌──────┴──────┐ ┌┴┐
https://john.doe:password@www.example.com:123/forum/questions/?tag=networking&order=newest#top
└─┬─┘   └──────────────────┬────────────────┘└───────┬───────┘ └───────────┬─────────────┘ └┬┘
scheme                 authority                    path                 query           fragment

ldap://[2001:db8::7]/c=GB?objectClass?one
└┬─┘   └─────┬─────┘└─┬─┘ └──────┬──────┘
scheme   authority   path      query

mailto:John.Doe@example.com
└─┬──┘ └────┬─────────────┘
scheme     path

news:comp.infosystems.www.servers.unix
└┬─┘ └─────────────┬─────────────────┘
scheme            path

tel:+1-816-555-1212
└┬┘ └──────┬──────┘
scheme    path

telnet://192.0.2.16:80/
└─┬──┘   └─────┬─────┘│
scheme     authority  path

urn:oasis:names:specification:docbook:dtd:xml:4.1.2
└┬┘ └──────────────────────┬──────────────────────┘
scheme                    path
*/
// clang-format on

#include "upnplib/src/net/uri/url.cpp"

#include "gmock/gmock.h"

using ::testing::ThrowsMessage;

//
namespace upnplib {
bool github_actions = std::getenv("GITHUB_ACTIONS");

//
TEST(UrlClassTestSuite, is_in_percent_encode_set) {
    EXPECT_TRUE(is_in_userinfo_percent_encode_set('\0'));
    EXPECT_TRUE(is_in_userinfo_percent_encode_set('|'));
    EXPECT_TRUE(is_in_userinfo_percent_encode_set(-1));
    EXPECT_TRUE(is_in_userinfo_percent_encode_set('\x1E'));
    EXPECT_TRUE(is_in_userinfo_percent_encode_set('\x1F'));
    EXPECT_TRUE(is_in_userinfo_percent_encode_set('\x20')); // space
    EXPECT_FALSE(is_in_userinfo_percent_encode_set('\x21'));
    EXPECT_FALSE(is_in_userinfo_percent_encode_set('A'));
}

TEST(UrlClassTestSuite, percent_encode) {
    EXPECT_EQ(UTF8_percent_encode('\0'), "%00");
    EXPECT_EQ(UTF8_percent_encode('\xa'), "%0A");
    EXPECT_EQ(UTF8_percent_encode('|'), "%7C");
    EXPECT_EQ(UTF8_percent_encode(-1), "%FF");
    EXPECT_EQ(UTF8_percent_encode('\x1e'), "%1E");
    EXPECT_EQ(UTF8_percent_encode('\x1f'), "%1F");
    EXPECT_EQ(UTF8_percent_encode(' '), "%20");
    EXPECT_EQ(UTF8_percent_encode('\x21'), "!");
    EXPECT_EQ(UTF8_percent_encode('A'), "A");
    EXPECT_EQ(UTF8_percent_encode('%'), "%");
}

TEST(UrlClassTestSuite, percent_encode_string) {
    EXPECT_EQ(esc_url(""), "");
    EXPECT_EQ(esc_url(" "), " ");
    EXPECT_EQ(esc_url("\x7F"), "%7F");
    EXPECT_EQ(esc_url("\x1Fhttps:\x0D\x0A//p "), "%1Fhttps:%0D%0A//p ");
}

TEST(UrlClassTestSuite, empty_url_object) {
    Url url;

    EXPECT_EQ((std::string)url, "");
    EXPECT_EQ(url.scheme(), "");
    EXPECT_EQ(url.authority(), "");
    EXPECT_EQ(url.username(), "");
    EXPECT_EQ(url.host(), "");
    EXPECT_EQ(url.port(), "");
    EXPECT_EQ(url.port_num(), 0);
    EXPECT_EQ(url.path(), "");
    EXPECT_EQ(url.query(), "");
    EXPECT_EQ(url.fragment(), "");
}

//
class UrlClassFTestSuite : public ::testing::Test {
  private:
    // Original clog pointer stored to recover it with the destructor.
    std::streambuf* m_clog_old;

  protected:
    std::ostringstream m_clogCapt;

    UrlClassFTestSuite() {
        // Redirect clog so we can check its output. Needs #include <sstream>
        m_clog_old = std::clog.rdbuf();
        std::clog.rdbuf(m_clogCapt.rdbuf());
    }

    ~UrlClassFTestSuite() override {
        // Restore clog
        std::clog.rdbuf(m_clog_old);
    }
};

// Parse full base url
// -------------------
TEST_F(UrlClassFTestSuite, parse_full_base_url) {
    Url url;
    url = "Https://John.Döe:Paßwd@www.Œ×ample.com:123/FƟŘŬM/questions/"
          "?tag=Ñetworking&order=newest#tôƥ";
    std::cout << m_clogCapt.str();

    EXPECT_EQ(url.scheme(), "https");
    EXPECT_EQ(url.username(), "john.d%C3%B6e");
#if false
    EXPECT_EQ(url.authority, "john.doe@www.example.com:123");
    EXPECT_EQ(url.auth().host(), "www.example.com");
    EXPECT_EQ(url.auth().port(), "123");
    EXPECT_EQ(url.auth().port_num(), 123);
    EXPECT_EQ(url.path(), "/forum/questions/");
    EXPECT_EQ(url.query(), "tag=networking&order=newest");
    EXPECT_EQ(url.fragment(), "top");
#endif
}

// Tests for clean_and_copy_url_to_input
// -------------------------------------
TEST_F(UrlClassFTestSuite, empty_url_string) {

#if defined _WIN32 && DEBUG
    GTEST_SKIP() << "  # Test needs rework due to pointer boundary asserts "
                    "with DEBUG on WIN32 enabled.";
#else
    std::cout << "  # Test needs rework due to pointer boundary asserts with "
                 "DEBUG on WIN32 enabled.\n";
#endif

    EXPECT_THAT(
        []() {
            Url url;
            url = "";
        },
        ThrowsMessage<std::invalid_argument>("Invalid URL: ''"));

    std::cout << m_clogCapt.str();
    EXPECT_NE(m_clogCapt.str().find("Error: no valid scheme found.\n"),
              std::string::npos);
}

TEST_F(UrlClassFTestSuite, one_space_url) {
    EXPECT_THAT(
        []() {
            Url url;
            url = " ";
        },
        ThrowsMessage<std::invalid_argument>("Invalid URL: ''"));

    std::cout << m_clogCapt.str();
    EXPECT_NE(m_clogCapt.str().find("Error: no valid scheme found.\n"),
              std::string::npos);
}

TEST_F(UrlClassFTestSuite, only_del_char_on_url) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "\x7F";
        },
        ThrowsMessage<std::invalid_argument>("Invalid URL: '%7F'"));

    std::cout << m_clogCapt.str();
    EXPECT_NE(m_clogCapt.str().find("Error: no valid scheme found.\n"),
              std::string::npos);
}

TEST_F(UrlClassFTestSuite, only_sharp_s_on_url) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "ß"; // Two bytes UTF-8 coding
        },
        ThrowsMessage<std::invalid_argument>("Invalid URL: '%C3%9F'"));

    std::cout << m_clogCapt.str();
    EXPECT_NE(m_clogCapt.str().find("Error: no valid scheme found.\n"),
              std::string::npos);
}

TEST_F(UrlClassFTestSuite, only_roman_numerical_ten_on_url) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "Ⅹ"; // Three bytes UTF-8 coding
        },
        ThrowsMessage<std::invalid_argument>("Invalid URL: '%E2%85%A9'"));

    std::cout << m_clogCapt.str();
    EXPECT_NE(m_clogCapt.str().find("Error: no valid scheme found.\n"),
              std::string::npos);
}

TEST_F(UrlClassFTestSuite, url_with_control_chars_and_spaces) {
    Url url;
    // url = "\1 \2a:\x0D//\x0Ag. h\x09.i\x7F\x4 ";
    url = "\1 \2a:\x0D//\x0Ag. h\x09.i\x7\x4 ";

    std::cout << m_clogCapt.str();
    EXPECT_NE(m_clogCapt.str().find("Removed 3 ASCII tab or newline character. "
                                    "Using \"a://g. h.i\" now.\n"),
              std::string::npos);

    EXPECT_EQ((std::string)url, "");
    EXPECT_EQ(url.scheme(), "a");
    EXPECT_EQ(url.authority(), "");
    EXPECT_EQ(url.username(), "");
    EXPECT_EQ(url.host(), "dummy2.host.state");
    EXPECT_EQ(url.port(), "");
    EXPECT_EQ(url.port_num(), 0);
    EXPECT_EQ(url.path(), "");
    EXPECT_EQ(url.query(), "");
    EXPECT_EQ(url.fragment(), "");
}

TEST_F(UrlClassFTestSuite, url_with_null_char) {
    // This is a special case with null terminated strings in C++. It is by
    // design and will end up in a truncated url.
    EXPECT_THAT(
        []() {
            Url url;
            url = "\1 \2h\0ttps://www.example.com";
        },
        ThrowsMessage<std::invalid_argument>("Invalid URL: 'h'"));

    std::cout << m_clogCapt.str();
    EXPECT_NE(m_clogCapt.str().find("Error: no valid scheme found.\n"),
              std::string::npos);
}

TEST_F(UrlClassFTestSuite, convert_url_chars_upper_to_lower) {
    Url url;
    url = "HTTps://Example.COM";
    std::cout << m_clogCapt.str();

    EXPECT_EQ((std::string)url, "");
    EXPECT_EQ(url.scheme(), "https");
    EXPECT_EQ(url.authority(), "");
    EXPECT_EQ(url.username(), "");
    EXPECT_EQ(url.host(), "dummy2.host.state");
    EXPECT_EQ(url.port(), "");
    EXPECT_EQ(url.port_num(), 0);
    EXPECT_EQ(url.path(), "");
    EXPECT_EQ(url.query(), "");
    EXPECT_EQ(url.fragment(), "");
}

// Tests for scheme start state
// ----------------------------
TEST_F(UrlClassFTestSuite, invalid_digit_at_first_pos_in_scheme) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "2ttp://a.b.c";
        },
        ThrowsMessage<std::invalid_argument>("Invalid URL: '2ttp://a.b.c'"));

    std::cout << m_clogCapt.str();
    EXPECT_NE(m_clogCapt.str().find("Error: no valid scheme found.\n"),
              std::string::npos);
}

TEST_F(UrlClassFTestSuite, valid_digits_not_at_first_pos_in_scheme) {
    Url url;
    url = "H9t6://a.b.c";
    std::cout << m_clogCapt.str();

    EXPECT_EQ((std::string)url, "");
    EXPECT_EQ(url.scheme(), "h9t6");
    EXPECT_EQ(url.authority(), "");
    EXPECT_EQ(url.username(), "");
    EXPECT_EQ(url.host(), "dummy2.host.state");
    EXPECT_EQ(url.port(), "");
    EXPECT_EQ(url.port_num(), 0);
    EXPECT_EQ(url.path(), "");
    EXPECT_EQ(url.query(), "");
    EXPECT_EQ(url.fragment(), "");
}

// Tests for scheme state
// ----------------------
TEST_F(UrlClassFTestSuite, scheme_with_invalid_character) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "ftp@x:";
        },
        ThrowsMessage<std::invalid_argument>("Invalid URL: 'ftp@x:'"));

    std::cout << m_clogCapt.str();
    EXPECT_NE(m_clogCapt.str().find("Error: no valid scheme found.\n"),
              std::string::npos);
}

TEST_F(UrlClassFTestSuite, invalid_scheme_without_colon) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "ftp";
        },
        ThrowsMessage<std::invalid_argument>("Invalid URL: 'ftp'"));

    std::cout << m_clogCapt.str();
    EXPECT_NE(m_clogCapt.str().find("Error: no valid scheme found.\n"),
              std::string::npos);
}

TEST_F(UrlClassFTestSuite, file_scheme_without_slashes) {
    Url url;
    url = "file:";

    std::cout << m_clogCapt.str();
    EXPECT_NE(m_clogCapt.str().find(
                  "Warning: 'file' scheme misses \"//\", ignoring.\n"),
              std::string::npos);

    EXPECT_EQ((std::string)url, "");
    EXPECT_EQ(url.scheme(), "file");
    EXPECT_EQ(url.authority(), "");
    EXPECT_EQ(url.username(), "");
    EXPECT_EQ(url.host(), "");
    EXPECT_EQ(url.port(), "");
    EXPECT_EQ(url.port_num(), 0);
    EXPECT_EQ(url.path(), "");
    EXPECT_EQ(url.query(), "");
    EXPECT_EQ(url.fragment(), "");
}

TEST_F(UrlClassFTestSuite, file_scheme_with_slashes) {
    Url url;
    url = "file://";
    std::cout << m_clogCapt.str();

    EXPECT_EQ((std::string)url, "");
    EXPECT_EQ(url.scheme(), "file");
    EXPECT_EQ(url.authority(), "");
    EXPECT_EQ(url.username(), "");
    EXPECT_EQ(url.host(), "");
    EXPECT_EQ(url.port(), "");
    EXPECT_EQ(url.port_num(), 0);
    EXPECT_EQ(url.path(), "");
    EXPECT_EQ(url.query(), "");
    EXPECT_EQ(url.fragment(), "");
}

TEST(UrlClassTestSuite, scheme_with_preloaded_base_url) {
    GTEST_SKIP() << "Create test if base url preload is available.";
    Url url;
    url = "a://b.c";
    url = "a:/d/e";
}

TEST_F(UrlClassFTestSuite, special_but_not_file_scheme_without_slashes) {
    Url url;

    EXPECT_THAT([&url] { url = "ws:"; }, ThrowsMessage<std::invalid_argument>(
                                             "Invalid hostname: 'ws:'"));

    std::cout << m_clogCapt.str();
    EXPECT_NE(
        m_clogCapt.str().find(
            "Warning: no \"//\" before authority: ignoring. Found \"\"\n"),
        std::string::npos);
    EXPECT_EQ(url.scheme(), "ws");
}

TEST_F(UrlClassFTestSuite, special_but_not_file_scheme_with_one_slash) {
    Url url;

    EXPECT_THAT(
        [&url] { url = "http:/"; },
        ThrowsMessage<std::invalid_argument>("Invalid hostname: 'http:/'"));

    std::cout << m_clogCapt.str();
    EXPECT_NE(
        m_clogCapt.str().find(
            "Warning: no \"//\" before authority: ignoring. Found \"/\"\n"),
        std::string::npos);
    EXPECT_EQ(url.scheme(), "http");
}

TEST_F(UrlClassFTestSuite, scheme_with_one_slash) {
    Url url;
    url = "s:/";

    std::cout << m_clogCapt.str();
    EXPECT_EQ(m_clogCapt.str().find("Warning: "), std::string::npos);
    EXPECT_EQ(url.scheme(), "s");
    GTEST_SKIP() << "Improve test with specific expectations.";
}

TEST_F(UrlClassFTestSuite, scheme_with_no_slashes) {
    Url url;
    url = "s:";

    std::cout << m_clogCapt.str();
    EXPECT_EQ(m_clogCapt.str().find("Warning: "), std::string::npos);
    EXPECT_EQ(url.scheme(), "s");
    GTEST_SKIP() << "Improve test with specific expectations.";
}

// Tests for authority state
// -------------------------
/* clang-format off
 * Complete test table
User    Passw   Host    Port
u       p       h       p      valid     s://u:p@h:1/  s://u:p@h:1
u       p       h       -      valid     s://u:p@h:    s://u:p@h
u       p       -       p      invalid   s://u:p@:1    s://u:p:1
u       p       -       -      invalid   s://u:p@      s://u:p
u       -       h       p      valid     s://u:@h:1    s://u@h:1
u       -       h       -      valid     s://u:@h:     s://u:@h    s://u@h:  s://u@h
u       -       -       p      invalid   s://u:@:1     s://u@:1
u       -       -       -      invalid   s://u:@:      s://u:@     s://u@:   s://u@
u       -       -       -      valid     s://u // This is interpreted as hostname only
-       p       h       p      valid     s://:p@h:1
-       p       h       -      valid     s://:p@h:     s://:p@h
-       p       -       p      invalid   s://:p@:1
-       p       -       -      invalid   s://:p@:      s://:p@     s://:p
-       -       h       p      valid     s://:@h:1     s://@h:1
-       -       h       -      valid     s://:@h:      s://:@h     s://@h:   s://@h
-       -       -       p      invalid   s://:@:1      s://@:1
-       -       -       -      invalid   s://:@:       s://:@      s://:     s://@    s://
clang-format on */
typedef UrlClassFTestSuite UrlClassAuthorityFTestSuite;

TEST_F(UrlClassAuthorityFTestSuite, user_pw_host_port_1) {
    Url url;
    url = "s://u:p@h:1/";
    std::cout << m_clogCapt.str();
    EXPECT_EQ(url.username(), "u");
    EXPECT_EQ(url.password(), "p");
    EXPECT_EQ(url.host(), "dummy1.host.state");
    EXPECT_EQ(url.port(), "1");
    EXPECT_EQ(url.port_num(), 1);
}

TEST_F(UrlClassAuthorityFTestSuite, user_pw_host_port_2) {
    Url url;
    url = "s://u:p@h:1";
    std::cout << m_clogCapt.str();
    EXPECT_EQ(url.username(), "u");
    EXPECT_EQ(url.password(), "p");
    EXPECT_EQ(url.host(), "dummy1.host.state");
    EXPECT_EQ(url.port(), "1");
    EXPECT_EQ(url.port_num(), 1);
}

TEST_F(UrlClassAuthorityFTestSuite, user_pw_host_noPort_1) {
    Url url;
    url = "s://u:p@h:";
    std::cout << m_clogCapt.str();
    EXPECT_EQ(url.username(), "u");
    EXPECT_EQ(url.password(), "p");
    EXPECT_EQ(url.host(), "dummy1.host.state");
    EXPECT_EQ(url.port(), "");
    EXPECT_EQ(url.port_num(), NULL);
}

TEST_F(UrlClassAuthorityFTestSuite, user_pw_host_noPort_2) {
    Url url;
    url = "s://u:p@h";
    std::cout << m_clogCapt.str();
    EXPECT_EQ(url.username(), "u");
    EXPECT_EQ(url.password(), "p");
    EXPECT_EQ(url.host(), "dummy2.host.state");
    EXPECT_EQ(url.port(), "");
    EXPECT_EQ(url.port_num(), NULL);
}

TEST_F(UrlClassAuthorityFTestSuite, user_pw_noHost_port_1) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "s://u:p@:1";
        },
        ThrowsMessage<std::invalid_argument>("Invalid hostname: 's://u:p@:1'"));
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassAuthorityFTestSuite, user_pw_noHost_port_2) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "s://u:p:1";
        },
        ThrowsMessage<std::invalid_argument>("Invalid port: 's://u:p:1'"));
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassAuthorityFTestSuite, user_pw_noHost_noPort_1) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "s://u:p@";
        },
        ThrowsMessage<std::invalid_argument>("Invalid authority: 's://u:p@'"));
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassAuthorityFTestSuite, user_pw_noHost_noPort_2) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "s://u:p";
        },
        ThrowsMessage<std::invalid_argument>("Invalid port: 's://u:p'"));
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassAuthorityFTestSuite, user_noPw_host_port_1) {
    Url url;
    url = "s://u:@h:1";
    std::cout << m_clogCapt.str();
    EXPECT_EQ(url.username(), "u");
    EXPECT_EQ(url.password(), "");
    EXPECT_EQ(url.host(), "dummy1.host.state");
    EXPECT_EQ(url.port(), "1");
    EXPECT_EQ(url.port_num(), 1);
}

TEST_F(UrlClassAuthorityFTestSuite, user_noPw_host_port_2) {
    Url url;
    url = "s://u@h:1";
    std::cout << m_clogCapt.str();
    EXPECT_EQ(url.username(), "u");
    EXPECT_EQ(url.password(), "");
    EXPECT_EQ(url.host(), "dummy1.host.state");
    EXPECT_EQ(url.port(), "1");
    EXPECT_EQ(url.port_num(), 1);
}

TEST_F(UrlClassAuthorityFTestSuite, user_noPw_host_noPort_1) {
    Url url;
    url = "s://u:@h:";
    std::cout << m_clogCapt.str();
    EXPECT_EQ(url.username(), "u");
    EXPECT_EQ(url.password(), "");
    EXPECT_EQ(url.host(), "dummy1.host.state");
    EXPECT_EQ(url.port(), "");
    EXPECT_EQ(url.port_num(), NULL);
}

TEST_F(UrlClassAuthorityFTestSuite, user_noPw_host_noPort_2) {
    Url url;
    url = "s://u:@h";
    std::cout << m_clogCapt.str();
    EXPECT_EQ(url.username(), "u");
    EXPECT_EQ(url.password(), "");
    EXPECT_EQ(url.host(), "dummy2.host.state");
    EXPECT_EQ(url.port(), "");
    EXPECT_EQ(url.port_num(), NULL);
}

TEST_F(UrlClassAuthorityFTestSuite, user_noPw_host_noPort_3) {
    Url url;
    url = "s://u@h:";
    std::cout << m_clogCapt.str();
    EXPECT_EQ(url.username(), "u");
    EXPECT_EQ(url.password(), "");
    EXPECT_EQ(url.host(), "dummy1.host.state");
    EXPECT_EQ(url.port(), "");
    EXPECT_EQ(url.port_num(), NULL);
}

TEST_F(UrlClassAuthorityFTestSuite, user_noPw_host_noPort_4) {
    Url url;
    url = "s://u@h";
    std::cout << m_clogCapt.str();
    EXPECT_EQ(url.username(), "u");
    EXPECT_EQ(url.password(), "");
    EXPECT_EQ(url.host(), "dummy2.host.state");
    EXPECT_EQ(url.port(), "");
    EXPECT_EQ(url.port_num(), NULL);
}

TEST_F(UrlClassAuthorityFTestSuite, user_noPw_noHost_Port_1) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "s://u:@:1";
        },
        ThrowsMessage<std::invalid_argument>("Invalid hostname: 's://u:@:1'"));
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassAuthorityFTestSuite, user_noPw_noHost_Port_2) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "s://u@:1";
        },
        ThrowsMessage<std::invalid_argument>("Invalid hostname: 's://u@:1'"));
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassAuthorityFTestSuite, user_noPw_noHost_noPort_1) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "s://u:@:";
        },
        ThrowsMessage<std::invalid_argument>("Invalid hostname: 's://u:@:'"));
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassAuthorityFTestSuite, user_noPw_noHost_noPort_2) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "s://u:@";
        },
        ThrowsMessage<std::invalid_argument>("Invalid authority: 's://u:@'"));
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassAuthorityFTestSuite, user_noPw_noHost_noPort_3) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "s://u@:";
        },
        ThrowsMessage<std::invalid_argument>("Invalid hostname: 's://u@:'"));
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassAuthorityFTestSuite, user_noPw_noHost_noPort_4) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "s://u@";
        },
        ThrowsMessage<std::invalid_argument>("Invalid authority: 's://u@'"));
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassAuthorityFTestSuite, user_noPw_noHost_noPort_valid) {
    // This url is interpreted as hostname only
    Url url;
    url = "s://u";
    std::cout << m_clogCapt.str();
    EXPECT_EQ(url.username(), "");
    EXPECT_EQ(url.password(), "");
    EXPECT_EQ(url.host(), "dummy2.host.state");
    EXPECT_EQ(url.port(), "");
    EXPECT_EQ(url.port_num(), NULL);
}

TEST_F(UrlClassAuthorityFTestSuite, noUser_pw_host_port_1) {
    Url url;
    url = "s://:p@h:1";
    std::cout << m_clogCapt.str();
    EXPECT_EQ(url.username(), "");
    EXPECT_EQ(url.password(), "p");
    EXPECT_EQ(url.host(), "dummy1.host.state");
    EXPECT_EQ(url.port(), "1");
    EXPECT_EQ(url.port_num(), 1);
}

TEST_F(UrlClassAuthorityFTestSuite, noUser_pw_host_noPort_1) {
    Url url;
    url = "s://:p@h:";
    std::cout << m_clogCapt.str();
    EXPECT_EQ(url.username(), "");
    EXPECT_EQ(url.password(), "p");
    EXPECT_EQ(url.host(), "dummy1.host.state");
    EXPECT_EQ(url.port(), "");
    EXPECT_EQ(url.port_num(), NULL);
}

TEST_F(UrlClassAuthorityFTestSuite, noUser_pw_host_noPort_2) {
    Url url;
    url = "s://:p@h";
    std::cout << m_clogCapt.str();
    EXPECT_EQ(url.username(), "");
    EXPECT_EQ(url.password(), "p");
    EXPECT_EQ(url.host(), "dummy2.host.state");
    EXPECT_EQ(url.port(), "");
    EXPECT_EQ(url.port_num(), NULL);
}

TEST_F(UrlClassAuthorityFTestSuite, noUser_pw_noHost_port_1) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "s://:p@:1";
        },
        ThrowsMessage<std::invalid_argument>("Invalid hostname: 's://:p@:1'"));
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassAuthorityFTestSuite, noUser_pw_noHost_noPort_1) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "s://:p@:";
        },
        ThrowsMessage<std::invalid_argument>("Invalid hostname: 's://:p@:'"));
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassAuthorityFTestSuite, noUser_pw_noHost_noPort_2) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "s://:p@";
        },
        ThrowsMessage<std::invalid_argument>("Invalid authority: 's://:p@'"));
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassAuthorityFTestSuite, noUser_pw_noHost_noPort_3) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "s://:p";
        },
        ThrowsMessage<std::invalid_argument>("Invalid hostname: 's://:p'"));
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassAuthorityFTestSuite, noUser_noPw_host_port_1) {
    Url url;
    url = "s://:@h:1";
    std::cout << m_clogCapt.str();
    EXPECT_EQ(url.username(), "");
    EXPECT_EQ(url.password(), "");
    EXPECT_EQ(url.host(), "dummy1.host.state");
    EXPECT_EQ(url.port(), "1");
    EXPECT_EQ(url.port_num(), 1);
}

TEST_F(UrlClassAuthorityFTestSuite, noUser_noPw_host_port_2) {
    Url url;
    url = "s://@h:1";
    std::cout << m_clogCapt.str();
    EXPECT_EQ(url.username(), "");
    EXPECT_EQ(url.password(), "");
    EXPECT_EQ(url.host(), "dummy1.host.state");
    EXPECT_EQ(url.port(), "1");
    EXPECT_EQ(url.port_num(), 1);
}

TEST_F(UrlClassAuthorityFTestSuite, noUser_noPw_host_noPort_1) {
    Url url;
    url = "s://:@h:";
    std::cout << m_clogCapt.str();
    EXPECT_EQ(url.username(), "");
    EXPECT_EQ(url.password(), "");
    EXPECT_EQ(url.host(), "dummy1.host.state");
    EXPECT_EQ(url.port(), "");
    EXPECT_EQ(url.port_num(), NULL);
}

TEST_F(UrlClassAuthorityFTestSuite, noUser_noPw_host_noPort_2) {
    Url url;
    url = "s://:@h";
    std::cout << m_clogCapt.str();
    EXPECT_EQ(url.username(), "");
    EXPECT_EQ(url.password(), "");
    EXPECT_EQ(url.host(), "dummy2.host.state");
    EXPECT_EQ(url.port(), "");
    EXPECT_EQ(url.port_num(), NULL);
}

TEST_F(UrlClassAuthorityFTestSuite, noUser_noPw_host_noPort_3) {
    Url url;
    url = "s://@h:";
    std::cout << m_clogCapt.str();
    EXPECT_EQ(url.username(), "");
    EXPECT_EQ(url.password(), "");
    EXPECT_EQ(url.host(), "dummy1.host.state");
    EXPECT_EQ(url.port(), "");
    EXPECT_EQ(url.port_num(), NULL);
}

TEST_F(UrlClassAuthorityFTestSuite, noUser_noPw_host_noPort_4) {
    Url url;
    url = "s://@h";
    std::cout << m_clogCapt.str();
    EXPECT_EQ(url.username(), "");
    EXPECT_EQ(url.password(), "");
    EXPECT_EQ(url.host(), "dummy2.host.state");
    EXPECT_EQ(url.port(), "");
    EXPECT_EQ(url.port_num(), NULL);
}

TEST_F(UrlClassAuthorityFTestSuite, noUser_noPw_noHost_port_1) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "s://:@:1";
        },
        ThrowsMessage<std::invalid_argument>("Invalid hostname: 's://:@:1'"));
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassAuthorityFTestSuite, noUser_noPw_noHost_port_2) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "s://@:1";
        },
        ThrowsMessage<std::invalid_argument>("Invalid hostname: 's://@:1'"));
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassAuthorityFTestSuite, noUser_noPw_noHost_noPort_1) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "s://:@:";
        },
        ThrowsMessage<std::invalid_argument>("Invalid hostname: 's://:@:'"));
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassAuthorityFTestSuite, noUser_noPw_noHost_noPort_2) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "s://:@";
        },
        ThrowsMessage<std::invalid_argument>("Invalid authority: 's://:@'"));
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassAuthorityFTestSuite, noUser_noPw_noHost_noPort_3) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "s://:";
        },
        ThrowsMessage<std::invalid_argument>("Invalid hostname: 's://:'"));
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassAuthorityFTestSuite, noUser_noPw_noHost_noPort_4) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "s://@";
        },
        ThrowsMessage<std::invalid_argument>("Invalid authority: 's://@'"));
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassAuthorityFTestSuite, noUser_noPw_noHost_noPort_5) {
    Url url;
    url = "s://";

    std::cout << m_clogCapt.str();
    EXPECT_EQ(m_clogCapt.str().find("Warning: "), std::string::npos);
    EXPECT_EQ(url.username(), "");
    GTEST_SKIP() << "Improve test with specific expectations.";
}

} // namespace upnplib

//
int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
