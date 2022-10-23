// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-10-23

// Mock network interfaces
// For further information look at https://stackoverflow.com/a/66498073/5014688

#include "upnpapi.hpp"

#include "upnplib/gtest_tools_win32.hpp"
#include "upnplib/upnptools.hpp" // For upnplib_native only
#include "upnplib/port.hpp"
#include "umock/iphlpapi.hpp"

#include "upnplib/gtest.hpp"
#include "gmock/gmock.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;

using ::upnplib::testing::CaptureStdOutErr;

namespace upnplib {

bool old_code{false}; // Managed in upnplib_gtest_main.inc
bool github_actions = ::std::getenv("GITHUB_ACTIONS");

class IphlpapiMock : public umock::IphlpapiInterface {
  public:
    virtual ~IphlpapiMock() override = default;
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
    IphlpapiMock m_mocked_iphlpapi;

    UpnpapiIPv4MockTestSuite() {
        // initialize needed global variables
        std::fill(std::begin(gIF_NAME), std::end(gIF_NAME), 0);
        std::fill(std::begin(gIF_IPV4), std::end(gIF_IPV4), 0);
        std::fill(std::begin(gIF_IPV4_NETMASK), std::end(gIF_IPV4_NETMASK), 0);
        std::fill(std::begin(gIF_IPV6), std::end(gIF_IPV6), 0);
        gIF_IPV6_PREFIX_LENGTH = 0;
        std::fill(std::begin(gIF_IPV6_ULA_GUA), std::end(gIF_IPV6_ULA_GUA), 0);
        gIF_IPV6_ULA_GUA_PREFIX_LENGTH = 0;
        gIF_INDEX = (unsigned)-1;
    }
};

TEST_F(UpnpapiIPv4MockTestSuite, UpnpGetIfInfo_called_with_valid_interface) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    // provide a network interface
    CNetIf4 ifaddr4Obj{};
    ifaddr4Obj.set(L"if0v4", "192.168.99.3/11");
    ifaddr4Obj.set_ifindex(2);
    ::PIP_ADAPTER_ADDRESSES adapts = ifaddr4Obj.get();
    EXPECT_STREQ(adapts->FriendlyName, L"if0v4");
    ::ULONG Size = 16383;

    umock::Iphlpapi iphlpapi_injectObj(&m_mocked_iphlpapi);
    EXPECT_CALL(m_mocked_iphlpapi, GetAdaptersAddresses(_, _, _, _, _))
        .Times(2)
        .WillOnce(
            DoAll(SetArgPointee<4>(*&Size), Return(ERROR_BUFFER_OVERFLOW)))
        .WillOnce(DoAll(SetArgPointee<3>(*adapts), Return(ERROR_SUCCESS)));

    // Test Unit
    int ret_UpnpGetIfInfo = ::UpnpGetIfInfo("if0v4");
    EXPECT_EQ(ret_UpnpGetIfInfo, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpGetIfInfo, UPNP_E_SUCCESS);

    // Check results
    EXPECT_STREQ(gIF_NAME, "if0v4");
    EXPECT_STREQ(gIF_IPV4, "192.168.99.3");

    if (old_code) {
        std::cout << "  BUG! Netmask should be set to \"255.224.0.0\".\n";
        EXPECT_STREQ(gIF_IPV4_NETMASK, "");
        std::cout << "  BUG! gIF_IPV6 must not be set to any floating ip "
                     "address because IPv6 is disabled.\n";
        EXPECT_STRNE(gIF_IPV6, "");

    } else {

        EXPECT_STREQ(gIF_IPV4_NETMASK, "255.224.0.0");
        EXPECT_STREQ(gIF_IPV6, "");
    }

    EXPECT_EQ(gIF_IPV6_PREFIX_LENGTH, (unsigned)0);
    EXPECT_STREQ(gIF_IPV6_ULA_GUA, "");
    EXPECT_EQ(gIF_IPV6_ULA_GUA_PREFIX_LENGTH, (unsigned)0);
    EXPECT_EQ(gIF_INDEX, (const unsigned int)2);
    EXPECT_EQ(LOCAL_PORT_V4, (unsigned short)0);
    EXPECT_EQ(LOCAL_PORT_V6, (unsigned short)0);
    EXPECT_EQ(LOCAL_PORT_V6_ULA_GUA, (unsigned short)0);
}

TEST_F(UpnpapiIPv4MockTestSuite, UpnpGetIfInfo_called_with_unknown_interface) {
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    // provide a valid network interface
    CNetIf4 ifaddr4Obj{};
    ifaddr4Obj.set(L"eth0", "192.168.77.48/22");
    ifaddr4Obj.set_ifindex(2);
    ::PIP_ADAPTER_ADDRESSES adapts = ifaddr4Obj.get();
    EXPECT_STREQ(adapts->FriendlyName, L"eth0");
    ::ULONG Size = 16383;

    umock::Iphlpapi iphlpapi_injectObj(&m_mocked_iphlpapi);
    EXPECT_CALL(m_mocked_iphlpapi, GetAdaptersAddresses(_, _, _, _, _))
        .Times(4)
        .WillOnce(
            DoAll(SetArgPointee<4>(*&Size), Return(ERROR_BUFFER_OVERFLOW)))
        .WillOnce(DoAll(SetArgPointee<3>(*adapts), Return(ERROR_SUCCESS)))
        .WillOnce(
            DoAll(SetArgPointee<4>(*&Size), Return(ERROR_BUFFER_OVERFLOW)))
        .WillOnce(DoAll(SetArgPointee<3>(*adapts), Return(ERROR_SUCCESS)));

    // Test Unit
    // First set a valid interface
    int ret_UpnpGetIfInfo = ::UpnpGetIfInfo("eth0");
    EXPECT_EQ(ret_UpnpGetIfInfo, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpGetIfInfo, UPNP_E_SUCCESS);
    // Then ask again
    // "ATTENTION! There is a wrong upper case 'O', not zero in 'ethO'";
    ret_UpnpGetIfInfo = ::UpnpGetIfInfo("ethO");
    EXPECT_EQ(ret_UpnpGetIfInfo, UPNP_E_INVALID_INTERFACE)
        << errStrEx(ret_UpnpGetIfInfo, UPNP_E_INVALID_INTERFACE);

    if (old_code) {
        std::cout << "  BUG! Interface name (e.g. ethO with upper case O), ip "
                  << "address should not be modified on wrong entries.\n";
        // ATTENTION! There is a wrong upper case 'O', not zero in "ethO";
        EXPECT_STREQ(gIF_NAME, "ethO");
        std::cout << "  BUG! Netmask should be set to \"255.255.252.0\".\n";
        EXPECT_STREQ(gIF_IPV4_NETMASK, "");
        std::cout << "  BUG! gIF_IPV6 must not be set to any floating ip "
                     "address because IPv6 is disabled.\n";
        EXPECT_STRNE(gIF_IPV6, "");

    } else {

        // There is a zero in the valid name
        EXPECT_STREQ(gIF_NAME, "eth0")
            << "  BUG! Interface name (e.g. ethO with upper case O), ip "
            << "address should not be modified on wrong entries.";
        EXPECT_STREQ(gIF_IPV4_NETMASK, "255.255.252.0");
        EXPECT_STREQ(gIF_IPV6, "");
    }

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
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    if (old_code) {
        GTEST_SKIP() << "  BUG! Bind to a socket fails and should be mocked "
                        "after creating a gtest for miniserver.";
    }

    // provide a network interface
    CNetIf4 ifaddr4Obj{};
    ifaddr4Obj.set(L"if0v4", "192.168.99.3/20");
    ifaddr4Obj.set_ifindex(2);
    ::PIP_ADAPTER_ADDRESSES adapts = ifaddr4Obj.get();
    EXPECT_STREQ(adapts->FriendlyName, L"if0v4");
    ::ULONG Size = 16383;

    umock::Iphlpapi iphlpapi_injectObj(&m_mocked_iphlpapi);
    EXPECT_CALL(m_mocked_iphlpapi, GetAdaptersAddresses(_, _, _, _, _))
        .Times(2)
        .WillOnce(
            DoAll(SetArgPointee<4>(*&Size), Return(ERROR_BUFFER_OVERFLOW)))
        .WillOnce(DoAll(SetArgPointee<3>(*adapts), Return(ERROR_SUCCESS)));

    // Initialize capturing of the stderr output
    CaptureStdOutErr captureObj(STDERR_FILENO);
    captureObj.start();

    // Test Unit
    int ret_UpnpInit2 = UpnpInit2(nullptr, 0);
    EXPECT_EQ(ret_UpnpInit2, UPNP_E_SOCKET_BIND)
        << errStrEx(ret_UpnpInit2, UPNP_E_SOCKET_BIND);

    // Get and check the captured data
    std::string capturedStderr = captureObj.get();
    EXPECT_EQ(capturedStderr, "")
        << "  There should not be any output to stderr.";

    // call the unit again to check if it returns to be already initialized
    ret_UpnpInit2 = UpnpInit2(nullptr, 0);
    EXPECT_EQ(ret_UpnpInit2, UPNP_E_INIT)
        << errStrEx(ret_UpnpInit2, UPNP_E_INIT);
}

} // namespace upnplib

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
#include "upnplib/gtest_main.inc"
}
