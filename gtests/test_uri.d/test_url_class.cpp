// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-03-23

#include "upnplib/url.hpp"
#include "gmock/gmock.h"
#include <sstream>

using ::testing::HasSubstr;
using ::testing::ThrowsMessage;

//
namespace upnplib {
// bool github_actions = std::getenv("GITHUB_ACTIONS");

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

TEST_F(UrlClassFTestSuite, empty_url_object) {
    Url url;
    std::cout << m_clogCapt.str();

    EXPECT_EQ((std::string)url, "");
    EXPECT_EQ(url.scheme(), "");
    EXPECT_EQ(url.authority(), "");
    EXPECT_EQ(url.userinfo(), "");
    EXPECT_EQ(url.host(), "");
    EXPECT_EQ(url.port(), "");
    EXPECT_EQ(url.port_num(), 0);
    EXPECT_EQ(url.path(), "");
    EXPECT_EQ(url.query(), "");
    EXPECT_EQ(url.fragment(), "");
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
    EXPECT_NE(m_clogCapt.str().find("Warning: Removed 1 ASCII control "
                                    "character or spaces. Using \"\" now.\n"),
              std::string::npos);
    EXPECT_NE(m_clogCapt.str().find("Error: no valid scheme found.\n"),
              std::string::npos);
}

TEST_F(UrlClassFTestSuite, url_with_control_chars_and_spaces) {
    Url url;
    url = "\1 \2a:\x0D/\3/\x0Ag. h\x09.i\x7F\x4 ";

    std::cout << m_clogCapt.str();
    EXPECT_NE(
        m_clogCapt.str().find("Warning: Removed 11 ASCII control character or "
                              "spaces. Using \"a://g.h.i\" now.\n"),
        std::string::npos);

    EXPECT_EQ((std::string)url, "");
    EXPECT_EQ(url.scheme(), "a");
    EXPECT_EQ(url.authority(), "");
    EXPECT_EQ(url.userinfo(), "");
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
            url = "\1 \2x\0https://www.example.com";
        },
        ThrowsMessage<std::invalid_argument>("Invalid URL: 'x'"));

    std::cout << m_clogCapt.str();
    EXPECT_NE(m_clogCapt.str().find("Warning: Removed 3 ASCII control "
                                    "character or spaces. Using \"x\" now.\n"),
              std::string::npos);
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
    EXPECT_EQ(url.userinfo(), "");
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
    url = "h9t6://a.b.c";
    std::cout << m_clogCapt.str();

    EXPECT_EQ((std::string)url, "");
    EXPECT_EQ(url.scheme(), "h9t6");
    EXPECT_EQ(url.authority(), "");
    EXPECT_EQ(url.userinfo(), "");
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
    EXPECT_NE(m_clogCapt.str().find("Warning: 'file' scheme misses \"//\".\n"),
              std::string::npos);

    EXPECT_EQ((std::string)url, "");
    EXPECT_EQ(url.scheme(), "file");
    EXPECT_EQ(url.authority(), "");
    EXPECT_EQ(url.userinfo(), "");
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
    EXPECT_EQ(url.userinfo(), "");
    EXPECT_EQ(url.host(), "");
    EXPECT_EQ(url.port(), "");
    EXPECT_EQ(url.port_num(), 0);
    EXPECT_EQ(url.path(), "");
    EXPECT_EQ(url.query(), "");
    EXPECT_EQ(url.fragment(), "");
}

TEST_F(UrlClassFTestSuite, parse_full_url) {
    Url url;
    url = "Https://John.Doe@www.Example.com:123/FORUM/questions/"
          "?tag=networking&order=newest#top";
    std::cout << m_clogCapt.str();

    EXPECT_EQ(url.scheme(), "https");
#if false
    EXPECT_EQ(url.authority, "john.doe@www.example.com:123");
    EXPECT_EQ(url.auth.userinfo, "john.doe");
    EXPECT_EQ(url.auth().host(), "www.example.com");
    EXPECT_EQ(url.auth().port(), "123");
    EXPECT_EQ(url.auth().port_num(), 123);
    EXPECT_EQ(url.path(), "/forum/questions/");
    EXPECT_EQ(url.query(), "tag=networking&order=newest");
    EXPECT_EQ(url.fragment(), "top");
#endif
}

} // namespace upnplib

int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
