// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-06-12

#include "pupnp/upnp/src/genlib/miniserver/miniserver.cpp"
#include "pupnp/upnp/src/api/upnpapi.cpp"

#include "upnplib/upnptools.hpp" // For upnplib_native only

#include "gtest/gtest.h"

namespace upnplib {
bool old_code{false}; // Managed in upnplib_gtest_main.inc
bool github_actions = std::getenv("GITHUB_ACTIONS");

//
// simple testsuite without fixtures
//----------------------------------
TEST(MiniserverTestSuite, StartMiniServer_in_context) {
    MINISERVER_REUSEADDR = false;

    // Perform initialization preamble.
    int ret_UpnpInitPreamble = UpnpInitPreamble();
    ASSERT_EQ(ret_UpnpInitPreamble, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpInitPreamble, UPNP_E_SUCCESS);

    // Retrieve interface information (Addresses, index, etc).
    int ret_UpnpGetIfInfo = UpnpGetIfInfo(nullptr);
    ASSERT_EQ(ret_UpnpGetIfInfo, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpGetIfInfo, UPNP_E_SUCCESS);

    // Due to initialization by components it should not have flagged to be
    // initialized. That will we do now.
    ASSERT_EQ(UpnpSdkInit, 0);
    UpnpSdkInit = 1;

    EXPECT_EQ(LOCAL_PORT_V4, 0);
    EXPECT_EQ(LOCAL_PORT_V6, 0);
    EXPECT_EQ(LOCAL_PORT_V6_ULA_GUA, 0);

    // Test Unit
    int ret_StartMiniServer =
        StartMiniServer(&LOCAL_PORT_V4, &LOCAL_PORT_V6, &LOCAL_PORT_V6_ULA_GUA);
    EXPECT_EQ(ret_StartMiniServer, UPNP_E_SUCCESS)
        << errStrEx(ret_StartMiniServer, UPNP_E_SUCCESS);

    EXPECT_EQ(UpnpSdkInit, 1);

    EXPECT_EQ(LOCAL_PORT_V4, 49153);
    EXPECT_EQ(LOCAL_PORT_V6, 0);
    EXPECT_EQ(LOCAL_PORT_V6_ULA_GUA, 0);

    StopMiniServer(); // Always returns 0

    int ret_UpnpFinish = UpnpFinish();
    EXPECT_EQ(ret_UpnpFinish, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpFinish, UPNP_E_SUCCESS);

    EXPECT_EQ(UpnpSdkInit, 0);
}

#if false // Disabled because we need to mock the ip address
TEST(MiniserverTestSuite, StartMiniServer_isolated) {
    MINISERVER_REUSEADDR = false;
    // srand((unsigned int)time(NULL));

    /* Initialize SDK global mutexes. */
    // ASSERT_EQ(ithread_rwlock_init(&GlobalHndRWLock, NULL), 0);
    // ASSERT_EQ(ithread_mutex_init(&gUUIDMutex, NULL), 0);
    // ASSERT_EQ(ithread_mutex_init(&GlobalClientSubscribeMutex, NULL), 0);

    /* Initializes the handle list. */
    // HandleLock();
    // for (int i = 0; i < NUM_HANDLE; ++i) {
    //    HandleTable[i] = NULL;
    //}
    // HandleUnlock();

    /* Initialize SDK global thread pools. */
    ASSERT_EQ(UpnpInitThreadPools(), UPNP_E_SUCCESS);

    /* Initialize the SDK timer thread. */
    // ASSERT_EQ(TimerThreadInit(&gTimerThread, &gSendThreadPool),
    // UPNP_E_SUCCESS);

    // Initialize needed variables
    strncpy(gIF_IPV4, "192.168.24.85", INET_ADDRSTRLEN);
    strncpy(gIF_IPV4_NETMASK, "255.255.255.0", INET_ADDRSTRLEN);

    uint16_t local_port_V4 = 0;
    uint16_t local_port_V6 = 0;
    uint16_t local_port_V6_ULA_GUA = 0;

    // Test Unit
    int ret_StartMiniServer =
        StartMiniServer(&local_port_V4, &local_port_V6, &local_port_V6_ULA_GUA);
    EXPECT_EQ(ret_StartMiniServer, UPNP_E_SUCCESS)
        << errStrEx(ret_StartMiniServer, UPNP_E_SUCCESS);

    EXPECT_EQ(local_port_V4, 49153);
    EXPECT_EQ(local_port_V6, 0);
    EXPECT_EQ(local_port_V6_ULA_GUA, 0);

    // Try again
    local_port_V4 = 0;
    ret_StartMiniServer =
        StartMiniServer(&local_port_V4, &local_port_V6, &local_port_V6_ULA_GUA);
    // This means miniserver is running
    EXPECT_EQ(ret_StartMiniServer, UPNP_E_INTERNAL_ERROR)
        << errStrEx(ret_StartMiniServer, UPNP_E_INTERNAL_ERROR);

    EXPECT_EQ(local_port_V4, 0);
    EXPECT_EQ(local_port_V6, 0);
    EXPECT_EQ(local_port_V6_ULA_GUA, 0);

    // Stop miniserver
    EXPECT_EQ(StopMiniServer(), 0);

    // Start again after stopped
    local_port_V4 = 0;
    ret_StartMiniServer =
        StartMiniServer(&local_port_V4, &local_port_V6, &local_port_V6_ULA_GUA);
    EXPECT_EQ(ret_StartMiniServer, UPNP_E_SUCCESS)
        << errStrEx(ret_StartMiniServer, UPNP_E_SUCCESS);

    EXPECT_EQ(local_port_V4, 49153);
    EXPECT_EQ(local_port_V6, 0);
    EXPECT_EQ(local_port_V6_ULA_GUA, 0);

    // Finaly stop miniserver
    EXPECT_EQ(StopMiniServer(), 0);
}

TEST(MiniserverTestSuite, get_miniserver_sockets) {
    MINISERVER_REUSEADDR = false;

    // Initialize needed structure
    MiniServerSockArray* miniSocket =
        (MiniServerSockArray*)malloc(sizeof(MiniServerSockArray));
    ASSERT_NE(miniSocket, nullptr);
    InitMiniServerSockArray(miniSocket);

    // Test Unit
    int ret_get_miniserver_sockets =
        get_miniserver_sockets(miniSocket, 0, 0, 0);
    EXPECT_EQ(ret_get_miniserver_sockets, UPNP_E_SUCCESS)
        << errStrEx(ret_get_miniserver_sockets, UPNP_E_SUCCESS);
}
#endif    // #if false

TEST(MiniserverTestSuite, init_socket_suff_default) {
    MINISERVER_REUSEADDR = false;

    // Set ip address and needed structure
    constexpr char text_addr[]{"192.168.24.85"};
    struct s_SocketStuff ss4;

    // Test Unit
    EXPECT_EQ(init_socket_suff(&ss4, text_addr, 4), 0);

    EXPECT_GT(ss4.fd, 2);
    EXPECT_EQ(ss4.ip_version, 4);
    EXPECT_STREQ(ss4.text_addr, "192.168.24.85");
    EXPECT_EQ(ss4.serverAddr4->sin_family, AF_INET);
    EXPECT_EQ(ss4.address_len, sizeof(*ss4.serverAddr4));
    EXPECT_STREQ(inet_ntoa(ss4.serverAddr4->sin_addr), "192.168.24.85");

    char reuseaddr;
    socklen_t optlen{sizeof(reuseaddr)};
    EXPECT_EQ(getsockopt(ss4.fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, &optlen),
              0);
    EXPECT_FALSE(reuseaddr);
}

TEST(MiniserverTestSuite, init_socket_suff_reuseaddr) {
    MINISERVER_REUSEADDR = true;

    // Set ip address and needed structure
    constexpr char text_addr[]{"192.168.24.85"};
    struct s_SocketStuff ss4;

    // Test Unit
    EXPECT_EQ(init_socket_suff(&ss4, text_addr, 4), 0);

    char reuseaddr;
    socklen_t optlen{sizeof(reuseaddr)};
    EXPECT_EQ(getsockopt(ss4.fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, &optlen),
              0);
    EXPECT_TRUE(reuseaddr);
}

TEST(MiniserverTestSuite, init_socket_suff_with_invalid_ip_address) {
    MINISERVER_REUSEADDR = false;

    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    // Set ip address and needed structure
    constexpr char text_addr[]{"9999.9999.999.99"};
    struct s_SocketStuff ss4;

    // Test Unit
    if (old_code) {
        std::cout << "  BUG! With an invalid ip address the function call "
                     "should fail.\n";
        EXPECT_EQ(init_socket_suff(&ss4, text_addr, 4), 0);
        EXPECT_STREQ(ss4.text_addr, "9999.9999.999.99");
        EXPECT_GE(ss4.fd, 3);

    } else {

        EXPECT_EQ(init_socket_suff(&ss4, text_addr, 4), 1);
        EXPECT_STREQ(ss4.text_addr, "0.0.0.0");
        EXPECT_EQ(ss4.fd, INVALID_SOCKET);
    }

    EXPECT_EQ(ss4.ip_version, 4);
    EXPECT_EQ(ss4.serverAddr4->sin_family, AF_INET);
    EXPECT_EQ(ss4.address_len, 16);
    EXPECT_STREQ(inet_ntoa(ss4.serverAddr4->sin_addr), "0.0.0.0");
}

TEST(MiniserverTestSuite, init_socket_suff_with_invalid_ip_version) {
    // Set ip address and needed structure
    constexpr char text_addr[]{"192.168.24.85"};
    struct s_SocketStuff ss4;

    // Test Unit
    EXPECT_EQ(init_socket_suff(&ss4, text_addr, 0), 1);

    EXPECT_EQ(ss4.fd, INVALID_SOCKET);
    EXPECT_EQ(ss4.ip_version, 0);
    EXPECT_STREQ(ss4.text_addr, "192.168.24.85");
    EXPECT_EQ(ss4.serverAddr4->sin_family, 0);
    EXPECT_EQ(ss4.address_len, 0);
    EXPECT_STREQ(inet_ntoa(ss4.serverAddr4->sin_addr), "0.0.0.0");
    EXPECT_EQ(ss4.fd, INVALID_SOCKET);
}

} // namespace upnplib

//
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
#include "upnplib/gtest_main.inc"
}
