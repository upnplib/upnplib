// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-03-09

#include "gtest/gtest.h"
#include "upnplib/url.hpp"

namespace upnplib {

bool github_actions = ::std::getenv("GITHUB_ACTIONS");

TEST(UrlClassTestSuite, parse_valid_url) {
    CUrl url;
    url.set("https://fred:password@www.wikipedia.org/"
            "what-me-worry?hello=there#wonder");

    EXPECT_EQ(url.getScheme(), "https");
    EXPECT_EQ(url.getUsername(), "fred");
    EXPECT_EQ(url.getPassword(), "password");
    EXPECT_EQ(url.getHost(), "www.wikipedia.org");
    EXPECT_EQ(url.getPort(), 443);
    EXPECT_EQ(url.getPath(), "/what-me-worry");
    EXPECT_EQ(url.getQuery(), "hello=there");
    // Has been removed because of no functionality
    // EXPECT_EQ(url.getQueryParameters().size(), 0);
    EXPECT_EQ(url.getFragment(), "wonder");
    EXPECT_EQ(url.getFullPath(), "/what-me-worry?hello=there#wonder");
    EXPECT_FALSE(url.isIpv6());
    EXPECT_TRUE(url.isSecure());
    EXPECT_EQ(url.toString(), "https://fred:password@www.wikipedia.org/"
                              "what-me-worry?hello=there#wonder");
    // explicit operator std::string()
    // EXPECT_EQ((std::string)url, "???");
}

TEST(UrlClassTestSuite, url_object_without_parameter) {
    CUrl url;

    EXPECT_EQ(url.getScheme(), "");
    EXPECT_EQ(url.getUsername(), "");
    EXPECT_EQ(url.getPassword(), "");
    EXPECT_EQ(url.getHost(), "");
    EXPECT_EQ(url.getPort(), 0);
    EXPECT_EQ(url.getPath(), "");
    EXPECT_EQ(url.getQuery(), "");
    EXPECT_EQ(url.getFragment(), "");
    EXPECT_FALSE(url.isSecure());
    EXPECT_FALSE(url.isIpv6());
}

TEST(UrlClassTestSuite, url_object_with_empty_parameter) {
    CUrl url{};

    EXPECT_EQ(url.getScheme(), "");
    EXPECT_EQ(url.getUsername(), "");
    EXPECT_EQ(url.getPassword(), "");
    EXPECT_EQ(url.getHost(), "");
    EXPECT_EQ(url.getPort(), 0);
    EXPECT_EQ(url.getPath(), "");
    EXPECT_EQ(url.getQuery(), "");
    EXPECT_EQ(url.getFragment(), "");
    EXPECT_FALSE(url.isSecure());
    EXPECT_FALSE(url.isIpv6());
}

TEST(UrlClassTestSuite, empty_url) {
    // This should be a valid parameter
    CUrl url;
    url.set("");

    EXPECT_EQ(url.getScheme(), "");
    EXPECT_EQ(url.getUsername(), "");
    EXPECT_EQ(url.getPassword(), "");
    EXPECT_EQ(url.getHost(), "");
    EXPECT_EQ(url.getPort(), 0);
    EXPECT_EQ(url.getPath(), "");
    EXPECT_EQ(url.getQuery(), "");
    EXPECT_EQ(url.getFragment(), "");
    EXPECT_FALSE(url.isSecure());
    EXPECT_FALSE(url.isIpv6());
}

TEST(UrlClassTestSuite, set_second_empty_url) {
    CUrl url;
    url.set("https://fred:password@www.wikipedia.org/"
            "what-me-worry?hello=there#wonder");
    // This should be a valid parameter
    url.set("");

    EXPECT_EQ(url.getScheme(), "");
    EXPECT_EQ(url.getUsername(), "");
    EXPECT_EQ(url.getPassword(), "");
    EXPECT_EQ(url.getHost(), "");
    EXPECT_EQ(url.getPort(), 0);
    EXPECT_EQ(url.getPath(), "");
    EXPECT_EQ(url.getQuery(), "");
    EXPECT_EQ(url.getFragment(), "");
    EXPECT_FALSE(url.isSecure());
    EXPECT_FALSE(url.isIpv6());
}

TEST(UrlClassTestSuite, url_with_only_colon) {
    if (github_actions)
        GTEST_SKIP() << "             known failing test on Github Actions";

    CUrl url;
    EXPECT_THROW(url.set(":"), std::invalid_argument);
}

TEST(UrlClassTestSuite, invalid_url) {
    if (github_actions)
        GTEST_SKIP() << "             known failing test on Github Actions";

    CUrl url;
    EXPECT_THROW(url.set("hello world"), std::invalid_argument);
}

TEST(UrlClassTestSuite, url_only_with_unsecure_scheme) {
    CUrl url;
    url.set("x---s:");

    EXPECT_EQ(url.getScheme(), "x---s");
    EXPECT_EQ(url.getUsername(), "");
    EXPECT_EQ(url.getPassword(), "");
    EXPECT_EQ(url.getHost(), "");
    EXPECT_EQ(url.getPort(), 0);
    EXPECT_EQ(url.getPath(), "");
    EXPECT_EQ(url.getQuery(), "");
    EXPECT_EQ(url.getFragment(), "");
    EXPECT_FALSE(url.isSecure());
    EXPECT_FALSE(url.isIpv6());
}

TEST(UrlClassTestSuite, url_only_with_secure_scheme) {
    CUrl url;
    url.set("https:");

    EXPECT_EQ(url.getScheme(), "https");
    EXPECT_EQ(url.getUsername(), "");
    EXPECT_EQ(url.getPassword(), "");
    EXPECT_EQ(url.getHost(), "");
    EXPECT_EQ(url.getPort(), 443);
    EXPECT_EQ(url.getPath(), "");
    EXPECT_EQ(url.getQuery(), "");
    EXPECT_EQ(url.getFragment(), "");
    EXPECT_TRUE(url.isSecure());
    EXPECT_FALSE(url.isIpv6());
}

} // namespace upnplib

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
