// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-08-09

// Include source code for testing. So we have also direct access to static
// functions which need to be tested.
#include "pupnp/upnp/src/genlib/net/http/httpreadwrite.cpp"
#include "upnplib/src/net/http/httpreadwrite.cpp"

#include "upnplib/upnptools.hpp"
#include "upnplib/uri.hpp"
#include "upnplib/gtest.hpp"

#include "umock/netdb_mock.hpp"
#include "umock/sys_socket_mock.hpp"
#include "umock/unistd_mock.hpp"

using ::testing::_;
using ::testing::DoAll;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::SetErrnoAndReturn;
using ::testing::StrEq;

using ::upnplib::Chttpreadwrite_old;
using ::upnplib::CUri;
using ::upnplib::Curi;
using ::upnplib::errStr;
using ::upnplib::errStrEx;

namespace compa {
bool old_code{false}; // Managed in compa/gtest_main.inc
bool github_actions = ::std::getenv("GITHUB_ACTIONS");


// Mocking
// =======
class Mock_netv4info : public umock::NetdbMock {
    // This is a derived class from mocking netdb to provide a structure for
    // addrinfo that can be given to the mocked program.
  private:
    // Provide structures to mock system call for network address
    struct sockaddr_in m_sa {};
    struct addrinfo m_res {};

  public:
    // Save and restore the old pointer to the production function
    Mock_netv4info() {
        m_sa.sin_family = AF_INET;
        m_res.ai_family = AF_INET;
    }

    addrinfo* get(const char* a_ipaddr, uint16_t a_port) {
        inet_pton(m_sa.sin_family, a_ipaddr, &m_sa.sin_addr);
        m_sa.sin_port = htons(a_port);

        m_res.ai_addrlen = sizeof(struct sockaddr);
        m_res.ai_addr = (sockaddr*)&m_sa;

        return &m_res;
    }
};

class PupnpHttpRwMock : public umock::PupnpHttpRwInterface {
  public:
    virtual ~PupnpHttpRwMock() override = default;
    MOCK_METHOD(int, private_connect,
                (SOCKET sockfd, const struct sockaddr* serv_addr,
                 socklen_t addrlen),
                (override));
};


// testsuites for Ip4 httpreadwrite
// ================================
#if false
TEST(OpenHttpConnectionTestSuite, open_http_connection_to_localhost) {
    // This test is more an integration test and needs the nc (or netcat, or
    // ncat) program. It examines the real situation without mocking anything.
    // It tests again the local loopback device 127.0.0.1, alias 'localhost'.
    // This is the reason why it isn't enabled permanently (#if true). I use it
    // to get the real situation so I can mock the scenario. To use it on Linux
    // you have to run something like
    // ~$ sudo nc -4 -l 127.0.0.1 80 # or
    // ~$ sudo nc -6 -l localhost 80
    // on a second console that will listen to the connect of
    // http_OpenHttpConnection(). Have attention to select the right IPv4 or
    // IPv6 option (-4 or -6) because there is no option to select it with
    // http_OpenHttpConnection() and it is a bit obscure what version it uses by
    // default. On Microsoft Windows you have to download and install ncat.exe
    // (prefered from the nmap site) but at this time it was removed by MS
    // Windows 10 antivirus detection after unpacking. It is reported on the web
    // that it is false positive. I think it is possible to disable its
    // detection but I haven't taken that effort.
    //
    // If ncat isnt't running, http_OpenHttpConnection() fails with
    // UPNP_E_SOCKET_CONNECT on Unix and with
    // UPNP_E_SOCKET_ERROR on MS Windows.
    // TODO: Check why failing is different on WIN32

    char serverurl[]{"http://localhost"};
    // char serverurl[]{"http://127.0.0.1"};
    url_str_len = strlen(url_str);

    http_connection_handle_t* phandle;
    memset(&phandle, 0xaa, sizeof(phandle));

    // Test Unit
    Chttpreadwrite_old httprw_oObj;
    int returned =
        httprw_oObj.http_OpenHttpConnection(serverurl, (void**)&phandle, 0);

    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);

    // Close connection
    returned = httprw_oObj.http_CloseHttpConnection(phandle);
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);
}
#endif

class Curi_type_testurl {
    // This is a fixed test uri structure. It is set exactly to the output of
    // parse_uri() incl. possible bugs. This is mainly made to understand the
    // structure and to have a valid uri_type url structure for testing.

  public:
    // To have access to these properties by pointing to the object (like
    // pointing to a C structure) they must be the first declarations. So we can
    // cast a pointer to the instantiation to uri_type* and use it as C
    // structure, for example:
    //
    // Curi_type_testurl testurlObj;
    // uri_type* url = (uri_type*)&testurlObj;
    // if (url->type == ABSOLUTE) {}; // C like usage
    // const char* test_url = testurlObj.get_testurl();

    enum uriType type;
    token scheme;
    enum pathType path_type;
    token pathquery;
    token fragment;
    hostport_type hostport;

  private:
    // The parts are only addressed by pointing to its position in the
    // m_url_str. Its lengths are only given by the size stored in the token.
    // They are not terminated by '\0'.

    // m_url_str with 55 characters is terminated with '\0'
    const char m_url_str[55 + 1]{
        "http://www.upnplib.net:80/dest/path/?key=value#fragment"};
    //   |      |                 |                     |      |
    //   0     +7                +25                   +47    +54 zero based

  public:
    Curi_type_testurl() {
        // Fill the uri_type structure.
        // TODO: Fix conflict with name ABSOLUTE
        // Due to name conflicts on different operating systems we need these
        // conditionals.

#ifdef __APPLE__
        // Seems here isn't our enum uriType used.
        type = (uriType)ABSOLUTE;
#else
#ifdef _WIN32
        // Unsuccessful attempt to fix the conflict.
        type = absolute;
#else
        // The only one that works as expected.
        type = ABSOLUTE;
#endif
#endif
        path_type = ABS_PATH;

        scheme.buff = m_url_str; // points to start of the scheme
        scheme.size = 4;         // limits to "http"

        hostport.text.buff = m_url_str + 7; // points to start of hostport
        hostport.text.size = 18;            // limits to "www.upnplib.net:80"

        // Fill ip address structure of the hostport property
        sockaddr_in* sai4 = (sockaddr_in*)&hostport.IPaddress;
        sai4->sin_family = AF_INET;
        sai4->sin_port = htons(80);
        sai4->sin_addr.s_addr = ::inet_addr("192.168.10.10");

        pathquery.buff = m_url_str + 25; // points to start of pathquery
        pathquery.size = 21;             // limits to "/dest/path/?key=value"

        fragment.buff = m_url_str + 47; // points to start of fragment
        fragment.size = 8;              // limits to "fragment"
    }

    // getter
    const char* get_testurl() { return m_url_str; }
};

TEST(ParseUriIp4TestSuite, verify_testurl) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    // I have made a fixed uri structure 'Curi_type_testurl' for testing. This
    // 'verify_testurl' is to check how gtest works on the structure and to
    // verfiy that the fixed test structure still match the output of
    // 'parse_uri()'.

    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.get("192.168.10.10", 80);

    // Mock for network address system calls. parse_uri() ask DNS server.
    umock::Netdb netdb_injectObj(&netv4inf);
    EXPECT_CALL(netv4inf, getaddrinfo(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(res), Return(0)));
    EXPECT_CALL(netv4inf, freeaddrinfo(_)).Times(1);

    // Get the test url
    Curi_type_testurl testurlObj;
    uri_type* test_url = (uri_type*)&testurlObj;
    const char* url_str = testurlObj.get_testurl();
    EXPECT_STREQ(url_str,
                 "http://www.upnplib.net:80/dest/path/?key=value#fragment");

    // Provide output structure but set to garbage to emulate uninitialization.
    uri_type out;
    memset(&out, 0xaa, sizeof(out));

    // Test Unit
    Curi uriObj;
    EXPECT_EQ(uriObj.parse_uri(url_str, strlen(url_str), &out), HTTP_SUCCESS);

    EXPECT_EQ(out.type, ABSOLUTE);

    if (old_code) {
        std::cout << CRED "[ BUG      ] " CRES << __LINE__
                  << ": Type value on uri structure should be ABSOLUTE but not "
                     "1 on MS Windows.\n";
#ifdef _WIN32
        EXPECT_EQ(1, out.type);
#else
        EXPECT_EQ(test_url->type, out.type);
#endif

    } else {

        std::cout << CRED "[ BUG      ] " CRES << __LINE__
                  << ": Type value on uri structure should be ABSOLUTE but not "
                     "1 on MS Windows.\n";
        EXPECT_EQ(test_url->type, out.type);
    }

    EXPECT_EQ(out.path_type, ABS_PATH);
    EXPECT_EQ(test_url->path_type, out.path_type);

    EXPECT_STREQ(out.scheme.buff,
                 "http://www.upnplib.net:80/dest/path/?key=value#fragment");
    EXPECT_EQ(out.scheme.size, (size_t)4);
    EXPECT_STREQ(test_url->scheme.buff, out.scheme.buff);
    EXPECT_EQ(test_url->scheme.size, out.scheme.size);

    EXPECT_STREQ(out.hostport.text.buff,
                 "www.upnplib.net:80/dest/path/?key=value#fragment");
    EXPECT_EQ(out.hostport.text.size, (size_t)18);
    EXPECT_STREQ(test_url->hostport.text.buff, out.hostport.text.buff);
    EXPECT_EQ(test_url->hostport.text.size, out.hostport.text.size);

    const sockaddr_in* out_sai4 = (sockaddr_in*)&out.hostport.IPaddress;
    const sockaddr_in* test_url_sai4 =
        (sockaddr_in*)&test_url->hostport.IPaddress;
    EXPECT_EQ(out_sai4->sin_family, AF_INET);
    EXPECT_EQ(out_sai4->sin_family, test_url_sai4->sin_family);
    EXPECT_EQ(out_sai4->sin_port, htons(80));
    EXPECT_EQ(out_sai4->sin_port, test_url_sai4->sin_port);
    EXPECT_STREQ(::inet_ntoa(out_sai4->sin_addr), "192.168.10.10");
    EXPECT_EQ(out_sai4->sin_addr.s_addr, test_url_sai4->sin_addr.s_addr);

    EXPECT_STREQ(out.pathquery.buff, "/dest/path/?key=value#fragment");
    EXPECT_EQ(out.pathquery.size, (size_t)21);
    EXPECT_STREQ(test_url->pathquery.buff, out.pathquery.buff);
    EXPECT_EQ(test_url->pathquery.size, out.pathquery.size);

    EXPECT_STREQ(out.fragment.buff, "fragment");
    EXPECT_EQ(out.fragment.size, (size_t)8);
    EXPECT_STREQ(test_url->fragment.buff, out.fragment.buff);
    EXPECT_EQ(test_url->fragment.size, out.fragment.size);
}

class OpenHttpConnectionIp4FTestSuite : public ::testing::Test {
  protected:
    // Provide mocked objects
    Mock_netv4info m_mock_netdbObj;
    PupnpHttpRwMock m_mock_pupnpHttpRwObj;
    umock::Sys_socketMock m_mock_socketObj;
    Chttpreadwrite_old m_httprw_oObj;

    // Dummy socket, if we do not need a real one due to mocking
    const ::SOCKET m_socketfd{514};

    OpenHttpConnectionIp4FTestSuite() {
        // Set default return values for getaddrinfo in case we get an
        // unexpected call but it should not occur.
        ON_CALL(m_mock_netdbObj, getaddrinfo(_, _, _, _))
            .WillByDefault(
                DoAll(SetArgPointee<3>(nullptr), Return(EAI_NONAME)));
    }
};


TEST_F(OpenHttpConnectionIp4FTestSuite, open_close_connection_successful) {
    // Expectations:
    // - get address info for url (DNS name resolution) succeeds
    // - free address info is called
    // - get network socket succeeds
    // - freeing network socket succeeds
    // - connect to network server succeeds

    // A connection handle will be allocated on memory by the function and the
    // pointer to it is returned here. As documented we must free it. We can use
    // a generic pointer because the function needs it:
    // void* phandle;
    // But that is bad for type-save C++. So we use the correct type with type
    // cast on the argument (void**)&phandle (see Test Unit below).
    // We initialize it to an invalid pointer.
    http_connection_handle_t* phandle;
    memset(&phandle, 0xaa, sizeof(phandle));

    addrinfo* res = m_mock_netdbObj.get("192.168.10.10", 80);
    const ::std::string servername{"upnplib.net"};

    // Mock to get network address info, means DNS name resolution.
    umock::Netdb netdb_injectObj(&m_mock_netdbObj);
    EXPECT_CALL(m_mock_netdbObj,
                getaddrinfo(StrEq(servername), nullptr, NotNull(), NotNull()))
        .WillOnce(DoAll(SetArgPointee<3>(res), Return(0)));
    // Check if it is freed.
    EXPECT_CALL(m_mock_netdbObj, freeaddrinfo(_)).Times(1);

    // Expect socket allocation
    umock::Sys_socketMock sys_socketObj;
    umock::Sys_socket sys_socket_injectObj(&sys_socketObj);
    EXPECT_CALL(sys_socketObj, socket(AF_INET, SOCK_STREAM, 0))
        .WillOnce(Return(m_socketfd));
    EXPECT_CALL(sys_socketObj, shutdown(_, _)).Times(1);
    umock::UnistdMock unistdObj;
    umock::Unistd unistd_injectObj(&unistdObj);
    EXPECT_CALL(unistdObj, CLOSE_SOCKET_P(_)).Times(1);

    // Mock for connection to a network server
    umock::PupnpHttpRw pupnp_httprw_injectObj(&m_mock_pupnpHttpRwObj);
    EXPECT_CALL(m_mock_pupnpHttpRwObj,
                private_connect(_, NotNull(), sizeof(sockaddr_in)))
        .WillOnce(Return(0));

    // Test Unit
    int returned = m_httprw_oObj.http_OpenHttpConnection(
        ("http://" + servername).c_str(), (void**)&phandle, 0);

    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);

    // Check phandle content
    EXPECT_EQ(phandle->sock_info.socket, m_socketfd);

    sockaddr_in* p_sa_in = (sockaddr_in*)&phandle->sock_info.foreign_sockaddr;
    // The sockaddr_storage is only filled in incoming requests. As http client
    // we are connecting to a server.
    EXPECT_EQ(p_sa_in->sin_port, 0);
    EXPECT_EQ(p_sa_in->sin_addr.s_addr, (unsigned int)0);

    // EXPECT_EQ(phandle->contentLength, 0); // Seems to be uninitialized
    // EXPECT_EQ(phandle->cancel, 0);        // Seems to be uninitialized

    // Close connection
    // Will call socket_h->shutdown and unistd_h->close
    returned = m_httprw_oObj.http_CloseHttpConnection(phandle);
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);
}

TEST_F(OpenHttpConnectionIp4FTestSuite, nullptr_to_url) {
    // Expectations:
    // - nothing done, return with error

    // Nothing should be happen
    umock::Netdb netdb_injectObj(&m_mock_netdbObj);
    EXPECT_CALL(m_mock_netdbObj, getaddrinfo(_, _, _, _)).Times(0);
    EXPECT_CALL(m_mock_netdbObj, freeaddrinfo(_)).Times(0);
    umock::Sys_socketMock sys_socketObj;
    umock::Sys_socket sys_socket_injectObj(&sys_socketObj);
    EXPECT_CALL(sys_socketObj, socket(_, _, _)).Times(0);
    EXPECT_CALL(sys_socketObj, shutdown(_, _)).Times(0);
    umock::UnistdMock unistdObj;
    umock::Unistd unistd_injectObj(&unistdObj);
    EXPECT_CALL(unistdObj, CLOSE_SOCKET_P(_)).Times(0);
    umock::PupnpHttpRw pupnp_httprw_injectObj(&m_mock_pupnpHttpRwObj);
    EXPECT_CALL(m_mock_pupnpHttpRwObj, private_connect(_, _, _)).Times(0);

    http_connection_handle_t* phandle;
    memset(&phandle, 0xaa, sizeof(phandle));
    http_connection_handle_t* phandle_bak{phandle};

    // Test Unit
    int returned =
        m_httprw_oObj.http_OpenHttpConnection(nullptr, (void**)&phandle, 0);

    EXPECT_EQ(returned, UPNP_E_INVALID_PARAM)
        << errStrEx(returned, UPNP_E_INVALID_PARAM);

    // phandle wasn't touched so it must not freed. Freeing the "uninitialized"
    // phandle would http_CloseHttpConnection() segfault.
    EXPECT_EQ(phandle, phandle_bak);
}

TEST_F(OpenHttpConnectionIp4FTestSuite, empty_url) {
    // Expectations:
    // - nothing done, return with error

    // Nothing should be happen
    umock::Netdb netdb_injectObj(&m_mock_netdbObj);
    EXPECT_CALL(m_mock_netdbObj, getaddrinfo(_, _, _, _)).Times(0);
    EXPECT_CALL(m_mock_netdbObj, freeaddrinfo(_)).Times(0);
    umock::Sys_socketMock sys_socketObj;
    umock::Sys_socket sys_socket_injectObj(&sys_socketObj);
    EXPECT_CALL(sys_socketObj, socket(_, _, _)).Times(0);
    EXPECT_CALL(sys_socketObj, shutdown(_, _)).Times(0);
    umock::UnistdMock unistdObj;
    umock::Unistd unistd_injectObj(&unistdObj);
    EXPECT_CALL(unistdObj, CLOSE_SOCKET_P(_)).Times(0);
    umock::PupnpHttpRw pupnp_httprw_injectObj(&m_mock_pupnpHttpRwObj);
    EXPECT_CALL(m_mock_pupnpHttpRwObj, private_connect(_, _, _)).Times(0);

    // Connection handle must be freed.
    http_connection_handle_t* phandle;
    memset(&phandle, 0xaa, sizeof(phandle));

    // Test Unit
    int returned =
        m_httprw_oObj.http_OpenHttpConnection("", (void**)&phandle, 0);

    EXPECT_EQ(returned, UPNP_E_INVALID_URL)
        << errStrEx(returned, UPNP_E_INVALID_URL);

    // A nullptr handle does not need to be freed.
    EXPECT_EQ(phandle, nullptr);
}

TEST_F(OpenHttpConnectionIp4FTestSuite, get_address_info_fails) {
    // Expectations:
    // - get address info for url (DNS name resolution) fails
    // - free address info not called
    // - get network socket not called
    // - freeing network socket not called
    // - connect to network server not called

    // For details look at test "open_connection_successful".
    addrinfo* res{};
    const ::std::string servername{"upnplib.net"};

    // Mock to get network address info, means DNS name resolution. This will
    // fail with error EAI_AGAIN, means "The name server returned a temporary
    // failure indication. Try again later."
    umock::Netdb netdb_injectObj(&m_mock_netdbObj);
    EXPECT_CALL(m_mock_netdbObj,
                getaddrinfo(StrEq(servername), nullptr, NotNull(), NotNull()))
        .WillOnce(DoAll(SetArgPointee<3>(res), Return(EAI_AGAIN)));
    // Should not be freed after a failed query.
    EXPECT_CALL(m_mock_netdbObj, freeaddrinfo(_)).Times(0);

    // Don't expect socket allocation.
    umock::Sys_socketMock sys_socketObj;
    umock::Sys_socket sys_socket_injectObj(&sys_socketObj);
    EXPECT_CALL(sys_socketObj, socket(_, _, _)).Times(0);
    EXPECT_CALL(sys_socketObj, shutdown(_, _)).Times(0);
    umock::UnistdMock unistdObj;
    umock::Unistd unistd_injectObj(&unistdObj);
    EXPECT_CALL(unistdObj, CLOSE_SOCKET_P(_)).Times(0);

    // Don't expect a connection to a network server
    umock::PupnpHttpRw pupnp_httprw_injectObj(&m_mock_pupnpHttpRwObj);
    EXPECT_CALL(m_mock_pupnpHttpRwObj, private_connect(_, _, _)).Times(0);

    // Connection handle must be freed.
    http_connection_handle_t* phandle;
    memset(&phandle, 0xaa, sizeof(phandle));

    // Test Unit
    int returned = m_httprw_oObj.http_OpenHttpConnection(
        ("http://" + servername).c_str(), (void**)&phandle, 0);

    EXPECT_EQ(returned, UPNP_E_INVALID_URL)
        << errStrEx(returned, UPNP_E_INVALID_URL);

    // A nullptr handle does not need to be freed.
    EXPECT_EQ(phandle, nullptr);
}

TEST_F(OpenHttpConnectionIp4FTestSuite, get_socket_fails) {
    // This test randomly fails with "Bus error***Exception" on Github Actions
    // running on MacOS config type "Release" and openssl enabled. Seems it is a
    // very specific environment that causes this error. I will try to isolate
    // the error step by step when doing other work.
#ifdef __APPLE__
    GTEST_SKIP() << "Test randomly only fails with 'Bus error***Exception' on "
                    "MacOS Release with openssl. That's hard to find.";
#endif

    // Expectations:
    // - get address info for url (DNS name resolution) succeeds
    // - free address info is called
    // - get network socket fails
    // - freeing network socket fails
    // - connect to network server not called

    // For details look at test "open_connection_successful".

    addrinfo* res = m_mock_netdbObj.get("192.168.10.10", 80);
    const ::std::string servername{"upnplib.net"};

    // Mock to get network address info, means DNS name resolution.
    umock::Netdb netdb_injectObj(&m_mock_netdbObj);
    EXPECT_CALL(m_mock_netdbObj,
                getaddrinfo(StrEq(servername), nullptr, NotNull(), NotNull()))
        .WillOnce(DoAll(SetArgPointee<3>(res), Return(0)));
    // Check if it is freed.
    EXPECT_CALL(m_mock_netdbObj, freeaddrinfo(_)).Times(1);

    // Socket allocation fails.
    umock::Sys_socketMock sys_socketObj;
    umock::Sys_socket sys_socket_injectObj(&sys_socketObj);
    EXPECT_CALL(sys_socketObj, socket(_, _, _))
        .WillOnce(SetErrnoAndReturn(EINVAL, -1));

    // Shutdown socket will fail with "The file descriptor sockfd does not refer
    // to a socket."
    EXPECT_CALL(sys_socketObj, shutdown(_, _))
#ifndef UPNP_ENABLE_OPEN_SSL
        .WillOnce(SetErrnoAndReturn(ENOTSOCK, -1));
#else
        .Times(0);
#endif

    // Close socket will fail with "fd isn't a valid open file descriptor."
    umock::UnistdMock unistdObj;
    umock::Unistd unistd_injectObj(&unistdObj);
    EXPECT_CALL(unistdObj, CLOSE_SOCKET_P(_))
#ifndef UPNP_ENABLE_OPEN_SSL
        .WillOnce(SetErrnoAndReturn(EBADF, -1));
#else
        .Times(0);
#endif

    // Don't expect a connection to a network server
    umock::PupnpHttpRw pupnp_httprw_injectObj(&m_mock_pupnpHttpRwObj);
    EXPECT_CALL(m_mock_pupnpHttpRwObj, private_connect(_, _, _)).Times(0);

    // Connection handle must be freed.
    http_connection_handle_t* invalid_ptr;
    memset(&invalid_ptr, 0xAA, sizeof(invalid_ptr));
    http_connection_handle_t* phandle{invalid_ptr};

    // Test Unit
    int returned = m_httprw_oObj.http_OpenHttpConnection(
        ("http://" + servername).c_str(), (void**)&phandle, 0);

    EXPECT_EQ(returned, UPNP_E_SOCKET_ERROR)
        << errStrEx(returned, UPNP_E_SOCKET_ERROR);

    if (old_code) {
        std::cout << CRED "[ BUG      ] " CRES << __LINE__
                  << ": Failing to get a socket must return INVALID_SOCKET.\n";
        EXPECT_NE((*phandle).sock_info.socket, INVALID_SOCKET); // Wrong!

    } else {

        EXPECT_EQ((*phandle).sock_info.socket, INVALID_SOCKET);
    }

    // Close connection
    // Will call socket_h->shutdown and unistd_h->close
    // Must not segfault with OpenSSL enabled, needs rework.
#ifndef UPNP_ENABLE_OPEN_SSL
    returned = m_httprw_oObj.http_CloseHttpConnection(phandle);
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);
#endif
}

TEST_F(OpenHttpConnectionIp4FTestSuite, connect_to_server_fails) {
    // Expectations:
    // - get address info for url (DNS name resolution) succeeds
    // - free address info is called
    // - get network socket succeeds
    // - freeing network socket succeeds
    // - connect to network server fails

    // For details look at test "open_connection_successful".

    addrinfo* res = m_mock_netdbObj.get("192.168.10.10", 80);
    const ::std::string servername{"upnplib.net"};

    // Mock to get network address info, means DNS name resolution.
    umock::Netdb netdb_injectObj(&m_mock_netdbObj);
    EXPECT_CALL(m_mock_netdbObj,
                getaddrinfo(StrEq(servername), nullptr, NotNull(), NotNull()))
        .WillOnce(DoAll(SetArgPointee<3>(res), Return(0)));
    // Check if it is freed.
    EXPECT_CALL(m_mock_netdbObj, freeaddrinfo(_)).Times(1);

    // Expect socket allocation
    umock::Sys_socketMock sys_socketObj;
    umock::Sys_socket sys_socket_injectObj(&sys_socketObj);
    EXPECT_CALL(sys_socketObj, socket(AF_INET, SOCK_STREAM, 0))
        .WillOnce(Return(m_socketfd));
    EXPECT_CALL(sys_socketObj, shutdown(_, _)).Times(1);
    umock::UnistdMock unistdObj;
    umock::Unistd unistd_injectObj(&unistdObj);
    EXPECT_CALL(unistdObj, CLOSE_SOCKET_P(_)).Times(1);

    // Connection to a network server will fail
    umock::PupnpHttpRw pupnp_httprw_injectObj(&m_mock_pupnpHttpRwObj);
    EXPECT_CALL(m_mock_pupnpHttpRwObj,
                private_connect(_, NotNull(), sizeof(sockaddr_in)))
        .WillOnce(Return(-1));

    // Connection handle must be freed.
    http_connection_handle_t* phandle;
    memset(&phandle, 0xaa, sizeof(phandle));

    // Test Unit
    int returned = m_httprw_oObj.http_OpenHttpConnection(
        ("http://" + servername).c_str(), (void**)&phandle, 0);

    EXPECT_EQ(returned, UPNP_E_SOCKET_CONNECT)
        << errStrEx(returned, UPNP_E_SOCKET_CONNECT);

    // Close connection
    // This does not call socket_h->shutdown or unistd_h->close. Seems it is
    // already done before because both are called Times(1).
    returned = m_httprw_oObj.http_CloseHttpConnection(phandle);
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);
}

TEST_F(OpenHttpConnectionIp4FTestSuite, open_connection_with_ip_address) {
    // Expectations:
    // - get address info for url (DNS name resolution) is not called
    // - free address info is not called
    // - get network socket succeeds
    // - freeing network socket succeeds
    // - connect to network server succeeds

    // For details look at test "open_connection_successful".

    constexpr char serverip[]{"http://192.168.168.168"};

    // Mock to get network address info, means DNS name resolution.
    umock::Netdb netdb_injectObj(&m_mock_netdbObj);
    EXPECT_CALL(m_mock_netdbObj, getaddrinfo(_, _, _, _)).Times(0);
    EXPECT_CALL(m_mock_netdbObj, freeaddrinfo(_)).Times(0);

    // Expect socket allocation
    umock::Sys_socketMock sys_socketObj;
    umock::Sys_socket sys_socket_injectObj(&sys_socketObj);
    EXPECT_CALL(sys_socketObj, socket(AF_INET, SOCK_STREAM, 0))
        .WillOnce(Return(m_socketfd));
    EXPECT_CALL(sys_socketObj, shutdown(_, _)).Times(1);
    umock::UnistdMock unistdObj;
    umock::Unistd unistd_injectObj(&unistdObj);
    EXPECT_CALL(unistdObj, CLOSE_SOCKET_P(_)).Times(1);

    // Mock for connection to a network server
    umock::PupnpHttpRw pupnp_httprw_injectObj(&m_mock_pupnpHttpRwObj);
    EXPECT_CALL(m_mock_pupnpHttpRwObj,
                private_connect(_, NotNull(), sizeof(sockaddr_in)))
        .WillOnce(Return(0));

    // Connection handle must be freed.
    http_connection_handle_t* phandle;
    memset(&phandle, 0xaa, sizeof(phandle));

    // Test Unit
    int returned =
        m_httprw_oObj.http_OpenHttpConnection(serverip, (void**)&phandle, 0);

    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);

    // Check phandle content
    EXPECT_EQ(phandle->sock_info.socket, m_socketfd);

    sockaddr_in* p_sa_in = (sockaddr_in*)&phandle->sock_info.foreign_sockaddr;
    // The sockaddr_storage is only filled in incoming requests. As http client
    // we are connecting to a server.
    EXPECT_EQ(p_sa_in->sin_port, 0);
    EXPECT_EQ(p_sa_in->sin_addr.s_addr, (unsigned int)0);

    // EXPECT_EQ(phandle->contentLength, 0);
    // EXPECT_EQ(phandle->cancel, 0);

    // Close connection
    // Will call socket_h->shutdown and unistd_h->close
    returned = m_httprw_oObj.http_CloseHttpConnection(phandle);
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);
}

typedef OpenHttpConnectionIp4FTestSuite CloseHttpConnectionIp4FTestSuite;

TEST_F(CloseHttpConnectionIp4FTestSuite, close_nullptr_handle) {
    // TODO: Improve tests which use http_CloseHttpConnection() to match the
    // current behavior. Rework funtion in compa to always be callable even with
    // previous error conditions. --Ingo

    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    http_connection_handle_t* phandle{nullptr};

    // Test Unit
    int returned = m_httprw_oObj.http_CloseHttpConnection(phandle);
    if (old_code) {
        EXPECT_EQ(returned, UPNP_E_INVALID_PARAM)
            << errStrEx(returned, UPNP_E_INVALID_PARAM);
    } else {
        EXPECT_EQ(returned, UPNP_E_SUCCESS)
            << "  OPT:" << errStrEx(returned, UPNP_E_SUCCESS);
    }
}

typedef OpenHttpConnectionIp4FTestSuite HttpConnectIp4FTestSuite;

TEST_F(HttpConnectIp4FTestSuite, successful_connect) {
    // Expect socket allocation
    umock::Sys_socketMock sys_socketObj;
    umock::Sys_socket sys_socket_injectObj(&sys_socketObj);
    EXPECT_CALL(sys_socketObj, socket(AF_INET, SOCK_STREAM, 0))
        .WillOnce(Return(m_socketfd));
    EXPECT_CALL(sys_socketObj, shutdown(_, _)).Times(0);
    umock::UnistdMock unistdObj;
    umock::Unistd unistd_injectObj(&unistdObj);
    EXPECT_CALL(unistdObj, CLOSE_SOCKET_P(_)).Times(0);

    // Mock for connection to a network server
    umock::PupnpHttpRw pupnp_httprw_injectObj(&m_mock_pupnpHttpRwObj);
    EXPECT_CALL(m_mock_pupnpHttpRwObj,
                private_connect(m_socketfd, NotNull(), sizeof(sockaddr_in)))
        .WillOnce(Return(0));

    // provide url structures for the http connection
    Curi_type_testurl testurlObj;
    uri_type* dest_url = (uri_type*)&testurlObj;
    uri_type fixed_url;
    memset(&fixed_url, 0xaa, sizeof(uri_type));

    // Test Unit
    SOCKET sockfd; // Returned value should be a valid socket.
    ASSERT_GT(sockfd = http_Connect(dest_url, &fixed_url), (SOCKET)0)
        << "  # Should be a valid socked file descriptor but not "
        << errStr((int)sockfd);

    EXPECT_STREQ(fixed_url.fragment.buff, "fragment");
    EXPECT_EQ(fixed_url.fragment.size, (size_t)8);
}

TEST_F(HttpConnectIp4FTestSuite, socket_allocation_fails) {
    // Expect no socket allocation
    umock::Sys_socketMock sys_socketObj;
    umock::Sys_socket sys_socket_injectObj(&sys_socketObj);
    EXPECT_CALL(sys_socketObj, socket(_, SOCK_STREAM, 0))
        .WillOnce(SetErrnoAndReturn(EACCES, -1));
    EXPECT_CALL(sys_socketObj, shutdown(_, _)).Times(0);
    umock::UnistdMock unistdObj;
    umock::Unistd unistd_injectObj(&unistdObj);
    EXPECT_CALL(unistdObj, CLOSE_SOCKET_P(_)).Times(0);

    // Mock for connection to a network server
    umock::PupnpHttpRw pupnp_httprw_injectObj(&m_mock_pupnpHttpRwObj);
    EXPECT_CALL(m_mock_pupnpHttpRwObj, private_connect(_, _, _)).Times(0);

    // provide url structures for the http connection
    Curi_type_testurl testurlObj;
    uri_type* dest_url = (uri_type*)&testurlObj;
    uri_type fixed_url;
    memset(&fixed_url, 0xaa, sizeof(uri_type));

    // Test Unit
    SOCKET sockfd;
    ASSERT_EQ(sockfd = http_Connect(dest_url, &fixed_url),
              (SOCKET)UPNP_E_OUTOF_SOCKET)
        << "  # Should be UPNP_E_OUTOF_SOCKET(" << UPNP_E_OUTOF_SOCKET
        << ") but not " << errStr((int)sockfd);
}

TEST_F(HttpConnectIp4FTestSuite, low_level_net_connect_fails) {
    // Expect no socket allocation
    umock::Sys_socketMock sys_socketObj;
    umock::Sys_socket sys_socket_injectObj(&sys_socketObj);
    EXPECT_CALL(sys_socketObj, socket(AF_INET, SOCK_STREAM, 0))
        .WillOnce(Return(m_socketfd));
    EXPECT_CALL(sys_socketObj, shutdown(_, _)).Times(1);
    umock::UnistdMock unistdObj;
    umock::Unistd unistd_injectObj(&unistdObj);
    EXPECT_CALL(unistdObj, CLOSE_SOCKET_P(_)).Times(1);

    // Connection to a network server will fail
    umock::PupnpHttpRw pupnp_httprw_injectObj(&m_mock_pupnpHttpRwObj);
    EXPECT_CALL(m_mock_pupnpHttpRwObj,
                private_connect(m_socketfd, NotNull(), sizeof(sockaddr_in)))
        .WillOnce(Return(-1));

    // provide url structures for the http connection
    Curi_type_testurl testurlObj;
    uri_type* dest_url = (uri_type*)&testurlObj;
    uri_type fixed_url;
    memset(&fixed_url, 0xaa, sizeof(uri_type));

    // Test Unit
    SOCKET sockfd;
    ASSERT_EQ(sockfd = http_Connect(dest_url, &fixed_url),
              (SOCKET)UPNP_E_SOCKET_CONNECT)
        << "  # Should be UPNP_E_SOCKET_CONNECT(" << UPNP_E_SOCKET_CONNECT
        << ") but not " << errStr((int)sockfd);
}

TEST(HttpFixUrl, check_http_FixStrUrl_successful) {
    if (github_actions)
        GTEST_SKIP() << "             known failing test on Github Actions";

#ifdef UPNP_ENABLE_OPEN_SSL
    constexpr char url_str[] =
        "https://user.name@upnplib.net:443/path/dest/?key=value#fragment";
#else
    constexpr char url_str[] =
        "http://user.name@upnplib.net:80/path/dest/?key=value#fragment";
#endif
    uri_type fixed_url;
    memset(&fixed_url, 0xaa, sizeof(uri_type));

    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.get("192.168.10.11", 80);

    // Mock for network address system calls, parse_uri() ask DNS server.
    umock::Netdb netdb_injectObj(&netv4inf);
    EXPECT_CALL(netv4inf, getaddrinfo(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(res), Return(0)));
    EXPECT_CALL(netv4inf, freeaddrinfo(_)).Times(1);

    // Test Unit
    int ret_http_FixStrUrl{UPNP_E_INTERNAL_ERROR};
    EXPECT_EQ(ret_http_FixStrUrl =
                  http_FixStrUrl(url_str, strlen(url_str), &fixed_url),
              UPNP_E_SUCCESS)
        << errStrEx(ret_http_FixStrUrl, UPNP_E_SUCCESS);

    EXPECT_EQ(fixed_url.type, ABSOLUTE);
    EXPECT_EQ(fixed_url.path_type, ABS_PATH);
    EXPECT_STREQ(fixed_url.scheme.buff,
                 "http://upnplib.net:80/path/?key=value#fragment");
    EXPECT_EQ(fixed_url.scheme.size, (size_t)4);
    EXPECT_STREQ(fixed_url.hostport.text.buff,
                 "upnplib.net:80/path/?key=value#fragment");
    EXPECT_EQ(fixed_url.hostport.text.size, (size_t)14);
    EXPECT_STREQ(fixed_url.pathquery.buff, "/path/?key=value#fragment");
    EXPECT_EQ(fixed_url.pathquery.size, (size_t)16);
    EXPECT_STREQ(fixed_url.fragment.buff, "fragment");
    EXPECT_EQ(fixed_url.fragment.size, (size_t)8);

    struct sockaddr_in* sai4 =
        (struct sockaddr_in*)&fixed_url.hostport.IPaddress;
    EXPECT_EQ(sai4->sin_family, AF_INET);
    EXPECT_EQ(sai4->sin_port, htons(80));
    EXPECT_STREQ(inet_ntoa(sai4->sin_addr), "192.168.10.11");
}

TEST(HttpFixUrl, empty_url_structure) {
    uri_type url{};
    uri_type fixed_url;
    memset(&fixed_url, 0xaa, sizeof(uri_type));

    // Test Unit
    Chttpreadwrite_old httprw_oObj;
    EXPECT_EQ(httprw_oObj.http_FixUrl(&url, &fixed_url), UPNP_E_INVALID_URL);

    // The input url structure isn't modified
    uri_type ref_url{};
    EXPECT_EQ(memcmp(&ref_url, &url, sizeof(uri_type)), 0);
    // The output url structure is modified
    memset(&ref_url, 0xaa, sizeof(uri_type));
    EXPECT_NE(memcmp(&ref_url, &fixed_url, sizeof(uri_type)), 0);
}

TEST(HttpFixUrl, fix_url_no_path_and_query_successful) {
    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.get("192.168.10.10", 80);

    // Mock for network address system calls, parse_uri() asks the DNS server.
    umock::Netdb netdb_injectObj(&netv4inf);
    EXPECT_CALL(netv4inf, getaddrinfo(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(res), Return(0)));
    EXPECT_CALL(netv4inf, freeaddrinfo(_)).Times(1);

    // Get a uri structure with parse_uri()
    constexpr char url_str[] = "http://upnplib.net#fragment";
    uri_type url;
    Curi uriObj;
    EXPECT_EQ(uriObj.parse_uri(url_str, strlen(url_str), &url), HTTP_SUCCESS);

    // Test Unit
    uri_type fixed_url;
    Chttpreadwrite_old httprw_oObj;
    EXPECT_EQ(httprw_oObj.http_FixUrl(&url, &fixed_url), UPNP_E_SUCCESS);

    // The relevant part (e.g. "http") in the token buffer is specified by the
    // token size. But it is no problem to compare the whole buffer because it
    // is defined to contain a C string.
    EXPECT_EQ(fixed_url.type, ABSOLUTE);
    EXPECT_EQ(fixed_url.path_type, OPAQUE_PART);
    EXPECT_STREQ(fixed_url.scheme.buff, "http://upnplib.net#fragment");
    EXPECT_EQ(fixed_url.scheme.size, (size_t)4);
    EXPECT_STREQ(fixed_url.hostport.text.buff, "upnplib.net#fragment");
    EXPECT_EQ(fixed_url.hostport.text.size, (size_t)11);
    EXPECT_STREQ(fixed_url.pathquery.buff, "/");
    EXPECT_EQ(fixed_url.pathquery.size, (size_t)1);
    EXPECT_STREQ(fixed_url.fragment.buff, "fragment");
    EXPECT_EQ(fixed_url.fragment.size, (size_t)8);
}

TEST(HttpFixUrl, nullptr_to_url) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    // Test Unit
    uri_type fixed_url;

    if (old_code) {
        std::cout << CRED "[ BUG      ] " CRES << __LINE__
                  << ": A nullptr to the input url must not segfault.\n";

    } else {

        Chttpreadwrite_old httprw_oObj;
        ASSERT_EXIT((httprw_oObj.http_FixUrl(nullptr, &fixed_url), exit(0)),
                    ::testing::ExitedWithCode(0), ".*")
            << "  BUG! A nullptr to the input url must not segfault.";
        EXPECT_EQ(httprw_oObj.http_FixUrl(nullptr, &fixed_url),
                  UPNP_E_INVALID_URL);
    }
}

TEST(HttpFixUrl, nullptr_to_fixed_url) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    // Test Unit
    Curi_type_testurl testurlObj;
    uri_type* url = (uri_type*)&testurlObj;

    if (old_code) {
        std::cout << CRED "[ BUG      ] " CRES << __LINE__
                  << ": A nullptr to the fixed url must not segfault.\n";

    } else {

        Chttpreadwrite_old httprw_oObj;
        ASSERT_EXIT((httprw_oObj.http_FixUrl(url, nullptr), exit(0)),
                    ::testing::ExitedWithCode(0), ".*")
            << "  BUG! A nullptr to the fixed url must not segfault.";
        EXPECT_EQ(httprw_oObj.http_FixUrl(url, nullptr), UPNP_E_INVALID_URL);
    }
}

TEST(HttpFixUrl, wrong_scheme_ftp) {
    // Get a uri structure with parse_uri()
    constexpr char url_str[] = "ftp://192.168.169.170:80#fragment";
    uri_type url;
    Curi uriObj;
    EXPECT_EQ(uriObj.parse_uri(url_str, strlen(url_str), &url), HTTP_SUCCESS);

    EXPECT_STREQ(url.scheme.buff, "ftp://192.168.169.170:80#fragment");
    EXPECT_EQ(url.scheme.size, (size_t)3);
    EXPECT_STREQ(url.pathquery.buff, "#fragment");
    EXPECT_EQ(url.pathquery.size, (size_t)0);

    // Test Unit
    uri_type fixed_url;
    Chttpreadwrite_old httprw_oObj;
    EXPECT_EQ(httprw_oObj.http_FixUrl(&url, &fixed_url), UPNP_E_INVALID_URL);

    EXPECT_STREQ(fixed_url.scheme.buff, "ftp://192.168.169.170:80#fragment");
    EXPECT_EQ(fixed_url.scheme.size, (size_t)3);
    EXPECT_STREQ(fixed_url.pathquery.buff, "#fragment");
    EXPECT_EQ(fixed_url.pathquery.size, (size_t)0);
}

TEST(HttpFixUrl, no_fragment) {
    // Get a uri structure with parse_uri()
    constexpr char url_str[] = "http://192.168.169.170:80/path/?key=value";
    uri_type url;
    Curi uriObj;
    EXPECT_EQ(uriObj.parse_uri(url_str, strlen(url_str), &url), HTTP_SUCCESS);

    // Test Unit
    uri_type fixed_url;
    Chttpreadwrite_old httprw_oObj;
    EXPECT_EQ(httprw_oObj.http_FixUrl(&url, &fixed_url), UPNP_E_SUCCESS);

    EXPECT_STREQ(url.scheme.buff, "http://192.168.169.170:80/path/?key=value");
    EXPECT_EQ(url.scheme.size, (size_t)4);
    EXPECT_EQ(url.fragment.buff, nullptr);
    EXPECT_EQ(url.fragment.size, (size_t)0);
}

TEST(GetHostaddr, valid_url_str) {
    const char url_str[]{"https://www.sample.net:49152/tvdevicedesc.xml"};
    const char* hoststr;
    memset(&hoststr, 0xaa, sizeof(hoststr));
    size_t hostlen{0xaa};

    int returned = get_hoststr(url_str, &hoststr, &hostlen);
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);
    EXPECT_STREQ(hoststr, "www.sample.net:49152/tvdevicedesc.xml");
    EXPECT_EQ(hostlen, (size_t)20);
}

TEST(GetHostaddr, wrong_url_str) {
    const char url_str[]{"www.sample.net:49152"};
    const char* hoststr;
    memset(&hoststr, 0xaa, sizeof(hoststr));
    size_t hostlen{0xaa};

    int returned = get_hoststr(url_str, &hoststr, &hostlen);
    EXPECT_EQ(returned, UPNP_E_INVALID_URL)
        << errStrEx(returned, UPNP_E_INVALID_URL);
    const char* refptr;
    memset(&refptr, 0xaa, sizeof(refptr));
    EXPECT_EQ(hoststr, refptr);
    EXPECT_EQ(hostlen, (size_t)0xaa);
}

TEST(GetHostaddr, short_url_str) {
    const char url_str[]{"https://"};
    const char* hoststr;
    memset(&hoststr, 0xaa, sizeof(hoststr));
    size_t hostlen{0xaa};

    int returned = get_hoststr(url_str, &hoststr, &hostlen);
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);
    EXPECT_STREQ(hoststr, "");
    EXPECT_EQ(hostlen, (size_t)0);
}

TEST(GetHostaddr, empty_url_str) {
    const char url_str[]{""};
    const char* hoststr;
    memset(&hoststr, 0xaa, sizeof(hoststr));
    size_t hostlen{0xaa};

    int retval = get_hoststr(url_str, &hoststr, &hostlen);
    EXPECT_EQ(retval, UPNP_E_INVALID_URL)
        << errStrEx(retval, UPNP_E_INVALID_URL);
    const char* refptr;
    memset(&refptr, 0xaa, sizeof(refptr));
    EXPECT_EQ(hoststr, refptr);
    EXPECT_EQ(hostlen, (size_t)0xaa);
}

TEST(GetHostaddr, nullptr_url_str) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    if (old_code) {
        std::cout << CRED "[ BUG      ] " CRES << __LINE__
                  << ": A nullptr to the url string must not segfault.\n";

    } else {

        const char* hoststr;
        memset(&hoststr, 0xaa, sizeof(hoststr));
        size_t hostlen{0xaa};

        // Test Unit
        int retval{UPNP_E_INVALID_URL};
        ASSERT_EXIT(
            (retval = ::get_hoststr(nullptr, &hoststr, &hostlen), exit(0)),
            ::testing::ExitedWithCode(0), ".*")
            << "  BUG! A nullptr to the url string must not segfault.\n";

        EXPECT_EQ(retval, UPNP_E_INVALID_URL)
            << errStrEx(retval, UPNP_E_INVALID_URL);
        const char* refptr;
        memset(&refptr, 0xaa, sizeof(refptr));
        EXPECT_EQ(hoststr, refptr);
        EXPECT_EQ(hostlen, (size_t)0xaa);
    }
}

TEST(CUri, construct_successful) {
    CUri url("https://www.sample.net:49152/tvdevicedesc.xml");
    EXPECT_EQ(url.url_str, "https://www.sample.net:49152/tvdevicedesc.xml");
    EXPECT_EQ(url.hostport, "www.sample.net:49152");
    EXPECT_EQ(url.hostport.size(), (std::string::size_type)20);
}

TEST(CUri, invalid_scheme_delimiter) {
    EXPECT_THROW(CUri url("https//www.sample.net:49152/tvdevicedesc.xml"),
                 std::invalid_argument);
    // EXPECT_TRUE(url.url_str.empty());
    // EXPECT_TRUE(url.hostport.empty());
    // EXPECT_EQ(url.hostport.size(), (size_t)0);
}

TEST(CUri, no_hostport_delimiter) {
    EXPECT_THROW(CUri url("https://www.sample.net:49152-tvdevicedesc.xml"),
                 std::invalid_argument);
}

TEST(CUri, only_hostport_url) {
    // This is syntactic the same as 'no_hostport_delimiter' but it's valid. It
    // needs semantic analysis.
    if (github_actions)
        GTEST_SKIP() << "             known failing test on Github Actions";

    CUri url("https://www.sample.net:49152");
    EXPECT_EQ(url.url_str, "https://www.sample.net:49152");
    EXPECT_EQ(url.hostport, "www.sample.net:49152");
    EXPECT_EQ(url.hostport.size(), (size_t)20);
}

TEST(CUri, empty_hostport_in_url_str) {
    if (github_actions)
        GTEST_SKIP() << "             known failing test on Github Actions";
    CUri url("https:///");
    EXPECT_EQ(url.url_str, "https:///");
    EXPECT_TRUE(url.hostport.empty());
    EXPECT_EQ(url.hostport.size(), (size_t)0);
}

TEST(CUri, only_scheme) {
    if (github_actions)
        GTEST_SKIP() << "             known failing test on Github Actions";

    CUri url("https://");
    EXPECT_EQ(url.url_str, "https://");
    EXPECT_TRUE(url.hostport.empty());
    EXPECT_EQ(url.hostport.size(), (size_t)0);
}

TEST(HttpReadWriteIp4TestSuite, http_RecvMessage) {
    // This test will be created when I have understood the message formats.
    // When we have this test then TEST(RunMiniServerTestSuite, handle_request)
    // in gtests/test_miniserver.cpp should also be completed. --Ingo
    GTEST_SKIP() << "Still needs to be done.";
}

TEST(HttpReadWriteIp4TestSuite, http_SendStatusResponse) {
    // This test will be created when I have understood the message formats.
    // When we have this test then TEST(RunMiniServerTestSuite, handle_error)
    // in gtests/test_miniserver.cpp should also be completed. --Ingo
    GTEST_SKIP() << "Still needs to be done.";
}


// testsuite for Ip6 httpreadwrite
// ===============================
// TODO: Improve ip6 tests.
TEST(HttpreadwriteIp6TestSuite, open_http_connection_with_ip_address) {
    // The handle will be allocated on memory by the function and the pointer
    // to it is returned here. As documented we must free it.
    // We can use a generic pointer because the function needs it:
    // void* phandle;
    // But that is bad for type-save C++. So we use the correct type with
    // type cast on the argument.
    GTEST_SKIP() << "This test need to be mocked because it queries internet "
                    "DNS nameserver with long delay.";

    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    http_connection_handle_t* phandle;
    memset(&phandle, 0xaa, sizeof(phandle));

    // Test the Unit
    Chttpreadwrite_old httprw_oObj;
    int returned = httprw_oObj.http_OpenHttpConnection(
        // This is the ip address from http://google.com
        "http://2a00:1450:4001:80e::200e", (void**)&phandle, 3);

    if (old_code) {
        std::cout << CRED "[ BUG      ] " CRES << __LINE__
                  << ": Connecting with an ip6 address should be possible\n";
        EXPECT_EQ(returned, UPNP_E_INVALID_URL)
            << errStrEx(returned, UPNP_E_INVALID_URL);

    } else {

        EXPECT_EQ(returned, UPNP_E_SUCCESS)
            << errStrEx(returned, UPNP_E_SUCCESS);
    }

    // Doing as documented. It's unclear so far what to do if
    // http_OpenHttpConnection() returns with an error.
    ::free(phandle);
}

// testsuite for statcodes
// =======================
TEST(StatcodesTestSuite, http_get_code_text) {
    // const char* code_text = ::http_get_code_text(HTTP_NOT_FOUND);
    // ::std::cout << "code_text: " << code_text << ::std::endl;
    EXPECT_STREQ(::http_get_code_text(HTTP_NOT_FOUND), "Not Found");
    EXPECT_EQ(::http_get_code_text(99), nullptr);
    EXPECT_EQ(::http_get_code_text(600), nullptr);
}

} // namespace compa

//
int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "compa/gtest_main.inc"
    return gtest_return_code; // managed in compa/gtest_main.inc
}
