// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-01-13

#include "gtest/gtest.h"

#include "pupnp/upnp/src/genlib/net/http/httpparser.cpp"
#include "core/src/genlib/net/http/httpparser.cpp"

namespace upnplib {
bool old_code{false};

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

    virtual void httpmsg_init(
            http_message_t* msg) = 0;
    virtual void httpmsg_destroy(
            http_message_t* msg) = 0;
    virtual http_header_t* httpmsg_find_hdr_str(
            http_message_t* msg, const char* header_name) = 0;
    virtual http_header_t* httpmsg_find_hdr(
            http_message_t* msg, int header_name_id, memptr* value) = 0;
    virtual parse_status_t matchstr(
            char* str, size_t slen, const char* fmt, ...) = 0;
    virtual parse_status_t parser_parse_responseline(
            http_parser_t* parser) = 0;
    virtual parse_status_t parser_parse_headers(
            http_parser_t* parser) = 0;
    virtual parse_status_t parser_get_entity_read_method(
            http_parser_t* parser) = 0;
    virtual parse_status_t parser_parse_entity(
            http_parser_t* parser) = 0;
    virtual void parser_request_init(
            http_parser_t* parser) = 0;
    virtual void parser_response_init(
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
    virtual int parser_get_unknown_headers(
            http_message_t* req, UpnpListHead* list) = 0;
    virtual void free_http_headers_list(
            UpnpListHead* list) = 0;
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
    parse_status_t matchstr(char* str, size_t slen, const char* fmt, ...) override {
        // return ::matchstr(str, slen, fmt, ...); }
        return PARSE_NO_MATCH; }
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
    int parser_get_unknown_headers(http_message_t* req, UpnpListHead* list) override {
        return ::parser_get_unknown_headers(req, list); }
    void free_http_headers_list(UpnpListHead* list) override {
        return ::free_http_headers_list(list); }
    // void print_http_headers(http_message_t* hmsg) override {
    //     return ::print_http_headers(hmsg); }
};

class Chttpparser : public Chttpparser_old {
  public:
    virtual ~Chttpparser() {}

    void httpmsg_init(http_message_t* msg) override {
        return upnplib::httpmsg_init(msg); }
};

// clang-format on

//
// testsuite for httpparser
//-------------------------
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
        Chttpparser_old httparsObj;
        httparsObj.httpmsg_init(&msg);
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

TEST(HttpparserTestSuite, httpmsg_find_hdr_str) {
    Chttpparser httparsObj;

    http_message_t msg;
    memset(&msg, 0xff, sizeof(msg));
    ::httpmsg_init(&msg);

    const char header_name[16]{"TestHeader"};

    EXPECT_EQ(httparsObj.httpmsg_find_hdr_str(&msg, header_name), nullptr);
}

} // namespace upnplib

//
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    // ::testing::InitGoogleMock(&argc, argv);

    // Parse for upnplib arguments prefixed with '--upnplib'. InitGoogleTest()
    // has removed its options prefixed with '--gtest' from the arguments and
    // corrected argc accordingly.
    if (argc > 2) {
        std::cerr
            << "Too many arguments supplied. Valid only:\n--upnplib_old_code"
            << std::endl;
        return EXIT_FAILURE;
    }
    if (argc == 2) {
        if (strncmp(argv[1], "--upnplib_old_code", 18) == 0) {
            upnplib::old_code = true;
        } else {
            std::cerr << "Unknown argument. Valid only:\n--upnplib_old_code"
                      << std::endl;
            return EXIT_FAILURE;
        }
    }

    int rc = RUN_ALL_TESTS();

    // At least some information what we have tested.
    if (upnplib::old_code)
        std::cout << "             Tested UPnPlib old code.\n";
    else
        std::cout << "             Tested UPnPlib new code.\n";

    return rc;
}
