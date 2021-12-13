// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-12-13

#include "gtest/gtest.h"

#include "genlib/net/http/httpreadwrite.cpp"

namespace upnp {

//
// Interface for the httpreadwrite module
// --------------------------------------
// clang-format off
// Only these functions are called exclusively by the upnpapi module.
//
// `http_CancelHttpGet(void*)'
// `http_CloseHttpConnection(void*)'
// `http_Download(char const*, int, char**, unsigned long*, char*)'
// `http_EndHttpRequest(void*, int)'
// `http_GetHttpResponse(void*, s_UpnpString*, char**, int*, int*, int)'
// `http_HttpGetProgress(void*, unsigned long*, unsigned long*)'
// `http_MakeHttpRequest(Upnp_HttpMethod_e, char const*, void*, s_UpnpString*, char const*, int, int)'
// `http_OpenHttpConnection(char const*, void**, int)'
// `http_OpenHttpGetEx(char const*, void**, char**, int*, int*, int, int, int)'
// `http_ReadHttpResponse(void*, char*, unsigned long*, int)'

class Ihttpreadwrite {
  public:
    virtual ~Ihttpreadwrite() {}

    virtual struct tm* http_gmtime_r(
            const time_t* clock, struct tm* result) = 0;
    virtual int http_FixUrl(
            uri_type* url, uri_type* fixed_url) = 0;
    virtual int http_FixStrUrl(
            const char* urlstr, size_t urlstrlen, uri_type* fixed_url) = 0;
    virtual SOCKET http_Connect(
            uri_type* destination_url, uri_type* url) = 0;
    virtual int http_RecvMessage(
            SOCKINFO* info, http_parser_t* parser, http_method_t request_method,
            int* timeout_secs, int* http_error_code) = 0;
    virtual int http_SendMessage(
            SOCKINFO* info, int* TimeOut, const char* fmt, ...) = 0;
    virtual int http_RequestAndResponse(
            uri_type* destination, const char* request, size_t request_length,
            http_method_t req_method, int timeout_secs, http_parser_t* response) = 0;
    virtual int http_Download(
            const char* url_str, int timeout_secs, char** document,
            size_t* doc_length, char* content_type) = 0;
    virtual int MakeGenericMessage(
            http_method_t method, const char* url_str, membuffer* request, uri_type* url,
            int contentLength, const char* contentType, const UpnpString* headers) = 0;
    virtual int http_HttpGetProgress(
            void* Handle, size_t* length, size_t* total) = 0;
    virtual int http_CancelHttpGet(
            void* Handle) = 0;
    virtual int http_OpenHttpConnection(
            const char* url_str, void** Handle, int timeout) = 0;
    virtual int http_MakeHttpRequest(
            Upnp_HttpMethod method, const char* url_str, void* Handle, UpnpString* headers,
            const char* contentType, int contentLength, int timeout) = 0;
    virtual int http_WriteHttpRequest(
            void* Handle, char* buf, size_t* size, int timeout) = 0;
    virtual int http_EndHttpRequest(
            void* Handle, int timeout) = 0;
    virtual int http_GetHttpResponse(
            void* Handle, UpnpString* headers, char** contentType, int* contentLength,
            int* httpStatus, int timeout) = 0;
    virtual int http_ReadHttpResponse(
            void* Handle, char* buf, size_t* size, int timeout) = 0;
    virtual int http_CloseHttpConnection(
            void* Handle) = 0;
    virtual int http_SendStatusResponse(
            SOCKINFO* info, int http_status_code, int request_major_version,
            int request_minor_version) = 0;
    virtual int http_MakeMessage(
            membuffer* buf, int http_major_version, int http_minor_version,
            const char* fmt, ...) = 0;
    virtual void http_CalcResponseVersion(
            int request_major_vers, int request_minor_vers, int* response_major_vers,
            int* response_minor_vers) = 0;
    virtual int MakeGetMessageEx(
            const char* url_str, membuffer* request, uri_type* url,
            struct SendInstruction* pRangeSpecifier) = 0;
    virtual int http_OpenHttpGetEx(
            const char* url_str, void** Handle, char** contentType, int* contentLength,
            int* httpStatus, int lowRange, int highRange, int timeout) = 0;
    virtual void get_sdk_info(
            char* info, size_t infoSize) = 0;
};

class Chttpreadwrite : Ihttpreadwrite {
  public:
    virtual ~Chttpreadwrite() override {}

    struct tm* http_gmtime_r(const time_t* clock, struct tm* result) override {
        return ::http_gmtime_r(clock, result); }
    int http_FixUrl(uri_type* url, uri_type* fixed_url) override {
        return ::http_FixUrl(url, fixed_url); }
    int http_FixStrUrl(const char* urlstr, size_t urlstrlen, uri_type* fixed_url) override {
        return ::http_FixStrUrl(urlstr, urlstrlen, fixed_url); }
    SOCKET http_Connect(uri_type* destination_url, uri_type* url) override {
        return ::http_Connect(destination_url, url); }
    int http_RecvMessage(SOCKINFO* info, http_parser_t* parser, http_method_t request_method, int* timeout_secs, int* http_error_code) override {
        return ::http_RecvMessage(info, parser, request_method, timeout_secs, http_error_code); }
    int http_SendMessage(SOCKINFO* info, int* TimeOut, const char* fmt, ...) override {
    //    return ::http_SendMessage(info, TimeOut, fmt, ...); }
        return UPNP_E_OUTOF_MEMORY; }
    int http_RequestAndResponse(uri_type* destination, const char* request, size_t request_length, http_method_t req_method, int timeout_secs, http_parser_t* response) override {
        return ::http_RequestAndResponse(destination, request, request_length, req_method, timeout_secs, response); }
    int http_Download(const char* url_str, int timeout_secs, char** document, size_t* doc_length, char* content_type) override {
        return ::http_Download(url_str, timeout_secs, document, doc_length, content_type); }
    int MakeGenericMessage(http_method_t method, const char* url_str, membuffer* request, uri_type* url, int contentLength, const char* contentType, const UpnpString* headers) override {
        return ::MakeGenericMessage(method, url_str, request, url, contentLength, contentType, headers); }
    int http_HttpGetProgress(void* Handle, size_t* length, size_t* total) override {
        return ::http_HttpGetProgress(Handle, length, total); }
    int http_CancelHttpGet(void* Handle) override {
        return ::http_CancelHttpGet(Handle); }
    int http_OpenHttpConnection(const char* url_str, void** Handle, int timeout) override {
        return ::http_OpenHttpConnection(url_str, Handle, timeout); }
    int http_MakeHttpRequest(Upnp_HttpMethod method, const char* url_str, void* Handle, UpnpString* headers, const char* contentType, int contentLength, int timeout) override {
        return ::http_MakeHttpRequest(method, url_str, Handle, headers, contentType, contentLength, timeout); }
    int http_WriteHttpRequest(void* Handle, char* buf, size_t* size, int timeout) override {
        return ::http_WriteHttpRequest(Handle, buf, size, timeout); }
    int http_EndHttpRequest(void* Handle, int timeout) override {
        return ::http_EndHttpRequest(Handle, timeout); }
    int http_GetHttpResponse(void* Handle, UpnpString* headers, char** contentType, int* contentLength, int* httpStatus, int timeout) override {
        return ::http_GetHttpResponse(Handle, headers, contentType, contentLength, httpStatus, timeout); }
    int http_ReadHttpResponse(void* Handle, char* buf, size_t* size, int timeout) override {
        return ::http_ReadHttpResponse(Handle, buf, size, timeout); }
    int http_CloseHttpConnection(void* Handle) override {
        return ::http_CloseHttpConnection(Handle); }
    int http_SendStatusResponse(SOCKINFO* info, int http_status_code, int request_major_version, int request_minor_version) override {
        return ::http_SendStatusResponse(info, http_status_code, request_major_version, request_minor_version); }
    int http_MakeMessage(membuffer* buf, int http_major_version, int http_minor_version, const char* fmt, ...) override {
        // return ::http_MakeMessage(buf, http_major_version, http_minor_version, fmt, ...); }
        return UPNP_E_OUTOF_MEMORY; }
    void http_CalcResponseVersion(int request_major_vers, int request_minor_vers, int* response_major_vers, int* response_minor_vers) override {
        return ::http_CalcResponseVersion(request_major_vers, request_minor_vers, response_major_vers, response_minor_vers); }
    int MakeGetMessageEx(const char* url_str, membuffer* request, uri_type* url, struct SendInstruction* pRangeSpecifier) override {
        return ::MakeGetMessageEx(url_str, request, url, pRangeSpecifier); }
    int http_OpenHttpGetEx(const char* url_str, void** Handle, char** contentType, int* contentLength, int* httpStatus, int lowRange, int highRange, int timeout) override {
        return ::http_OpenHttpGetEx(url_str, Handle, contentType, contentLength, httpStatus, lowRange, highRange, timeout); }
    void get_sdk_info(char* info, size_t infoSize) override {
        return ::get_sdk_info(info, infoSize); }
};
// clang-format on

// testsuite for httpreadwrite
//----------------------------
TEST(HttpreadwriteTestSuite, open_close_http_connection) {
    GTEST_SKIP() << "Test must be created.";

    // Chttpreadwrite httprwObj;
    // EXPECT_EQ(httprwObj.http_OpenHttpConnection(url, handle, timeout),
    // UPNP_E_SUCCESS);
}

// testsuite for statcodes
//------------------------
TEST(StatcodesTestSuite, http_get_code_text) {
    // const char* code_text = ::http_get_code_text(HTTP_NOT_FOUND);
    // ::std::cout << "code_text: " << code_text << ::std::endl;
    EXPECT_STREQ(::http_get_code_text(HTTP_NOT_FOUND), "Not Found");
    EXPECT_EQ(::http_get_code_text(99), nullptr);
    EXPECT_EQ(::http_get_code_text(600), nullptr);
}

} // namespace upnp

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
