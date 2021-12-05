// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-12-05

// Mock network interfaces
// For further information look at https://stackoverflow.com/a/66498073/5014688

#include "gmock/gmock.h"
#include "custom_gtest_tools_all.hpp"
#include "custom_gtest_tools_win32.hpp"
#include "port_unistd.hpp"

#include "api/upnpapi.cpp"

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;

namespace upnp {

class Mock_iphlpapi : public Biphlpapi {
    // Class to mock the free system functions.
    Biphlpapi* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_iphlpapi() {
        m_oldptr = iphlpapi_h;
        iphlpapi_h = this;
    }
    virtual ~Mock_iphlpapi() { iphlpapi_h = m_oldptr; }

    MOCK_METHOD(ULONG, GetAdaptersAddresses,
                (ULONG Family, ULONG Flags, PVOID Reserved,
                 PIP_ADAPTER_ADDRESSES AdapterAddresses, PULONG SizePointer),
                (override));
};

// UpnpApi Testsuite for IP4
//==========================

// This TestSuite is with instantiating mocks
//-------------------------------------------
class UpnpapiIPv4MockTestSuite : public ::testing::Test
// Fixtures for this Testsuite
{
  protected:
    // Provide mocked functions
    Mock_iphlpapi m_mocked_iphlpapi;

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
        std::fill(std::begin(gIF_NAME), std::end(gIF_NAME), 0);
        std::fill(std::begin(gIF_IPV4), std::end(gIF_IPV4), 0);
        std::fill(std::begin(gIF_IPV4_NETMASK), std::end(gIF_IPV4_NETMASK), 0);
        std::fill(std::begin(gIF_IPV6), std::end(gIF_IPV6), 0);
        gIF_IPV6_PREFIX_LENGTH = 0;
        std::fill(std::begin(gIF_IPV6_ULA_GUA), std::end(gIF_IPV6_ULA_GUA), 0);
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
    CNetIf4 ifaddr4Obj{};
    ifaddr4Obj.set(L"if0v4", "192.168.99.3/11");
    ifaddr4Obj.set_ifindex(2);
    ::PIP_ADAPTER_ADDRESSES adapts = ifaddr4Obj.get();
    EXPECT_STREQ(adapts->FriendlyName, L"if0v4");
    ::ULONG Size = 16383;

    EXPECT_CALL(m_mocked_iphlpapi, GetAdaptersAddresses(_, _, _, _, _))
        .Times(2)
        .WillOnce(
            DoAll(SetArgPointee<4>(*&Size), Return(ERROR_BUFFER_OVERFLOW)))
        .WillOnce(DoAll(SetArgPointee<3>(*adapts), Return(ERROR_SUCCESS)));

    // call the unit
    EXPECT_STREQ(UpnpGetErrorMessage(::UpnpGetIfInfo("if0v4")),
                 "UPNP_E_SUCCESS");

    // Check results
    EXPECT_STREQ(gIF_NAME, "if0v4");
    EXPECT_STREQ(gIF_IPV4, "192.168.99.3");
#ifdef OLD_TEST
    std::cout << "  BUG! Netmask should be set to \"255.224.0.0\".\n";
    EXPECT_STREQ(gIF_IPV4_NETMASK, "");
    std::cout << "  BUG! gIF_IPV6 is floating (uninitialized v6_addr) but must "
                 "be set to \"::/128\".\n";
    // EXPECT_STREQ(gIF_IPV6, "::/128");
#else
    EXPECT_STREQ(gIF_IPV4_NETMASK, "255.224.0.0")
        << "  BUG! Netmask should be set.";
    EXPECT_STREQ(gIF_IPV6, "::/128")
        << "  BUG! gIF_IPV6 should be set to \"::/128\".";
#endif
    EXPECT_EQ(gIF_IPV6_PREFIX_LENGTH, (unsigned)0);
    EXPECT_STREQ(gIF_IPV6_ULA_GUA, "");
    EXPECT_EQ(gIF_IPV6_ULA_GUA_PREFIX_LENGTH, (unsigned)0);
    EXPECT_EQ(gIF_INDEX, (const unsigned int)2);
    EXPECT_EQ(LOCAL_PORT_V4, (unsigned short)0);
    EXPECT_EQ(LOCAL_PORT_V6, (unsigned short)0);
    EXPECT_EQ(LOCAL_PORT_V6_ULA_GUA, (unsigned short)0);
}

TEST_F(UpnpapiIPv4MockTestSuite, UpnpGetIfInfo_called_with_unknown_interface) {
    // provide a network interface
    CNetIf4 ifaddr4Obj{};
    ifaddr4Obj.set(L"eth0", "192.168.77.48/22");
    ifaddr4Obj.set_ifindex(2);
    ::PIP_ADAPTER_ADDRESSES adapts = ifaddr4Obj.get();
    EXPECT_STREQ(adapts->FriendlyName, L"eth0");
    ::ULONG Size = 16383;

    EXPECT_CALL(m_mocked_iphlpapi, GetAdaptersAddresses(_, _, _, _, _))
        .Times(4)
        .WillOnce(
            DoAll(SetArgPointee<4>(*&Size), Return(ERROR_BUFFER_OVERFLOW)))
        .WillOnce(DoAll(SetArgPointee<3>(*adapts), Return(ERROR_SUCCESS)))
        .WillOnce(
            DoAll(SetArgPointee<4>(*&Size), Return(ERROR_BUFFER_OVERFLOW)))
        .WillOnce(DoAll(SetArgPointee<3>(*adapts), Return(ERROR_SUCCESS)));

    // call the unit
    // First set a valid interface
    EXPECT_STREQ(UpnpGetErrorMessage(::UpnpGetIfInfo("eth0")),
                 "UPNP_E_SUCCESS");
    // Then ask again
    // "ATTENTION! There is a wrong upper case 'O', not zero in 'ethO'";
    EXPECT_STREQ(UpnpGetErrorMessage(::UpnpGetIfInfo("ethO")),
                 "UPNP_E_INVALID_INTERFACE");

#ifdef OLD_TEST
    std::cout << "  BUG! Interface name (e.g. ethO with upper case O), ip "
              << "address should not be modified on wrong entries.\n";
    // ATTENTION! There is a wrong upper case 'O', not zero in "ethO";
    EXPECT_STREQ(gIF_NAME, "ethO");
    std::cout << "  BUG! Netmask should be set to \"255.255.252.0\".\n";
    EXPECT_STREQ(gIF_IPV4_NETMASK, "");
    std::cout << "  BUG! gIF_IPV6 is floating (uninitialized v6_addr) but must "
                 "be set to \"::/128\".\n";
    // EXPECT_STREQ(gIF_IPV6, "::/128");
#else
    // There is a zero in the name
    EXPECT_STREQ(gIF_NAME, "eth0")
        << "  BUG! Interface name (e.g. ethO with upper case O), ip "
        << "address should not be modified on wrong entries.";
    EXPECT_STREQ(gIF_IPV4_NETMASK, "255.255.252.0")
        << "  BUG! Netmask should be set to \"255.255.252.0\".";
    EXPECT_STREQ(gIF_IPV6, "::/128")
        << "  BUG! gIF_IPV6 is set to \"::\" but should be set to \"::/128\".";
#endif
    EXPECT_STREQ(gIF_IPV4, "192.168.77.48"); // OK
    EXPECT_EQ(gIF_IPV6_PREFIX_LENGTH, (unsigned)0);
    EXPECT_STREQ(gIF_IPV6_ULA_GUA, "");
    EXPECT_EQ(gIF_IPV6_ULA_GUA_PREFIX_LENGTH, (unsigned)0);
    EXPECT_EQ(gIF_INDEX, 2);
    EXPECT_EQ(LOCAL_PORT_V4, (unsigned short)0);
    EXPECT_EQ(LOCAL_PORT_V6, (unsigned short)0);
    EXPECT_EQ(LOCAL_PORT_V6_ULA_GUA, (unsigned short)0);
}

TEST_F(UpnpapiIPv4MockTestSuite, initialize_default_UpnpInit2) {
    // provide a network interface
    CNetIf4 ifaddr4Obj{};
    ifaddr4Obj.set(L"if0v4", "192.168.99.3/20");
    ifaddr4Obj.set_ifindex(2);
    ::PIP_ADAPTER_ADDRESSES adapts = ifaddr4Obj.get();
    EXPECT_STREQ(adapts->FriendlyName, L"if0v4");
    ::ULONG Size = 16383;

    EXPECT_CALL(m_mocked_iphlpapi, GetAdaptersAddresses(_, _, _, _, _))
        .Times(2)
        .WillOnce(
            DoAll(SetArgPointee<4>(*&Size), Return(ERROR_BUFFER_OVERFLOW)))
        .WillOnce(DoAll(SetArgPointee<3>(*adapts), Return(ERROR_SUCCESS)));

    // Initialize capturing of the stderr output
    CCaptureStdOutErr captureObj(STDERR_FILENO);
    captureObj.start();

    // call the unit
    EXPECT_EQ(UpnpSdkInit, 0);
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInit2(NULL, 0)), "UPNP_E_SUCCESS");

    // Get and check the captured data
    std::string capturedStderr = captureObj.get();
    EXPECT_EQ(capturedStderr, "")
        << "  There should not be any output to stderr.";

    EXPECT_EQ(UpnpSdkInit, 1);

    // call the unit again to check if it returns to be already initialized
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
