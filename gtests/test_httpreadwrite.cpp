// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-01-20

#include "gmock/gmock.h"
#include "upnplib_gtest_tools.hpp"
#include "upnpmock/netdb.hpp"

// Include source code for testing. So we have also direct access to static
// functions which need to be tested.
#include "pupnp/upnp/src/genlib/net/http/httpreadwrite.cpp"
#include "core/src/genlib/net/http/httpreadwrite.cpp"

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;

namespace upnplib {

bool old_code{false}; // Managed in upnplib_gtest_main.inc
bool github_actions = ::std::getenv("GITHUB_ACTIONS");

//
// ######################################
// Mocking
// ######################################

class Mock_netdb : public Bnetdb
// Class to mock the free system functions.
{
  private:
    Bnetdb* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_netdb() {
        m_oldptr = netdb_h;
        netdb_h = this;
    }
    virtual ~Mock_netdb() override { netdb_h = m_oldptr; }

    MOCK_METHOD(int, getaddrinfo,
                (const char* node, const char* service,
                 const struct addrinfo* hints, struct addrinfo** res),
                (override));
    MOCK_METHOD(void, freeaddrinfo, (struct addrinfo * res), (override));
};

class Mock_netv4info : public Mock_netdb
// This is a derived class from mocking netdb to provide a structure for
// addrinfo that can be given to the mocked program.
{
  private:
    Bnetdb* m_oldptr;

    // Provide structures to mock system call for network address
    struct sockaddr_in m_sa {};
    struct addrinfo m_res {};

  public:
    // Save and restore the old pointer to the production function
    Mock_netv4info() {
        m_oldptr = netdb_h;
        netdb_h = this;

        m_sa.sin_family = AF_INET;
    }

    virtual ~Mock_netv4info() override { netdb_h = m_oldptr; }

    addrinfo* set(const char* a_ipaddr, short int a_port) {
        inet_pton(m_sa.sin_family, a_ipaddr, &m_sa.sin_addr);
        m_sa.sin_port = htons(a_port);

        m_res.ai_family = m_sa.sin_family;
        m_res.ai_addrlen = sizeof(struct sockaddr);
        m_res.ai_addr = (sockaddr*)&m_sa;

        return &m_res;
    }
};

//
// ######################################
// testsuite for Ip4 httpreadwrite
// ######################################
#if false
TEST(OpenHttpConnectionTestSuite, open_connection_successful) {
    // The handle will be allocated on memory by the function and the pointer
    // to it is returned here. As documented we must free it.
    // We can use a generic pointer because the function needs it:
    // void* phandle;
    // But that is bad for type-save C++. So we use the correct type with
    // type cast on the argument.
    http_connection_handle_t* phandle;

    Mock_netv4info netv4inf;
    addrinfo* res = netv4inf.set("192.168.10.10", 80);

    // Mock for network address system call
    EXPECT_CALL(netv4inf, getaddrinfo(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(res), Return(0)));
    EXPECT_CALL(netv4inf, freeaddrinfo(_)).Times(1);

    // uri_type out;
    // struct sockaddr_in* sai4 = (struct sockaddr_in*)&out.hostport.IPaddress;

    // Test Unit
    Chttpreadwrite_old httprw_oObj;
    int returned = httprw_oObj.http_OpenHttpConnection("http://upnplib.net",
                                                       (void**)&phandle, 3);

    EXPECT_EQ(returned, UPNP_E_SUCCESS)
        << "  # Should be UPNP_E_SUCCESS(" << UPNP_E_SUCCESS
        << ") but not " << UpnpGetErrorMessage(returned) << '(' << returned
        << ").";

    // Doing as documented. It's unclear so far what to do if
    // http_OpenHttpConnection() returns with an error.
    ::free(phandle);
}
#endif

TEST(OpenHttpConnectionTestSuite, open_http_connection_to_local_ip_address) {
    // For details look at test "open_connection_successful".
    http_connection_handle_t* phandle;

    // Test the Unit
    Chttpreadwrite_old httprw_oObj;
    int returned = httprw_oObj.http_OpenHttpConnection("http://127.0.0.1",
                                                       (void**)&phandle, 3);
// TODO: Check why failing is different
#ifdef _WIN32
    EXPECT_EQ(returned, UPNP_E_SOCKET_ERROR)
        << "  # Should be UPNP_E_SOCKET_ERROR(" << UPNP_E_SOCKET_ERROR
        << ") but not " << UpnpGetErrorMessage(returned) << '(' << returned
        << ").";
#else
    EXPECT_EQ(returned, UPNP_E_SOCKET_CONNECT)
        << "  # Should be UPNP_E_SOCKET_CONNECT(" << UPNP_E_SOCKET_CONNECT
        << ") but not " << UpnpGetErrorMessage(returned) << '(' << returned
        << ").";
#endif

    // Doing as documented. It's unclear so far what to do if
    // http_OpenHttpConnection() returns with an error.
    ::free(phandle);
}

//
// ######################################
// testsuite for Ip6 httpreadwrite
// ######################################
TEST(HttpreadwriteIp6TestSuite, open_http_connection_with_ip_address) {
    // The handle will be allocated on memory by the function and the pointer
    // to it is returned here. As documented we must free it.
    // We can use a generic pointer because the function needs it:
    // void* phandle;
    // But that is bad for type-save C++. So we use the correct type with
    // type cast on the argument.
    http_connection_handle_t* phandle;

    // Test the Unit
    Chttpreadwrite_old httprw_oObj;
    int returned = httprw_oObj.http_OpenHttpConnection(
        // This is the ip address from http://google.com
        "http://2a00:1450:4001:80e::200e", (void**)&phandle, 3);

    if (old_code) {
        ::std::cout << "[ BUG!     ] Connecting with an ip6 address should be "
                       "possible\n";
        EXPECT_EQ(returned, UPNP_E_INVALID_URL)
            << "  # Should be UPNP_E_INVALID_URL(" << UPNP_E_INVALID_URL
            << ") but not " << UpnpGetErrorMessage(returned) << '(' << returned
            << ").";

    } else if (github_actions) {
        ::std::cout << "[  SKIPPED ] Test on Github Actions\n";
        SUCCEED();
    } else {

        EXPECT_EQ(returned, UPNP_E_SUCCESS)
            << "  # Should be UPNP_E_SUCCESS(" << UPNP_E_SUCCESS << ") but not "
            << UpnpGetErrorMessage(returned) << '(' << returned << ").";
    }

    // Doing as documented. It's unclear so far what to do if
    // http_OpenHttpConnection() returns with an error.
    ::free(phandle);
}

// ######################################
// testsuite for statcodes
// ######################################
TEST(StatcodesTestSuite, http_get_code_text) {
    // const char* code_text = ::http_get_code_text(HTTP_NOT_FOUND);
    // ::std::cout << "code_text: " << code_text << ::std::endl;
    EXPECT_STREQ(::http_get_code_text(HTTP_NOT_FOUND), "Not Found");
    EXPECT_EQ(::http_get_code_text(99), nullptr);
    EXPECT_EQ(::http_get_code_text(600), nullptr);
}

} // namespace upnplib

//
int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include "upnplib_gtest_main.inc"
}

// vim: nowrap
