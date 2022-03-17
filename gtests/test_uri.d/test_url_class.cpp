// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-03-17

#include "gtest/gtest.h"
#include "upnplib/url.hpp"

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

TEST(UrlClassTestSuite, empty_url_string) {
    Url url;
    url = "";

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

TEST(UrlClassTestSuite, one_space_url) {
    Url url;
    url = " ";

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

TEST(UrlClassTestSuite, url_with_leading_control_chars) {
    Url url;
    url = "\1 \2x\0https://www.example.com";

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

TEST(UrlClassTestSuite, url_with_trailing_control_chars) {
    Url url;
    url = "https://www.example.com \3\4 ";

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

TEST(UrlClassTestSuite, url_with_leading_and_trailing_control_chars) {
    Url url;
    url = " \1\2https://www.example.com \3\4 ";

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

TEST(UrlClassTestSuite, url_with_tabs_and_newlines) {
    Url url;
    url = "https:\x0D//\x0Awww.example\x09.com";

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

TEST(UrlClassTestSuite, parse_full_url) {
    if (github_actions)
        GTEST_SKIP() << "             known failing test on Github Actions";

    Url url;
    url = "https://john.doe@www.example.com:123/forum/questions/"
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
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
