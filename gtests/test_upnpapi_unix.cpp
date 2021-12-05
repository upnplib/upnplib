// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-12-05

// Mock network interfaces
// For further information look at https://stackoverflow.com/a/66498073/5014688

#include "gmock/gmock.h"
#include "custom_gtest_tools_all.hpp"
#include "custom_gtest_tools_unix.hpp"
#include "api/upnpapi.cpp"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;

namespace upnp {

class Mock_ifaddrs : public Bifaddrs {
    // Class to mock the free system functions.
    Bifaddrs* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_ifaddrs() {
        m_oldptr = ifaddrs_h;
        ifaddrs_h = this;
    }
    virtual ~Mock_ifaddrs() override { ifaddrs_h = m_oldptr; }

    MOCK_METHOD(int, getifaddrs, (struct ifaddrs**), (override));
    MOCK_METHOD(void, freeifaddrs, (struct ifaddrs*), (override));
};

class Mock_net_if : public Bnet_if {
    // Class to mock the free system functions.
    Bnet_if* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_net_if() {
        m_oldptr = net_if_h;
        net_if_h = this;
    }
    virtual ~Mock_net_if() { net_if_h = m_oldptr; }

    MOCK_METHOD(unsigned int, if_nametoindex, (const char* ifname), (override));
};

//
// UpnpApi Testsuite for IP4
//==========================

// This TestSuite is with instantiating mocks
//-------------------------------------------
class UpnpapiIPv4MockTestSuite : public ::testing::Test
// Fixtures for this Testsuite
{
  protected:
    // Provide mocked functions
    Mock_ifaddrs m_mocked_ifaddrs;
    Mock_net_if m_mocked_net_if;

    // constructor of this testsuite
    UpnpapiIPv4MockTestSuite() {
        // initialize global variables with file scope for upnpapi.cpp
        virtualDirCallback = {};
        pVirtualDirList = nullptr;
        // GlobalClientSubscribeMutex = {}; // mutex, must be initialized,
        // only used with gena.h
        GlobalHndRWLock = {}; // mutex, must be initialzed
        // gTimerThread             // must be initialized
        gSDKInitMutex = PTHREAD_MUTEX_INITIALIZER;
        gUUIDMutex = {}; // mutex, must be initialzed
        // gSendThreadPool          // type ThreadPool must be initialized
        // gRecvThreadPool;         // type ThreadPool must be initialized
        // gMiniServerThreadPool;   // type ThreadPool must be initialized
        bWebServerState = WEB_SERVER_DISABLED;
        // Due to a bug there is annoying warning with initializing gIF_*:
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=99578
        // gIF_NAME[LINE_SIZE] = {'\0'};
        // gIF_IPV4[INET_ADDRSTRLEN] = {'\0'};
        // gIF_IPV4_NETMASK[INET_ADDRSTRLEN] = {'\0'};
        // gIF_IPV6[INET6_ADDRSTRLEN] = {'\0'};
        gIF_IPV6_PREFIX_LENGTH = 0;
        // gIF_IPV6_ULA_GUA[INET6_ADDRSTRLEN] = {'\0'};
        gIF_IPV6_ULA_GUA_PREFIX_LENGTH = 0;
        gIF_INDEX = (unsigned)-1;
        LOCAL_PORT_V4 = 0;
        LOCAL_PORT_V6 = 0;
        LOCAL_PORT_V6_ULA_GUA = 0;
        HandleTable[NUM_HANDLE] = {};
        g_maxContentLength = DEFAULT_SOAP_CONTENT_LENGTH;
        g_UpnpSdkEQMaxLen = MAX_SUBSCRIPTION_QUEUED_EVENTS;
        g_UpnpSdkEQMaxAge = MAX_SUBSCRIPTION_EVENT_AGE;
        UpnpSdkInit = 0;
        UpnpSdkClientRegistered = 0;
        UpnpSdkDeviceRegisteredV4 = 0;
        UpnpSdkDeviceregisteredV6 = 0;
#ifdef UPNP_HAVE_OPTSSDP
        gUpnpSdkNLSuuid = {};
#endif /* UPNP_HAVE_OPTSSDP */
#ifdef UPNP_ENABLE_OPEN_SSL
        SSL_CTX* gSslCtx = nullptr;
#endif
    }
};

TEST_F(UpnpapiIPv4MockTestSuite, UpnpGetIfInfo_called_with_valid_interface) {
    // provide a network interface
    struct ifaddrs* ifaddr = nullptr;

    CIfaddr4 ifaddr4Obj;
    ifaddr4Obj.set("if0v4", "192.168.99.3/11");
    ifaddr = ifaddr4Obj.get();
    EXPECT_STREQ(ifaddr->ifa_name, "if0v4");

    EXPECT_CALL(m_mocked_ifaddrs, getifaddrs(_))
        .WillOnce(DoAll(SetArgPointee<0>(ifaddr), Return(0)));
    EXPECT_CALL(m_mocked_ifaddrs, freeifaddrs(ifaddr)).Times(1);
    EXPECT_CALL(m_mocked_net_if, if_nametoindex(_)).WillOnce(Return(2));

    // call the unit
    EXPECT_STREQ(UpnpGetErrorMessage(::UpnpGetIfInfo("if0v4")),
                 "UPNP_E_SUCCESS");

    // gIF_NAME mocked with getifaddrs above
    EXPECT_STREQ(gIF_NAME, "if0v4");
    // gIF_IPV4 mocked with getifaddrs above
    EXPECT_STREQ(gIF_IPV4, "192.168.99.3");
    // EXPECT_THAT(gIF_IPV4,
    // MatchesRegex("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}"));
    EXPECT_STREQ(gIF_IPV4_NETMASK, "255.224.0.0");
    EXPECT_STREQ(gIF_IPV6, "");
    EXPECT_EQ(gIF_IPV6_PREFIX_LENGTH, (unsigned)0);
    EXPECT_STREQ(gIF_IPV6_ULA_GUA, "");
    EXPECT_EQ(gIF_IPV6_ULA_GUA_PREFIX_LENGTH, (unsigned)0);
    // index mocked with if_nametoindex above
    EXPECT_EQ(gIF_INDEX, (const unsigned int)2);
    EXPECT_EQ(LOCAL_PORT_V4, (unsigned short)0);
    EXPECT_EQ(LOCAL_PORT_V6, (unsigned short)0);
    EXPECT_EQ(LOCAL_PORT_V6_ULA_GUA, (unsigned short)0);
}

TEST_F(UpnpapiIPv4MockTestSuite, UpnpGetIfInfo_called_with_unknown_interface) {
    // provide a network interface
    struct ifaddrs* ifaddr = nullptr;

    CIfaddr4 ifaddr4Obj;
    ifaddr4Obj.set("eth0", "192.168.77.48/22");
    ifaddr = ifaddr4Obj.get();
    EXPECT_STREQ(ifaddr->ifa_name, "eth0");

    EXPECT_CALL(m_mocked_ifaddrs, getifaddrs(_))
        .WillOnce(DoAll(SetArgPointee<0>(ifaddr), Return(0)));
    EXPECT_CALL(m_mocked_ifaddrs, freeifaddrs(ifaddr)).Times(1);
    EXPECT_CALL(m_mocked_net_if, if_nametoindex(_)).Times(0);

    // call the unit
    // "ATTENTION! There is a wrong upper case 'O', not zero in 'ethO'";
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpGetIfInfo("ethO")),
                 "UPNP_E_INVALID_INTERFACE");
#ifdef OLD_TEST
    std::cout
        << "  BUG! Interface name (e.g. ethO with upper case O), ip "
        << "address and netmask should not be modified on wrong entries.\n";
    // gIF_NAME mocked with getifaddrs above
    // ATTENTION! There is a wrong upper case 'O', not zero in "ethO";
    EXPECT_STREQ(gIF_NAME, "ethO");
    // gIF_IPV4 with "192.68.77.48/22" mocked by getifaddrs above
    // but get ip address from previous successful call
    EXPECT_STREQ(gIF_IPV4, "192.168.99.3");
    // get netmask from previous successful call
    EXPECT_STREQ(gIF_IPV4_NETMASK, "255.224.0.0");
#else
    // gIF_NAME mocked with getifaddrs above
    EXPECT_STREQ(gIF_NAME, "")
        << "ATTENTION! There is a wrong upper case 'O', not zero in \"ethO\".\n"
        << "# Interface name should not be modified on wrong entries.";
    // gIF_IPV4 mocked with getifaddrs above
    EXPECT_STREQ(gIF_IPV4, "")
        << "# Ip address should not be modified on wrong entries.";
    EXPECT_STREQ(gIF_IPV4_NETMASK, "")
        << "# Netmask should not be modified on wrong entries.";
#endif
    EXPECT_STREQ(gIF_IPV6, "");
    EXPECT_EQ(gIF_IPV6_PREFIX_LENGTH, (unsigned)0);
    EXPECT_STREQ(gIF_IPV6_ULA_GUA, "");
    EXPECT_EQ(gIF_IPV6_ULA_GUA_PREFIX_LENGTH, (unsigned)0);
    // index mocked with if_nametoindex above
    EXPECT_EQ(gIF_INDEX, (unsigned)4294967295) << "    Which is: (unsigned)-1";
    EXPECT_EQ(LOCAL_PORT_V4, (unsigned short)0);
    EXPECT_EQ(LOCAL_PORT_V6, (unsigned short)0);
    EXPECT_EQ(LOCAL_PORT_V6_ULA_GUA, (unsigned short)0);
}

TEST_F(UpnpapiIPv4MockTestSuite, initialize_default_UpnpInit2) {
    // provide a network interface
    CIfaddr4 ifaddr4Obj;
    ifaddr4Obj.set("if0v4", "192.168.99.3/20");
    struct ifaddrs* ifaddr = ifaddr4Obj.get();
    EXPECT_STREQ(ifaddr->ifa_name, "if0v4");

    // expect calls to system functions (which are mocked)
    EXPECT_CALL(m_mocked_ifaddrs, getifaddrs(_))
        .WillOnce(DoAll(SetArgPointee<0>(ifaddr), Return(0)));
    EXPECT_CALL(m_mocked_ifaddrs, freeifaddrs(ifaddr)).Times(1);
    EXPECT_CALL(m_mocked_net_if, if_nametoindex(_)).Times(1);

    // Initialize capturing of the stderr output
    CCaptureStdOutErr captureObj(STDERR_FILENO);
    std::string capturedStderr;
    ASSERT_TRUE(captureObj.start());

    // call the unit
    EXPECT_EQ(UpnpSdkInit, 0);
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInit2(NULL, 0)), "UPNP_E_SUCCESS");

    // Get and check the captured data
    ASSERT_TRUE(captureObj.get(capturedStderr));
    EXPECT_EQ(capturedStderr, "")
        << "  There should not be any output to stderr.";

    EXPECT_EQ(UpnpSdkInit, 1);

    // call the unit again
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInit2(NULL, 0)), "UPNP_E_INIT");
    EXPECT_EQ(UpnpSdkInit, 1);
}

// TEST_F(UpnpapiIPv4MockTestSuite, UpnpInitMutexes) {
// TODO
//    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitMutexes()), "UPNP_E_SUCCESS");
// }

// UpnpApi common Testsuite
//-------------------------
// TEST(UpnpapiTestSuite, WinsockInit) {
// TODO
//    EXPECT_STREQ(UpnpGetErrorMessage(UPNP_E_SUCCESS), "UPNP_E_SUCCESS");
// }

TEST(UpnpapiTestSuite, get_handle_info) {
    Handle_Info** HndInfo = 0;
    EXPECT_EQ(GetHandleInfo(0, HndInfo), HND_INVALID);
    EXPECT_EQ(GetHandleInfo(1, HndInfo), HND_INVALID);
}

TEST(UpnpapiTestSuite, get_error_message) {
    EXPECT_STREQ(UpnpGetErrorMessage(0), "UPNP_E_SUCCESS");
    EXPECT_STREQ(UpnpGetErrorMessage(-121), "UPNP_E_INVALID_INTERFACE");
    EXPECT_STREQ(UpnpGetErrorMessage(1), "Unknown error code");
}

} // namespace upnp

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
