// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-03-22

#include "upnplib/url.hpp"

#include "gmock/gmock.h"

using ::testing::HasSubstr;
using ::testing::ThrowsMessage;

//
namespace upnplib {
bool github_actions = std::getenv("GITHUB_ACTIONS");

TEST(UrlClassTestSuite, empty_url_object) {
    Url url;

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

// Tests for copy_url_clean_to_input
// ---------------------------------
TEST(UrlClassTestSuite, empty_url_string) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "";
        },
        ThrowsMessage<std::invalid_argument>("Invalid URL: ''"));
}

TEST(UrlClassTestSuite, one_space_url) {
    EXPECT_THAT(
        []() {
            Url url;
            url = " ";
        },
        ThrowsMessage<std::invalid_argument>("Invalid URL: ''"));
}

TEST(UrlClassTestSuite, url_with_control_chars_and_spaces) {
    Url url;
    url = "\1 \2a:\x0D/\3/\x0Ag. h\x09.i\x7F\x4 ";

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

TEST(UrlClassTestSuite, url_with_null_char) {
    // This is a special case with null terminated strings in C++. It is by
    // design and will end up in a truncated url.
    EXPECT_THAT(
        []() {
            Url url;
            url = "\1 \2x\0https://www.example.com";
        },
        ThrowsMessage<std::invalid_argument>("Invalid URL: 'x'"));
}

TEST(UrlClassTestSuite, convert_url_chars_upper_to_lower) {
    Url url;
    url = "HTTps://Example.COM";

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
TEST(UrlClassTestSuite, invalid_digit_at_first_pos_in_scheme) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "2ttp://a.b.c";
        },
        ThrowsMessage<std::invalid_argument>("Invalid URL: '2ttp://a.b.c'"));
}

TEST(UrlClassTestSuite, valid_digits_not_at_first_pos_in_scheme) {
    Url url;
    url = "h9t6://a.b.c";

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
TEST(UrlClassTestSuite, scheme_with_invalid_character) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "ftp@x:";
        },
        ThrowsMessage<std::invalid_argument>("Invalid URL: 'ftp@x:'"));
}

TEST(UrlClassTestSuite, invalid_scheme_without_colon) {
    EXPECT_THAT(
        []() {
            Url url;
            url = "ftp";
        },
        ThrowsMessage<std::invalid_argument>("Invalid URL: 'ftp'"));
}

TEST(UrlClassTestSuite, file_scheme_without_slashes) {
    Url url;
    url = "file:";

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

TEST(UrlClassTestSuite, file_scheme_with_slashes) {
    Url url;
    url = "file://";

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

TEST(UrlClassTestSuite, parse_full_url) {
    Url url;
    url = "Https://John.Doe@www.Example.com:123/FORUM/questions/"
          "?tag=networking&order=newest#top";

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
