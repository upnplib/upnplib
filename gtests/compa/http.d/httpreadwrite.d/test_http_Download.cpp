// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-02-12

// Include source code for testing. So we have also direct access to static
// functions which need to be tested.
#include "pupnp/upnp/src/genlib/net/http/httpreadwrite.cpp"

#include "upnplib/upnptools.hpp"
#include "upnplib/gtest.hpp"

#include "gmock/gmock.h"
#include "umock/sysinfo_mock.hpp"

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetErrnoAndReturn;

using ::upnplib::errStrEx;

namespace compa {
bool old_code{true}; // Managed in upnplib/gtest_main.inc
bool github_actions = std::getenv("GITHUB_ACTIONS");

/*
clang-format off

     http_Download()
02)  |__ http_FixStrUrl()
03)  |__ get_hoststr()
04)  |__ http_MakeMessage()
     |__ http_RequestAndResponse()  // get doc msg
     |__ print_http_headers()
     |__ if content_type
     |      copy content type
     |__ extract doc from msg

02) Tested with test_httpreadwrite.cpp - TEST(HttpFixUrl, *)
03) Tested with test_httpreadwrite.cpp - TEST(GetHostaddr, *)
04) Tested with TEST(HttpMakeMessage, *)

clang-format on
*/

// Testsuite for http_MakeMessage()
// ================================
/* All available format types are tested:
clang-format off
Format types:
  'B':  arg = int status_code        -- appends content-length, content-type
                                        and HTML body for given code.
  'b':  arg1 = const char* buf;
        arg2 = size_t buf_length memory ptr
  'C':  (no args)                    -- appends a HTTP CONNECTION: close header
                                        depending on major, minor version.
  'c':  (no args)                    -- appends CRLF "\r\n"
  'D':  (no args)                    -- appends HTTP DATE: header
  'd':  arg = int number             -- appends decimal number
  'G':  arg = range information      -- add range header
  'h':  arg = off_t number           -- appends off_t number
  'K':  (no args)                    -- add chunky header
  'L':  arg = language information   -- add Content-Language header if Accept-
                                        Language header is not empty and if
                                        WEB_SERVER_CONTENT_LANGUAGE is not
                                        empty
  'N':  arg1 = off_t content_length  -- content-length header
  'Q':  arg1 = http_method_t;        -- start line of request
        arg2 = char* url;
        arg3 = size_t url_length
  'q':  arg1 = http_method_t         -- request start line and HOST header
        arg2 = (uri_type *)
  'R':  arg = int status_code        -- adds a response start line
  'S':  (no args)                    -- appends HTTP SERVER: header
  's':  arg = const char *           -- C_string
  'T':  arg = char * content_type;   -- add content-type header,
                                        format e.g: "text/html";
  't':  arg = time_t * gmt_time      -- appends time in RFC 1123 fmt
  'U':  (no args)                    -- appends HTTP USER-AGENT: header
  'X':  arg = const char *           -- useragent; "redsonic"
                                        HTTP X-User-Agent: useragent
clang-format on
*/

class HttpMakeMessageFTestSuite : public ::testing::Test {
  protected:
    membuffer m_request{};

    HttpMakeMessageFTestSuite() { membuffer_init(&m_request); }

    ~HttpMakeMessageFTestSuite() override { membuffer_destroy(&m_request); }
};

TEST_F(HttpMakeMessageFTestSuite, format_B_successful) {
    // format type 'B':  arg = int status_code  -- appends content-length,
    //                                             content-type and HTML body
    //                                             for given code.
    // status_code as given in .*/statcodes.cpp in ranges beginning with 100,
    // 200, 300, 400 and 500.

    // Test Unit with status_code 200 -> "OK"
    EXPECT_EQ(http_MakeMessage(&m_request, 1, 1, "B", 200), 0);
    EXPECT_STREQ(m_request.buf,
                 "CONTENT-LENGTH: 41\r\nAccept-Ranges: bytes\r\nCONTENT-TYPE: "
                 "text/html\r\n\r\n<html><body><h1>200 OK</h1></body></html>");
}

TEST_F(HttpMakeMessageFTestSuite, format_b_successful) {
    // format type 'b':  arg1 = const char* buf;  -- mem buffer
    //                   arg2 = size_t buf_length memory ptr

    constexpr char hoststr[]{"upnplib.net:443/tvdevicedesc.xml"};
    size_t hostlen = 15; // hoststr[14] is '/' but not '\0'!

    // Test Unit
    EXPECT_EQ(http_MakeMessage(&m_request, 1, 1, "b", hoststr, hostlen), 0);
    EXPECT_STREQ(m_request.buf, "upnplib.net:443");
}

TEST_F(HttpMakeMessageFTestSuite, format_C_successful) {
    // format type 'C':  (no args)  -- appends a HTTP CONNECTION: close header
    //                                 depending on major, minor version.

    // Test Unit
    EXPECT_EQ(http_MakeMessage(&m_request, 1, 1, "C"), 0);
    EXPECT_STREQ(m_request.buf, "CONNECTION: close\r\n");
}

TEST_F(HttpMakeMessageFTestSuite, format_c_successful) {
    // format type 'c':  (no args)  -- appends CRLF "\r\n"

    // Test Unit
    EXPECT_EQ(http_MakeMessage(&m_request, 1, 1, "c"), 0);
    EXPECT_STREQ(m_request.buf, "\r\n");
}

TEST_F(HttpMakeMessageFTestSuite, format_D_successful) {
    // format type 'D':  (no args)  -- appends HTTP DATE: header

    // Mock "current" time to have a constant value.
    umock::SysinfoMock mocked_sysinfoObj;
    umock::Sysinfo sysinfo_injectObj(&mocked_sysinfoObj);
    // Return Unix Epoch
    EXPECT_CALL(mocked_sysinfoObj, time(nullptr)).WillOnce(Return(1675628181));

    // Test Unit
    EXPECT_EQ(http_MakeMessage(&m_request, 1, 1, "D"), 0);
    EXPECT_STREQ(m_request.buf, "DATE: Sun, 05 Feb 2023 20:16:21 GMT\r\n");
}

TEST_F(HttpMakeMessageFTestSuite, format_d_successful) {
    // format type 'd':  arg = int number  -- appends decimal number

    // Test Unit
    EXPECT_EQ(http_MakeMessage(&m_request, 1, 1, "d", 123456), 0);
    EXPECT_STREQ(m_request.buf, "123456");
}

TEST_F(HttpMakeMessageFTestSuite, format_G_successful) {
    // format type 'G':  arg = range information  -- add range header

    // From */upnp.hpp to function 'UpnpOpenHttpGetEx()':
    // Gets specified number of bytes from a file specified in the URL.
    // The number of bytes is specified through a low count and a high count
    // which are passed as a range of bytes for the request.

    SendInstruction rangeBuf;
    constexpr int lowRange{2}; // Maybe an empty file with "\r\n"
    constexpr int highRange{32768};

    // Provide a rangeBuf (what ever it is ?-: )
    // Taken from */httpreadwrite.cpp function 'http_OpenHttpGetEx()'
    memset(&rangeBuf, 0, sizeof(rangeBuf));
    int rc = snprintf(rangeBuf.RangeHeader, sizeof(rangeBuf.RangeHeader),
                      "Range: bytes=%d-%d\r\n", lowRange, highRange);
    if (rc < 0 || (unsigned int)rc >= sizeof(rangeBuf.RangeHeader))
        GTEST_FAIL();

    // Test Unit
    EXPECT_EQ(http_MakeMessage(&m_request, 1, 1, "G", &rangeBuf), 0);
    EXPECT_STREQ(m_request.buf, "Range: bytes=2-32768\r\n");
}

TEST_F(HttpMakeMessageFTestSuite, format_h_successful) {
    // format type 'h':  arg = off_t number  -- appends off_t number

    // One more will compile to -Werror=overflow
    constexpr off_t bignum{1073741824 + 1073741823};

    // Test Unit
    EXPECT_EQ(http_MakeMessage(&m_request, 1, 1, "h", bignum), 0);
    EXPECT_STREQ(m_request.buf, "2147483647");
}

TEST_F(HttpMakeMessageFTestSuite, format_K_successful) {
    // format type 'K':  (no args)  -- add chunky header

    // Test Unit
    EXPECT_EQ(http_MakeMessage(&m_request, 1, 1, "K"), 0);
    EXPECT_STREQ(m_request.buf, "TRANSFER-ENCODING: chunked\r\n");
}

TEST_F(HttpMakeMessageFTestSuite, format_L_successful) {
    // format type 'L':  arg = language information
    //                   -- add Content-Language header only
    //                      if Accept-Language header is not empty and
    //                      if WEB_SERVER_CONTENT_LANGUAGE is not empty

    GTEST_SKIP() << "Still to be done";
    // Here we have a bug with the conditions noted above. They are checked with
    // logical AND but need to be ORed.

    SendInstruction RespInstr;
    memset(&RespInstr, 0, sizeof(RespInstr));
    // RFC 9110: 12.5.4. Accept-Language
    strcpy(RespInstr.AcceptLanguageHeader, // buffer size is 200
           "en, de-DE;q=0.8, de;q=0.7");

    // Test Unit
    EXPECT_EQ(http_MakeMessage(&m_request, 1, 1, "L", &RespInstr), 0);
    EXPECT_STREQ(m_request.buf, "en, de-DE;q=0.8, de;q=0.7\r\n");
}

TEST_F(HttpMakeMessageFTestSuite, format_N_successful) {
    // format type 'N':  arg1 = off_t content_length  -- content-length header

    // One more will compile to -Werror=overflow
    constexpr off_t bignum{1073741824 + 1073741823};

    // Test Unit
    EXPECT_EQ(http_MakeMessage(&m_request, 1, 1, "N", bignum), 0);
    EXPECT_STREQ(m_request.buf,
                 "CONTENT-LENGTH: 2147483647\r\nAccept-Ranges: bytes\r\n");
}

TEST_F(HttpMakeMessageFTestSuite, format_Q_successful) {
    // format type 'Q':  arg1 = http_method_t;  -- start line of request
    //                   arg2 = char* urlpath; (not complete url)
    //                   arg3 = size_t urlpath_length

    // Provide only path from e.g.
    // "https://user.name@www.upnplib.net:443/path/dest/?query=value#fragment";
    constexpr char urlpath[]{"/path/dest/?query=value#fragment"};

    // Test Unit
    EXPECT_EQ(http_MakeMessage(&m_request, 1, 1, "Q", HTTPMETHOD_GET, urlpath,
                               sizeof(urlpath)),
              0);
    EXPECT_STREQ(m_request.buf, "GET /path/dest/?query=value#fragment");
}

TEST_F(HttpMakeMessageFTestSuite, format_q_successful) {
    // format type 'q':  arg1 = http_method_t  -- request start line
    //                                            and HOST header
    //                   arg2 = (uri_type *)

    // Provide a url structure
    uri_type url;
    // I use an ip address. With a name a DNS server would be asked and that
    // fails with test conditions.
#ifdef UPNP_ENABLE_OPEN_SSL
    constexpr char url_str[]{"https://192.168.192.170:443/path/dest/"
                             "?query=value#fragment"};
#else
    constexpr char url_str[]{"http://192.168.192.171:80/path/dest/"
                             "?query=value#fragment"};
#endif
    ASSERT_EQ(http_FixStrUrl(url_str, strlen(url_str), &url), 0);

    // Test Unit
    int ret_http_MakeMessage{UPNP_E_INTERNAL_ERROR};
    ret_http_MakeMessage =
        http_MakeMessage(&m_request, 1, 1, "q", HTTPMETHOD_GET, &url);
    EXPECT_EQ(ret_http_MakeMessage, 0) << errStrEx(ret_http_MakeMessage, 0);
#ifdef UPNP_ENABLE_OPEN_SSL
    EXPECT_STREQ(m_request.buf, "GET /path/dest/?query=value HTTP/1.1\r\nHOST: "
                                "192.168.192.170:443\r\n");
#else
    EXPECT_STREQ(
        m_request.buf,
        "GET /path/dest/?query=value HTTP/1.1\r\nHOST: 192.168.192.171:80\r\n");
#endif
}

TEST_F(HttpMakeMessageFTestSuite, format_R_successful) {
    // format type 'R':  arg = int status_code  -- adds a response start line

    // Test Unit
    EXPECT_EQ(http_MakeMessage(&m_request, 1, 1, "R", HTTP_OK), 0);
    EXPECT_STREQ(m_request.buf, "HTTP/1.1 200 OK\r\n");
}

ACTION_P(StructCpyToArg0, buf) { memcpy(arg0, buf, sizeof(*arg0)); }

TEST_F(HttpMakeMessageFTestSuite, format_S_successful) {
    // format type 'S':  (no args)  -- appends HTTP SERVER: header

    std::cout << CRED "[ BUG      ] " CRES << __LINE__
              << ": On Microsoft Windows the HTTP response header 'Server' "
                 "should be correct formated.\n";

#ifdef _WIN32
    // Test Unit
    EXPECT_EQ(http_MakeMessage(&m_request, 1, 1, "S"), 0);

    std::string result;
    if (old_code) {
        result = "SERVER: UPnP/1.0, Portable SDK for UPnP devices/" +
                 std::string(UPNP_VERSION_STRING) + "on windows\r\n";
        EXPECT_EQ(std::string(m_request.buf), result);

    } else {

        if (github_actions)
            GTEST_SKIP() << "             known failing test on Github Actions";

        result = "SERVER: Microsoft Windows, UPnP/1.0, Portable SDK for UPnP "
                 "devices/" +
                 std::string(UPNP_VERSION_STRING) + "\r\n";
        EXPECT_EQ(std::string(m_request.buf), result);
    }

#else

    // Provide structure for 'uname()' with system info
    utsname sysinf{};
    strncpy(sysinf.sysname, "Linux", sizeof(sysinf.sysname) - 1);
    strncpy(sysinf.release, "5.10.0-20-amd64", sizeof(sysinf.release) - 1);

    // Mock uname system info
    umock::SysinfoMock mocked_sysinfoObj;
    umock::Sysinfo sysinfo_injectObj(&mocked_sysinfoObj);
    EXPECT_CALL(mocked_sysinfoObj, uname(_))
        .WillOnce(DoAll(StructCpyToArg0(&sysinf), Return(0)));

    // Test Unit
    EXPECT_EQ(http_MakeMessage(&m_request, 1, 1, "S"), 0);

    std::string result = "SERVER: Linux/5.10.0-20-amd64, UPnP/1.0, Portable "
                         "SDK for UPnP devices/" +
                         std::string(UPNP_VERSION_STRING) + "\r\n";
    EXPECT_EQ(std::string(m_request.buf), result);
#endif
}

TEST_F(HttpMakeMessageFTestSuite, format_T_successful) {
    // format type 'T':  arg = char* content_type;  -- add content-type header,
    //                                                 format e.g: "text/html";

    // Test Unit
    EXPECT_EQ(http_MakeMessage(&m_request, 1, 1, "T", "text/html"), 0);
    EXPECT_STREQ(m_request.buf, "CONTENT-TYPE: text/html\r\n");
}

TEST_F(HttpMakeMessageFTestSuite, format_U_successful) {
    GTEST_SKIP() << "Still to be done.";
    // This is the same as with format type 'S' except that the HTTP header
    // field starts with "USER-AGENT: " instead of "SERVER: ". Just test only
    // this difference.
}


// Tests for the HTTP version number
// ---------------------------------

TEST_F(HttpMakeMessageFTestSuite, format_C_invalid_version) {
    // format type 'C':  (no args)  -- appends a HTTP CONNECTION: close header
    //                                 depending on major, minor version.

    // Test Unit
    EXPECT_EQ(http_MakeMessage(&m_request, 0, 0, "C"), 0);
    EXPECT_EQ(m_request.buf, nullptr);
}

TEST_F(HttpMakeMessageFTestSuite, format_Q_invalid_version) {
    // format type 'Q':  arg1 = http_method_t;  -- start line of request
    //                   arg2 = char* urlpath; (not complete url)
    //                   arg3 = size_t urlpath_length

    // Provide only path from e.g.
    // "https://user.name@www.upnplib.net:443/path/dest/?query=value#fragment"};
    constexpr char urlpath[]{"/path/dest/?query=value#fragment"};

    // Test Unit
    EXPECT_EQ(http_MakeMessage(&m_request, 0, 0, "Q", HTTPMETHOD_GET, urlpath,
                               sizeof(urlpath)),
              0);
    EXPECT_STREQ(m_request.buf, "GET /path/dest/?query=value#fragment");
}

TEST(HttpMakeMessageTestSuite, improve_tests_for_version_no) {
    GTEST_SKIP() << "To be done: improve tests for the HTTP version number";
}


// Tests for error conditions on HttpMakeMessage()
// -----------------------------------------------
TEST(HttpMakeMessageTestSuite, improve_tests_for_error_conditions) {
    GTEST_SKIP() << "To be done: improve tests for error conditions on "
                    "HttpMakeMessage()";
}

TEST_F(HttpMakeMessageFTestSuite, format_S_with_failed_system_info) {
    // format type 'S':  (no args)  -- appends HTTP SERVER: header

    // TODO: Make this Test mainly with testing get_sdk_info(). Here only
    // provide a simple failing condition.

#ifndef _WIN32
    // Provide invalid structure for 'uname()'
    utsname sysinf;
    constexpr size_t sysinf_size{sizeof(sysinf)};
    memset(&sysinf, 0xAA, sysinf_size);
    // To be on the safe side we will have a defined end for strings.
    memset((unsigned char*)&sysinf + sysinf_size - 1, '\0', 1);

    // Mock 'uname()' system info to fail with invalid utsname structure. errno
    // returns 'EFAULT buf is not valid'.
    umock::SysinfoMock mocked_sysinfoObj;
    umock::Sysinfo sysinfo_injectObj(&mocked_sysinfoObj);
    EXPECT_CALL(mocked_sysinfoObj, uname(_))
        .WillOnce(
            DoAll(StructCpyToArg0(&sysinf), SetErrnoAndReturn(EFAULT, -1)));

    // Test Unit
    EXPECT_EQ(http_MakeMessage(&m_request, 1, 1, "S"), 0);

    if (old_code) {

        std::cout << CRED "[ BUG      ] " CRES << __LINE__
                  << ": Formating HTTP response header 'Server' should not "
                     "return garbage with failing system info.\n";
        EXPECT_EQ(strncmp(m_request.buf,
                          "SERVER: \xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA\xAA",
                          18),
                  0); // Wrong!

    } else {

        std::string result =
            "SERVER: Unknown, UPnP/1.0, Portable SDK for UPnP devices/" +
            std::string(UPNP_VERSION_STRING) + "\r\n";
        EXPECT_EQ(std::string(m_request.buf), result);
    }
#endif
}


} // namespace compa


int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "compa/gtest_main.inc"
}
