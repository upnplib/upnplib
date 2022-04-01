// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-04-01
//
// If you need more information how the Url class works you can temporary
// uncomment #define DEBUG_URL and run the tests with
// ./build/gtests/test_url_class --gtest_filter=UrlClass*
#define DEBUG_URL

#include "gmock/gmock.h"
#include <sstream>

#include "core/src/genlib/net/uri/url.cpp"

using ::testing::HasSubstr;
using ::testing::ThrowsMessage;

//
namespace upnplib {
// bool github_actions = std::getenv("GITHUB_ACTIONS");

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
    EXPECT_EQ(url.host(), "");
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
    EXPECT_EQ(url.host(), "");
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
    EXPECT_EQ(url.host(), "");
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
    url = "ws:";

    std::cout << m_clogCapt.str();
    EXPECT_NE(
        m_clogCapt.str().find(
            "Warning: no \"//\" before authority: ignoring. Found \"\"\n"),
        std::string::npos);
    EXPECT_EQ(url.scheme(), "ws");
}

TEST_F(UrlClassFTestSuite, special_but_not_file_scheme_with_one_slash) {
    Url url;
    url = "http:/";

    std::cout << m_clogCapt.str();
    EXPECT_NE(
        m_clogCapt.str().find(
            "Warning: no \"//\" before authority: ignoring. Found \"/\"\n"),
        std::string::npos);
    EXPECT_EQ(url.scheme(), "http");
}

TEST_F(UrlClassFTestSuite, scheme_with_two_slashes) {
    Url url;
    url = "s://";

    std::cout << m_clogCapt.str();
    EXPECT_EQ(m_clogCapt.str().find("Warning: "), std::string::npos);
    EXPECT_EQ(url.scheme(), "s");
    EXPECT_EQ(url.username(), "");
    GTEST_SKIP() << "Improve test with specific expectations.";
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
// Test with empty authority "s://" already done with 'scheme_with_two_slashes'

TEST_F(UrlClassFTestSuite, authority_empty_userinfo) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "s://@";
        },
        ThrowsMessage<std::invalid_argument>("Invalid authority: 's://@'"));
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassFTestSuite, authority_empty_username_and_password) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "s://:@";
        },
        ThrowsMessage<std::invalid_argument>("Invalid authority: 's://:@'"));
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassFTestSuite, authority_username_but_empty_password) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "s://u:@";
        },
        ThrowsMessage<std::invalid_argument>("Invalid authority: 's://u:@'"));
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassFTestSuite, authority_empty_username_but_password) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "s://:p@";
        },
        ThrowsMessage<std::invalid_argument>("Invalid authority: 's://:p@'"));
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassFTestSuite, authority_username_and_password) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "s://u:p@";
        },
        ThrowsMessage<std::invalid_argument>("Invalid authority: 's://u:p@'"));
    std::cout << m_clogCapt.str();
}

// Tests for host state
// --------------------
TEST_F(UrlClassFTestSuite, host_empty_userinfo) {
    Url url;
    url = "s://@h";
    std::cout << m_clogCapt.str();
    EXPECT_EQ(url.scheme(), "s");
    EXPECT_EQ(url.username(), "");
    // EXPECT_EQ(url.host(), "h");
    EXPECT_EQ(url.port_num(), 0);
}

TEST_F(UrlClassFTestSuite, host_empty_username_and_password) {
    Url url;
    url = "s://:@h";
    std::cout << m_clogCapt.str();
    EXPECT_EQ(url.scheme(), "s");
    EXPECT_EQ(url.username(), "");
    // EXPECT_EQ(url.host(), "h");
    EXPECT_EQ(url.port_num(), 0);
}

TEST_F(UrlClassFTestSuite, host_username_but_empty_password) {
    Url url;
    url = "s://u:@h";
    std::cout << m_clogCapt.str();
    EXPECT_EQ(url.scheme(), "s");
    EXPECT_EQ(url.username(), "u");
    // EXPECT_EQ(url.host(), "h");
    EXPECT_EQ(url.port_num(), 0);
}

TEST_F(UrlClassFTestSuite, host_empty_username_but_password) {
    Url url;
    url = "s://:p@h";
    std::cout << m_clogCapt.str();
    EXPECT_EQ(url.scheme(), "s");
    EXPECT_EQ(url.username(), "");
    // EXPECT_EQ(url.host(), "h");
    EXPECT_EQ(url.port_num(), 0);
}

TEST_F(UrlClassFTestSuite, host_username_and_password) {
    Url url;
    url = "s://u:p@h";
    std::cout << m_clogCapt.str();
    EXPECT_EQ(url.scheme(), "s");
    EXPECT_EQ(url.username(), "u");
    // EXPECT_EQ(url.host(), "h");
    EXPECT_EQ(url.port_num(), 0);
}

} // namespace upnplib

//
int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
