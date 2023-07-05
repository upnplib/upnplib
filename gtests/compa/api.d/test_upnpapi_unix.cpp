// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-07-05

// Mock network interfaces
// For further information look at https://stackoverflow.com/a/66498073/5014688

#include "pupnp/upnp/src/api/upnpapi.cpp"

#include "upnplib/upnptools.hpp" // For upnplib only
#include "upnplib/gtest_tools_unix.hpp"
#include "umock/ifaddrs_mock.hpp"
#include "umock/net_if_mock.hpp"

#include "upnplib/gtest.hpp"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;

using ::upnplib::testing::CaptureStdOutErr;

using ::upnplib::CIfaddr4;
using ::upnplib::errStrEx;

namespace compa {
bool old_code{false}; // Managed in upnplib_gtest_main.inc
bool github_actions = std::getenv("GITHUB_ACTIONS");

// UpnpApi Testsuite for IP4
//==========================

// This TestSuite is with instantiating mocks
//-------------------------------------------
class UpnpapiIPv4MockTestSuite : public ::testing::Test
// Fixtures for this Testsuite
{
  protected:
    // Provide mocked functions
    umock::IfaddrsMock m_mocked_ifaddrs;
    umock::Net_ifMock m_mocked_net_if;

    // constructor of this testsuite
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
        UpnpSdkInit = 0xAA; // This should not be used and modified here
    }
};

TEST_F(UpnpapiIPv4MockTestSuite, UpnpGetIfInfo_called_with_valid_interface) {
    // provide a network interface
    CIfaddr4 ifaddr4Obj;
    ifaddr4Obj.set("if0v4", "192.168.99.3/11");
    ifaddrs* ifaddr = ifaddr4Obj.get();
    EXPECT_STREQ(ifaddr->ifa_name, "if0v4");

    // Mock system functions
    umock::Ifaddrs ifaddrs_injectObj(&m_mocked_ifaddrs);
    umock::Net_if net_if_injectObj(&m_mocked_net_if);
    EXPECT_CALL(m_mocked_ifaddrs, getifaddrs(_))
        .WillOnce(DoAll(SetArgPointee<0>(ifaddr), Return(0)));
    EXPECT_CALL(m_mocked_ifaddrs, freeifaddrs(ifaddr)).Times(1);
    EXPECT_CALL(m_mocked_net_if, if_nametoindex(_)).WillOnce(Return(2));

    // Test Unit
    int ret_UpnpGetIfInfo = ::UpnpGetIfInfo("if0v4");
    EXPECT_EQ(ret_UpnpGetIfInfo, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpGetIfInfo, UPNP_E_SUCCESS);

    // Check if UpnpSdkInit has been modified
    EXPECT_EQ(UpnpSdkInit, 0xAA);

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
    EXPECT_EQ(gIF_INDEX, (unsigned int)2);
    EXPECT_EQ(LOCAL_PORT_V4, (unsigned short)0);
    EXPECT_EQ(LOCAL_PORT_V6, (unsigned short)0);
    EXPECT_EQ(LOCAL_PORT_V6_ULA_GUA, (unsigned short)0);
}

TEST_F(UpnpapiIPv4MockTestSuite, UpnpGetIfInfo_called_with_unknown_interface) {
    // provide a network interface
    CIfaddr4 ifaddr4Obj;
    ifaddr4Obj.set("eth0", "192.168.77.48/22");
    ifaddrs* ifaddr = ifaddr4Obj.get();
    EXPECT_STREQ(ifaddr->ifa_name, "eth0");

    umock::Ifaddrs ifaddrs_injectObj(&m_mocked_ifaddrs);
    umock::Net_if net_if_injectObj(&m_mocked_net_if);
    EXPECT_CALL(m_mocked_ifaddrs, getifaddrs(_))
        .WillOnce(DoAll(SetArgPointee<0>(ifaddr), Return(0)));
    EXPECT_CALL(m_mocked_ifaddrs, freeifaddrs(ifaddr)).Times(1);
    EXPECT_CALL(m_mocked_net_if, if_nametoindex(_)).Times(0);

    // Test Unit
    // "ATTENTION! There is a wrong upper case 'O', not zero in 'ethO'";
    int ret_UpnpGetIfInfo = UpnpGetIfInfo("ethO");
    EXPECT_EQ(ret_UpnpGetIfInfo, UPNP_E_INVALID_INTERFACE)
        << errStrEx(ret_UpnpGetIfInfo, UPNP_E_INVALID_INTERFACE);

    // Check if UpnpSdkInit has been modified
    EXPECT_EQ(UpnpSdkInit, 0xAA);

    if (old_code) {
        std::cout
            << CRED "[ BUG      ] " CRES << __LINE__
            << ": interface name (e.g. ethO with upper case O), ip "
            << "address and netmask should not be modified on wrong entries.\n";
        // gIF_NAME mocked with getifaddrs above
        // ATTENTION! There is a wrong upper case 'O', not zero in "ethO";
        EXPECT_STREQ(gIF_NAME, "ethO");
        // gIF_IPV4 with "192.68.77.48/22" mocked by getifaddrs above
        // but get an empty ip address from initialization. That's OK.
        EXPECT_STREQ(gIF_IPV4, "");
        // get an empty netmask from initialization. That's also OK.
        EXPECT_STREQ(gIF_IPV4_NETMASK, "");

    } else if (!github_actions) {

        // gIF_NAME mocked with getifaddrs above
        EXPECT_STREQ(gIF_NAME, "")
            << "  # ATTENTION! There is a wrong upper case 'O', not zero in "
               "\"ethO\".\n"
            << "  # Interface name should not be modified on wrong entries.";
        // gIF_IPV4 mocked with getifaddrs above
        EXPECT_STREQ(gIF_IPV4, "")
            << "  # Ip address should not be modified on wrong entries.";
        EXPECT_STREQ(gIF_IPV4_NETMASK, "")
            << "  # Netmask should not be modified on wrong entries.";
    }

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

TEST_F(UpnpapiIPv4MockTestSuite, UpnpInit2_default_initialization) {
    // provide a network interface
    CIfaddr4 ifaddr4Obj;
    ifaddr4Obj.set("if0v4", "192.168.99.3/20");
    ifaddrs* ifaddr = ifaddr4Obj.get();
    EXPECT_STREQ(ifaddr->ifa_name, "if0v4");

    // expect calls to system functions (which are mocked)
    umock::Ifaddrs ifaddrs_injectObj(&m_mocked_ifaddrs);
    umock::Net_if net_if_injectObj(&m_mocked_net_if);
    EXPECT_CALL(m_mocked_ifaddrs, getifaddrs(_))
        .WillOnce(DoAll(SetArgPointee<0>(ifaddr), Return(0)));
    EXPECT_CALL(m_mocked_ifaddrs, freeifaddrs(ifaddr)).Times(1);
    EXPECT_CALL(m_mocked_net_if, if_nametoindex(_)).Times(1);

    // Initialize capturing of the stderr output
    CaptureStdOutErr captureObj(STDERR_FILENO);
    captureObj.start();

    // Test Unit
    UpnpSdkInit = 0;
    int ret_UpnpInit2 = UpnpInit2(nullptr, 0);
    EXPECT_EQ(ret_UpnpInit2, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpInit2, UPNP_E_SUCCESS);

    // Get and check the captured data
    std::string capturedStderr = captureObj.get();
    EXPECT_EQ(capturedStderr, "")
        << "  There should not be any output to stderr.";

    EXPECT_EQ(UpnpSdkInit, 1);

    // call the unit again to check if it returns to be already initialized
    ret_UpnpInit2 = UpnpInit2(nullptr, 0);
    EXPECT_EQ(ret_UpnpInit2, UPNP_E_INIT)
        << errStrEx(ret_UpnpInit2, UPNP_E_INIT);
    EXPECT_EQ(UpnpSdkInit, 1);

    // Finish library
    int ret_UpnpFinish = UpnpFinish();
    EXPECT_EQ(ret_UpnpFinish, UPNP_E_SUCCESS)
        << errStrEx(ret_UpnpFinish, UPNP_E_SUCCESS);
    EXPECT_EQ(UpnpSdkInit, 0);

    // Finish library again
    ret_UpnpFinish = UpnpFinish();
    EXPECT_EQ(ret_UpnpFinish, UPNP_E_FINISH)
        << errStrEx(ret_UpnpFinish, UPNP_E_FINISH);
    EXPECT_EQ(UpnpSdkInit, 0);
}

} // namespace compa

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
#include "compa/gtest_main.inc"
    return gtest_return_code; // managed in compa/gtest_main.inc
}
