// Copyright (C) 2021+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-03-08

#include "pupnp/upnp/src/genlib/net/http/httpparser.cpp"
#ifndef UPNPLIB_WITH_NATIVE_PUPNP
#define NS ::upnplib
#include "upnplib/src/net/http/httpparser.cpp"
#else
#define NS
#endif

#include "gmock/gmock.h"

namespace compa {

bool old_code{false}; // Managed in compa/gtest_main.inc
bool github_actions = ::std::getenv("GITHUB_ACTIONS");

//
// Interface for the httpparser module
// -----------------------------------
// clang-format off
// Only these functions are called exclusively by the httpreadwrite module.
//
// `httpmsg_destroy(http_message_t*)'
// `httpmsg_find_hdr(http_message_t*, int, memptr*)'
// `method_to_str(http_method_t)'
// `parser_append(http_parser_t*, char const*, unsigned long)'
// `parser_get_entity_read_method(http_parser_t*)'
// `parser_parse_entity(http_parser_t*)'
// `parser_parse_headers(http_parser_t*)'
// `parser_parse_responseline(http_parser_t*)'
// `parser_request_init(http_parser_t*)'
// `parser_response_init(http_parser_t*, http_method_t)'

class Ihttpparser {
  public:
    virtual ~Ihttpparser() {}

    virtual void httpmsg_init( // gtest available
            http_message_t* msg) = 0;
    virtual void httpmsg_destroy( // gtest available
            http_message_t* msg) = 0;
    virtual http_header_t* httpmsg_find_hdr_str(
            http_message_t* msg, const char* header_name) = 0;
    virtual http_header_t* httpmsg_find_hdr(
            http_message_t* msg, int header_name_id, memptr* value) = 0;
    // virtual parse_status_t matchstr(
    //         char* str, size_t slen, const char* fmt, ...) = 0;
    virtual parse_status_t parser_parse_responseline(
            http_parser_t* parser) = 0;
    virtual parse_status_t parser_parse_headers(
            http_parser_t* parser) = 0;
    virtual parse_status_t parser_get_entity_read_method(
            http_parser_t* parser) = 0;
    virtual parse_status_t parser_parse_entity(
            http_parser_t* parser) = 0;
    virtual void parser_request_init( // gtest available
            http_parser_t* parser) = 0;
    virtual void parser_response_init( // gtest available
            http_parser_t* parser, http_method_t request_method) = 0;
    virtual parse_status_t parser_parse(
            http_parser_t* parser) = 0;
    virtual parse_status_t parser_append(
            http_parser_t* parser, const char* buf, size_t buf_length) = 0;
    virtual int raw_to_int(
            memptr* raw_value, int base) = 0;
    virtual int raw_find_str(
            memptr* raw_value, const char* str) = 0;
    virtual const char* method_to_str(
            http_method_t method) = 0;
    // virtual void print_http_headers(http_message_t* hmsg) = 0;
};

class Chttpparser_old : public Ihttpparser {
  public:
    virtual ~Chttpparser_old() {}

    void httpmsg_init(http_message_t* msg) override {
        return ::httpmsg_init(msg); }
    void httpmsg_destroy(http_message_t* msg) override {
        return ::httpmsg_destroy(msg); }
    http_header_t* httpmsg_find_hdr_str(http_message_t* msg, const char* header_name) override {
        return ::httpmsg_find_hdr_str(msg, header_name); }
    http_header_t* httpmsg_find_hdr(http_message_t* msg, int header_name_id, memptr* value) override {
        return ::httpmsg_find_hdr(msg, header_name_id, value); }
    // parse_status_t matchstr(char* str, size_t slen, const char* fmt, ...) override {
    //    return ::matchstr(str, slen, fmt, ...); }
    //    return PARSE_NO_MATCH; }
    parse_status_t parser_parse_responseline(http_parser_t* parser) override {
        return ::parser_parse_responseline(parser); }
    parse_status_t parser_parse_headers(http_parser_t* parser) override {
        return ::parser_parse_headers(parser); }
    parse_status_t parser_get_entity_read_method(http_parser_t* parser) override {
        return ::parser_get_entity_read_method(parser); }
    parse_status_t parser_parse_entity(http_parser_t* parser) override {
        return ::parser_parse_entity(parser); }
    void parser_request_init(http_parser_t* parser) override {
        return ::parser_request_init(parser); }
    void parser_response_init(http_parser_t* parser, http_method_t request_method) override {
        return ::parser_response_init(parser, request_method); }
    parse_status_t parser_parse(http_parser_t* parser) override {
        return ::parser_parse(parser); }
    parse_status_t parser_append(http_parser_t* parser, const char* buf, size_t buf_length) override {
        return ::parser_append(parser, buf, buf_length); }
    int raw_to_int(memptr* raw_value, int base) override {
        return ::raw_to_int(raw_value, base); }
    int raw_find_str(memptr* raw_value, const char* str) override {
        return ::raw_find_str(raw_value, str); }
    const char* method_to_str(http_method_t method) override {
        return ::method_to_str(method); }
    // void print_http_headers(http_message_t* hmsg) override {
    //     return ::print_http_headers(hmsg); }
};

class Chttpparser : public Chttpparser_old {
  public:
    virtual ~Chttpparser() {}

    void httpmsg_init(http_message_t* msg) override {
        return NS::httpmsg_init(msg); }
};
// clang-format on

//
// testsuite for httpparser
// ========================
TEST(HttpparserTestSuite, map_str_to_int) {
    GTEST_SKIP() << "  # Move function map_str_to_int() to httpparser.cpp, "
                    "test it here and remove module strintmap.";
}

TEST(HttpparserTestSuite, map_int_to_str) {
    GTEST_SKIP() << "  # Move function map_int_to_str() to httpparser.cpp, "
                    "test it here and remove module strintmap.";
}

TEST(HttpparserTestSuite, httpmsg_init_and_httpmsg_destroy) {
    http_message_t msg;
    memset(&msg, 0xff, sizeof(msg));
    EXPECT_EQ(msg.headers.head.next, (LISTNODE*)0xffffffffffffffff);

    Chttpparser httparsObj;

    if (old_code) {
        std::cout
            << "  BUG! httpmsg_init: msg->urlbuf set to nullptr otherwise "
               "segfault with httpmsg_destroy.\n";
        Chttpparser_old httpars_oldObj;
        httpars_oldObj.httpmsg_init(&msg);
        EXPECT_NE(msg.urlbuf, nullptr);
    } else {
        // Fixed function
        httparsObj.httpmsg_init(&msg);
        EXPECT_EQ(msg.urlbuf, nullptr);
    }
    EXPECT_EQ(msg.initialized, 1);
    EXPECT_EQ(msg.entity.buf, nullptr);
    EXPECT_EQ(msg.entity.length, (size_t)0);
    EXPECT_EQ(msg.headers.head.prev, nullptr);
    EXPECT_NE(msg.headers.head.next, (LISTNODE*)0xffffffffffffffff);
    EXPECT_EQ(msg.msg.buf, nullptr);
    EXPECT_EQ(msg.msg.length, (size_t)0);
    EXPECT_EQ(msg.msg.capacity, (size_t)0);
    EXPECT_EQ(msg.status_msg.buf, nullptr);
    EXPECT_EQ(msg.status_msg.length, (size_t)0);
    EXPECT_EQ(msg.status_msg.capacity, (size_t)0);

    if (old_code) {
        std::cout << "       A buggy httpmsg_init() must not raise a segfault "
                     "with httpmsg_destroy().\n";
    } else {
        httparsObj.httpmsg_destroy(&msg);
        EXPECT_EQ(msg.initialized, 0);
    }
}

TEST(HttpparserTestSuite, httpmsg_init_a_nullptr) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    if (old_code) {
        ::std::cout << "[ BUG!     ] A nullptr to a message structure must not "
                       "segfault.\n";
    } else {
        // Test Unit
        Chttpparser_old httparsObj;
        ASSERT_EXIT((httparsObj.httpmsg_init(nullptr), exit(0)),
                    ::testing::ExitedWithCode(0), ".*")
            << "   BUG! A nullptr to a message structure must not segfault.";
    }
}

TEST(HttpparserTestSuite, httpmsg_destroy_a_nullptr) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    if (old_code) {
        ::std::cout << "[ BUG!     ] A nullptr to a message structure must not "
                       "segfault.\n";
    } else {
        // Test Unit
        Chttpparser_old httparsObj;
        ASSERT_EXIT((httparsObj.httpmsg_destroy(nullptr), exit(0)),
                    ::testing::ExitedWithCode(0), ".*")
            << "   BUG! A nullptr to a message structure must not segfault.";
    }
}

TEST(HttpparserTestSuite, httpmsg_find_hdr_str) {
    Chttpparser httparsObj;

    http_message_t msg;
    memset(&msg, 0xaa, sizeof(msg));
    ::httpmsg_init(&msg);

    constexpr char header_name[16]{"TestHeader"};

    EXPECT_EQ(httparsObj.httpmsg_find_hdr_str(&msg, header_name), nullptr);
}

TEST(HttpparserTestSuite, parser_init) {
    http_parser_t parser;
    ::memset(&parser, 0xaa, sizeof(http_parser_t));

    // Test Unit
    ::parser_init(&parser);

    EXPECT_EQ(parser.http_error_code, HTTP_BAD_REQUEST);
    EXPECT_EQ(parser.ent_position, ENTREAD_DETERMINE_READ_METHOD);
    EXPECT_EQ(parser.valid_ssdp_notify_hack, 0);
    EXPECT_EQ(parser.msg.initialized, 1);
    EXPECT_EQ(parser.scanner.cursor, (size_t)0);
    EXPECT_EQ(parser.scanner.msg, &parser.msg.msg);
    EXPECT_EQ(parser.scanner.entire_msg_loaded, 0);
}

TEST(HttpparserTestSuite, parser_init_a_nullptr) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    if (old_code) {
        ::std::cout << "[ BUG!     ] A nullptr to a parser structure must not "
                       "segfault.\n";
    } else {
#ifdef DEBUG
        // Test Unit
        ASSERT_EXIT((::parser_init(nullptr), exit(0)),
                    ::testing::ExitedWithCode(0), ".*")
            << "   BUG! A nullptr to a parser structure must not segfault.";
#endif
    }
}

TEST(HttpparserTestSuite, parser_request_init) {
    http_parser_t parser;
    ::memset(&parser, 0xaa, sizeof(http_parser_t));

    // Test Unit
    Chttpparser_old httpars_oObj;
    httpars_oObj.parser_request_init(&parser);

    EXPECT_EQ(parser.msg.is_request, 1);
    EXPECT_EQ(parser.position, POS_REQUEST_LINE);
    // from parser_init()
    EXPECT_EQ(parser.http_error_code, HTTP_BAD_REQUEST);
    EXPECT_EQ(parser.msg.initialized, 1);
}

TEST(HttpparserTestSuite, parser_request_init_a_nullptr) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    if (old_code) {
        ::std::cout << "[ BUG!     ] A nullptr to a parser structure must not "
                       "segfault.\n";
    } else {
#ifdef DEBUG
        // Test Unit
        Chttpparser_old httpars_oObj;
        ASSERT_EXIT((httpars_oObj.parser_request_init(nullptr), exit(0)),
                    ::testing::ExitedWithCode(0), ".*")
            << "   BUG! A nullptr to a parser structure must not segfault.";
#endif
    }
}

TEST(HttpparserTestSuite, parser_response_init) {
    http_parser_t parser;
    ::memset(&parser, 0xaa, sizeof(http_parser_t));

    // Test Unit
    Chttpparser_old httpars_oObj;
    httpars_oObj.parser_response_init(&parser, HTTPMETHOD_NOTIFY);

    EXPECT_EQ(parser.msg.is_request, 0);
    EXPECT_EQ(parser.msg.request_method, HTTPMETHOD_NOTIFY);
    EXPECT_EQ(parser.msg.amount_discarded, (size_t)0);
    EXPECT_EQ(parser.position, POS_RESPONSE_LINE);
    // from parser_init()
    EXPECT_EQ(parser.http_error_code, HTTP_BAD_REQUEST);
    EXPECT_EQ(parser.msg.initialized, 1);
}

TEST(HttpparserTestSuite, parser_response_init_a_nullptr) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    if (old_code) {
        ::std::cout << "[ BUG!     ] A nullptr to a parser structure must not "
                       "segfault.\n";
    } else {
#ifdef DEBUG
        // Test Unit
        Chttpparser_old httpars_oObj;
        ASSERT_EXIT(
            (httpars_oObj.parser_response_init(nullptr, HTTPMETHOD_NOTIFY),
             exit(0)),
            ::testing::ExitedWithCode(0), ".*")
            << "   BUG! A nullptr to a parser structure must not segfault.";
#endif
    }
}

} // namespace compa

//
int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "compa/gtest_main.inc"
    return gtest_return_code; // managed in compa/gtest_main.inc
}
