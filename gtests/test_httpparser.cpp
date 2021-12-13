// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-12-08

#include "gtest/gtest.h"

#include "genlib/net/http/httpparser.cpp"

namespace upnp {

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

class Chttpparser : public Ihttpparser {
  public:
    virtual ~Chttpparser() {}

    void httpmsg_init(http_message_t* msg) override {
        return ::httpmsg_init(msg); }
    void httpmsg_destroy(http_message_t* msg) override {
        return ::httpmsg_destroy(msg); }
    http_header_t* httpmsg_find_hdr_str(http_message_t* msg, const char* header_name) {
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
// clang-format on

// testsuite for httpparser
//-------------------------
TEST(HttpparserTestSuite, dummy_test) {}

} // namespace upnp

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
