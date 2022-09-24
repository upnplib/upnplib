// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-09-25

// Mock network interfaces
// For further information look at https://stackoverflow.com/a/66498073/5014688

#include "pupnp/upnp/src/api/upnpapi.cpp"

#include "upnplib/upnptools.hpp" // For upnplib_native only
#include "upnplib/gtest_tools_unix.hpp"
#include "upnplib/mocking/ifaddrs.hpp"
#include "upnplib/mocking/net_if.hpp"

#include "gmock/gmock.h"
#include "upnplib/gtest.hpp"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;

using ::upnplib::mocking::Ifaddrs;
using ::upnplib::mocking::IfaddrsInterface;
using ::upnplib::mocking::Net_if;
using ::upnplib::mocking::Net_ifInterface;

using ::upnplib::testing::CaptureStdOutErr;

namespace upnplib {
bool old_code{false}; // Managed in upnplib_gtest_main.inc
bool github_actions = std::getenv("GITHUB_ACTIONS");

//
class IfaddrsMock : public IfaddrsInterface {
  public:
    virtual ~IfaddrsMock() override {}
    MOCK_METHOD(int, getifaddrs, (struct ifaddrs**), (override));
    MOCK_METHOD(void, freeifaddrs, (struct ifaddrs*), (override));
};

class Net_ifMock : public Net_ifInterface {
  public:
    virtual ~Net_ifMock() override {}
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
    IfaddrsMock m_mocked_ifaddrs;
    Net_ifMock m_mocked_net_if;

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
    }
};

TEST_F(UpnpapiIPv4MockTestSuite, UpnpGetIfInfo_called_with_valid_interface) {
    // provide a network interface
    struct ifaddrs* ifaddr = nullptr;

    CIfaddr4 ifaddr4Obj;
    ifaddr4Obj.set("if0v4", "192.168.99.3/11");
    ifaddr = ifaddr4Obj.get();
    EXPECT_STREQ(ifaddr->ifa_name, "if0v4");

    Ifaddrs ifaddrs_injectObj(&m_mocked_ifaddrs);
    Net_if net_if_injectObj(&m_mocked_net_if);
    EXPECT_CALL(m_mocked_ifaddrs, getifaddrs(_))
        .WillOnce(DoAll(SetArgPointee<0>(ifaddr), Return(0)));
    EXPECT_CALL(m_mocked_ifaddrs, freeifaddrs(ifaddr)).Times(1);
    EXPECT_CALL(m_mocked_net_if, if_nametoindex(_)).WillOnce(Return(2));

    // call the unit
    int returned = ::UpnpGetIfInfo("if0v4");
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);

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
    if (github_actions && !old_code)
        GTEST_SKIP() << "             known failing test on Github Actions";

    // provide a network interface
    struct ifaddrs* ifaddr = nullptr;

    CIfaddr4 ifaddr4Obj;
    ifaddr4Obj.set("eth0", "192.168.77.48/22");
    ifaddr = ifaddr4Obj.get();
    EXPECT_STREQ(ifaddr->ifa_name, "eth0");

    Ifaddrs ifaddrs_injectObj(&m_mocked_ifaddrs);
    Net_if net_if_injectObj(&m_mocked_net_if);
    EXPECT_CALL(m_mocked_ifaddrs, getifaddrs(_))
        .WillOnce(DoAll(SetArgPointee<0>(ifaddr), Return(0)));
    EXPECT_CALL(m_mocked_ifaddrs, freeifaddrs(ifaddr)).Times(1);
    EXPECT_CALL(m_mocked_net_if, if_nametoindex(_)).Times(0);

    // Test Unit
    // "ATTENTION! There is a wrong upper case 'O', not zero in 'ethO'";
    int returned = UpnpGetIfInfo("ethO");
    EXPECT_EQ(returned, UPNP_E_INVALID_INTERFACE)
        << errStrEx(returned, UPNP_E_INVALID_INTERFACE);

    if (old_code) {
        std::cout
            << "  BUG! Interface name (e.g. ethO with upper case O), ip "
            << "address and netmask should not be modified on wrong entries.\n";
        // gIF_NAME mocked with getifaddrs above
        // ATTENTION! There is a wrong upper case 'O', not zero in "ethO";
        EXPECT_STREQ(gIF_NAME, "ethO");
        // gIF_IPV4 with "192.68.77.48/22" mocked by getifaddrs above
        // but get an empty ip address from initialization. That's OK.
        EXPECT_STREQ(gIF_IPV4, "");
        // get an empty netmask from initialization. That's also OK.
        EXPECT_STREQ(gIF_IPV4_NETMASK, "");

    } else {

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
    struct ifaddrs* ifaddr = ifaddr4Obj.get();
    EXPECT_STREQ(ifaddr->ifa_name, "if0v4");

    // expect calls to system functions (which are mocked)
    Ifaddrs ifaddrs_injectObj(&m_mocked_ifaddrs);
    Net_if net_if_injectObj(&m_mocked_net_if);
    EXPECT_CALL(m_mocked_ifaddrs, getifaddrs(_))
        .WillOnce(DoAll(SetArgPointee<0>(ifaddr), Return(0)));
    EXPECT_CALL(m_mocked_ifaddrs, freeifaddrs(ifaddr)).Times(1);
    EXPECT_CALL(m_mocked_net_if, if_nametoindex(_)).Times(1);

    // Initialize capturing of the stderr output
    CaptureStdOutErr captureObj(STDERR_FILENO);
    captureObj.start();

    // call the unit
    // EXPECT_EQ(UpnpSdkInit, 0);
    int returned = UpnpInit2(nullptr, 0);
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);

    // Get and check the captured data
    std::string capturedStderr = captureObj.get();
    EXPECT_EQ(capturedStderr, "")
        << "  There should not be any output to stderr.";

    // EXPECT_EQ(UpnpSdkInit, 1);

    // call the unit again
    returned = UpnpInit2(nullptr, 0);
    EXPECT_EQ(returned, UPNP_E_INIT) << errStrEx(returned, UPNP_E_INIT);
    // EXPECT_EQ(UpnpSdkInit, 1);

    // Finish library
    returned = UpnpFinish();
    EXPECT_EQ(returned, UPNP_E_SUCCESS) << errStrEx(returned, UPNP_E_SUCCESS);

    // Finish library again
    returned = UpnpFinish();
    EXPECT_EQ(returned, UPNP_E_FINISH) << errStrEx(returned, UPNP_E_FINISH);
}

} // namespace upnplib

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
#include "upnplib/gtest_main.inc"
}
