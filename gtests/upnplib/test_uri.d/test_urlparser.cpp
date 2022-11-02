// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-11-02

#include "upnplib/src/net/uri/urlparser.cpp"

#include "gmock/gmock.h"

using ::testing::ElementsAre;
using ::testing::ThrowsMessage;

//
namespace upnplib {

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

//
// Tests for IPv6 parser
// ---------------------
typedef UrlClassFTestSuite UrlClassIpv6ParserFTestSuite;

TEST_F(UrlClassIpv6ParserFTestSuite, empty_address) {
    EXPECT_THROW(ipv6_parser(""), std::invalid_argument);
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassIpv6ParserFTestSuite, single_colon_address) {
    EXPECT_THROW(ipv6_parser(":"), std::invalid_argument);
    std::cout << m_clogCapt.str();
}

TEST(UrlClassIpv6ParserTestSuite, double_colon_address) {
    EXPECT_THAT(ipv6_parser("::"), ElementsAre(0, 0, 0, 0, 0, 0, 0, 0));
}

TEST_F(UrlClassIpv6ParserFTestSuite, single_colon_with_digit_address) {
    EXPECT_THROW(ipv6_parser(":1"), std::invalid_argument);
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassIpv6ParserFTestSuite, three_colon_address) {
    EXPECT_THROW(ipv6_parser(":::"), std::invalid_argument);
    std::cout << m_clogCapt.str();
}

TEST(UrlClassIpv6ParserTestSuite, valid_address_0) {
    EXPECT_THAT(ipv6_parser("::0"), ElementsAre(0, 0, 0, 0, 0, 0, 0, 0));
}

TEST(UrlClassIpv6ParserTestSuite, valid_address_1) {
    EXPECT_THAT(ipv6_parser("::1"), ElementsAre(0, 0, 0, 0, 0, 0, 0, 1));
}

TEST(UrlClassIpv6ParserTestSuite, valid_address_9) {
    EXPECT_THAT(ipv6_parser("::9"), ElementsAre(0, 0, 0, 0, 0, 0, 0, 9));
}

TEST(UrlClassIpv6ParserTestSuite, valid_address_A) {
    EXPECT_THAT(ipv6_parser("::A"), ElementsAre(0, 0, 0, 0, 0, 0, 0, 0x000A));
}

TEST(UrlClassIpv6ParserTestSuite, valid_address_F) {
    EXPECT_THAT(ipv6_parser("::F"), ElementsAre(0, 0, 0, 0, 0, 0, 0, 0x000F));
}

TEST(UrlClassIpv6ParserTestSuite, valid_address_a) {
    EXPECT_THAT(ipv6_parser("::a00"), ElementsAre(0, 0, 0, 0, 0, 0, 0, 0x0A00));
}

TEST(UrlClassIpv6ParserTestSuite, valid_address_f) {
    EXPECT_THAT(ipv6_parser("::f"), ElementsAre(0, 0, 0, 0, 0, 0, 0, 0x000F));
}

TEST(UrlClassIpv6ParserTestSuite, valid_address_B) {
    EXPECT_THAT(ipv6_parser("B::"), ElementsAre(0x000B, 0, 0, 0, 0, 0, 0, 0));
}

TEST(UrlClassIpv6ParserTestSuite, valid_address_c) {
    EXPECT_THAT(ipv6_parser("0:0:c000::"),
                ElementsAre(0, 0, 0xC000, 0, 0, 0, 0, 0));
}

TEST(UrlClassIpv6ParserTestSuite, valid_address_double_colon) {
    EXPECT_THAT(ipv6_parser("2001:0DB8::F000"),
                ElementsAre(0x2001, 0x0DB8, 0, 0, 0, 0, 0, 0xF000));
}

TEST(UrlClassIpv6ParserTestSuite, valid_address) {
    EXPECT_THAT(ipv6_parser("2001:Db8:1234:5678:9abC:DeF0:1234:5678"),
                ElementsAre(0x2001, 0x0DB8, 0x1234, 0x5678, 0x9ABC, 0xDEF0,
                            0x1234, 0x5678));
}

TEST(UrlClassIpv6ParserTestSuite, ipv4_mapped_address_with_dots_2) {
    EXPECT_THAT(ipv6_parser("2001:DB8::ffff:192.0.2.128"),
                ElementsAre(0x2001, 0x0DB8, 0, 0, 0, 0xFFFF, 0xC000, 0x0280));
}

TEST(UrlClassIpv6ParserTestSuite, ipv4_mapped_address_with_dots_1) {
    EXPECT_THAT(ipv6_parser("::ffff:192.0.2.128"),
                ElementsAre(0, 0, 0, 0, 0, 0xFFFF, 0xC000, 0x0280));
}

TEST_F(UrlClassIpv6ParserFTestSuite, ipv4_mapped_address_with_wrong_dots) {
    EXPECT_THROW(ipv6_parser("::ffff.192.0.2.128"), std::invalid_argument);
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassIpv6ParserFTestSuite, address_invalid_at_begin) {
    EXPECT_THROW(ipv6_parser("X001:Db8:1234:5678:9abC:DeF0:1234:5678"),
                 std::invalid_argument);
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassIpv6ParserFTestSuite, address_invalid_at_end) {
    EXPECT_THROW(ipv6_parser("2001:Db8:1234:5678:9abC:DeF0:1234:567g"),
                 std::invalid_argument);
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassIpv6ParserFTestSuite, address_invalid_with_smiley) {
    EXPECT_THROW(ipv6_parser("2001:Db8:1234:56😀8:9abC:DeF0:1234:5678"),
                 std::invalid_argument);
    std::cout << m_clogCapt.str();
}

TEST_F(UrlClassIpv6ParserFTestSuite,
       address_invalid_with_double_double_colons) {
    EXPECT_THROW(ipv6_parser("2001:Db8::9abC::5678"), std::invalid_argument);
    std::cout << m_clogCapt.str();
}

// Tests for host parser
// ---------------------
typedef UrlClassFTestSuite UrlClassHostParserFTestSuite;

TEST_F(UrlClassHostParserFTestSuite, no_closing_bracket) {
    EXPECT_THAT(
        []() {
            // Missing closing ']'
            host_parser("[2001:DB8:1234:5678:9ABC:DEF0:1234:5678");
        },
        ThrowsMessage<std::invalid_argument>(
            "Missing closing ']': '[2001:DB8:1234:5678:9ABC:DEF0:1234:5678'"));
    std::cout << m_clogCapt.str();
}

TEST(UrlClassHostParserTestSuite, valid_ip6_address) {
    EXPECT_EQ(host_parser("[2001:DB8:1234:5678:9ABC:DEF0:1234:5678]",
                          /*isNotSpecial*/ true),
              "2001:DB8:1234:5678:9ABC:DEF0:1234:5678");
}

TEST(UrlClassHostParserTestSuite, not_special_host_name) {
    EXPECT_EQ(host_parser("dummy.not_special.host", /*isNotSpecial*/ true), "");
}

TEST(UrlClassHostParserTestSuite, not_special_empty_host_name) {
    EXPECT_EQ(host_parser("", /*isNotSpecial*/ true), "");
}

TEST(UrlClassHostParserTestSuite, special_host_name) {
    EXPECT_EQ(host_parser("dummy.final.parsed.host"),
              "dummy.final.parsed.host");
}

TEST(UrlClassHostParserTestSuite, special_empty_host_name) {
    EXPECT_THAT(
        []() { host_parser(""); },
        ThrowsMessage<std::invalid_argument>("Host string must not be empty."));
}

} // namespace upnplib

//
int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
