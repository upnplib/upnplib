// Copyright (C) 2024+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-07-03

#include <upnplib/netaddr.hpp>

#include <upnplib/addrinfo.hpp>
#include <iostream>
#include <utest/utest.hpp>

namespace utest {

using ::testing::EndsWith;

using ::upnplib::CAddrinfo;
using ::upnplib::is_netaddr;
using ::upnplib::is_numport;
using ::upnplib::Netaddr;


// Netaddr TestSuite
// =================

TEST(NetaddrTestSuite, netaddr_successful) {
    CAddrinfo aiObj("[2001:db8::1]", "61234");
    aiObj.load();
    Netaddr napObj = aiObj.netaddr();
    EXPECT_EQ(napObj.str(), "[2001:db8::1]:61234");

    // CaptureStdOutErr captureObj(STDOUT_FILENO); // or STDERR_FILENO
    // captureObj.start();

    // Test Unit output stream
    // std::cout << std::endl << napObj << std::endl;
    // EXPECT_THAT(captureObj.str(), EndsWith("[2001:db8::1]:61234\n"));

    // Test Unit default copy constructor
    Netaddr nap2Obj = napObj;
    EXPECT_EQ(nap2Obj.str(), "[2001:db8::1]:61234");

    // Test Unit default assignment operator
    Netaddr nap3Obj;
    nap3Obj = nap2Obj;
    EXPECT_EQ(nap3Obj.str(), "[2001:db8::1]:61234");

    // Test Unit concatenating strings
    EXPECT_EQ("netaddrp " + napObj.str(), "netaddrp [2001:db8::1]:61234");
}

TEST(NetaddrTestSuite, is_numeric_node) {
    // Free function 'is_netaddr()' checks only the netaddress without port.
    EXPECT_EQ(is_netaddr("[2001:db8::41]", AF_INET6), AF_INET6);
    EXPECT_EQ(is_netaddr("[2001:db8::42]", AF_INET), AF_UNSPEC);
    EXPECT_EQ(is_netaddr("[2001:db8::42]", AF_UNSPEC), AF_INET6);
    EXPECT_EQ(is_netaddr("192.168.47.91", AF_INET), AF_INET);
    EXPECT_EQ(is_netaddr("192.168.47.92", AF_INET6), AF_UNSPEC);
    EXPECT_EQ(is_netaddr("192.168.47.93", AF_UNSPEC), AF_INET);
    EXPECT_EQ(is_netaddr("192.168.47.8"), AF_INET);
    EXPECT_EQ(is_netaddr("[2001:db8::5]"), AF_INET6);
    EXPECT_EQ(is_netaddr("[2001::db8::6]"), AF_UNSPEC); // double double colon
    EXPECT_EQ(is_netaddr("[12.168.88.93]"), AF_UNSPEC);
    EXPECT_EQ(is_netaddr("[12.168.88.94]", AF_INET6), AF_UNSPEC);
    EXPECT_EQ(is_netaddr("[12.168.88.95]", AF_INET), AF_UNSPEC);
    EXPECT_EQ(is_netaddr("[12.168.88.96]:"), AF_UNSPEC);
    EXPECT_EQ(is_netaddr("[12.168.88.97]:9876"), AF_UNSPEC);
    EXPECT_EQ(is_netaddr("[::1]"), AF_INET6);
    EXPECT_EQ(is_netaddr("127.0.0.1"), AF_INET);
    EXPECT_EQ(is_netaddr("[::]"), AF_INET6);
    EXPECT_EQ(is_netaddr("0.0.0.0"), AF_INET);
    EXPECT_EQ(is_netaddr("::1"), AF_UNSPEC);
    EXPECT_EQ(is_netaddr(" ::1"), AF_UNSPEC);
    EXPECT_EQ(is_netaddr(":::1"), AF_UNSPEC);
    EXPECT_EQ(is_netaddr("[::1"), AF_UNSPEC);
    EXPECT_EQ(is_netaddr("::1]"), AF_UNSPEC);
    EXPECT_EQ(is_netaddr("["), AF_UNSPEC);
    EXPECT_EQ(is_netaddr("]"), AF_UNSPEC);
    EXPECT_EQ(is_netaddr(":"), AF_UNSPEC);
    EXPECT_EQ(is_netaddr("[]"), AF_UNSPEC);
    EXPECT_EQ(is_netaddr("::"), AF_UNSPEC);
    EXPECT_EQ(is_netaddr("[:]"), AF_UNSPEC);
    EXPECT_EQ(is_netaddr(""), AF_UNSPEC);
    // This should be an invalid address family
    EXPECT_EQ(is_netaddr("[2001:db8::99]", 67890), AF_UNSPEC);
    EXPECT_EQ(is_netaddr("192.168.77.77", 67891), AF_UNSPEC);
    // Next are never numeric addresses
    EXPECT_EQ(is_netaddr("localhost"), AF_UNSPEC);
    EXPECT_EQ(is_netaddr("localhost", AF_INET6), AF_UNSPEC);
    EXPECT_EQ(is_netaddr("localhost", AF_INET), AF_UNSPEC);
    EXPECT_EQ(is_netaddr("localhost", AF_UNSPEC), AF_UNSPEC);
    EXPECT_EQ(is_netaddr("example.com"), AF_UNSPEC);
    EXPECT_EQ(is_netaddr("[fe80::37%1]"), AF_INET6);
    EXPECT_EQ(is_netaddr("[2001:db8::77:78%2]"), AF_INET6);
    EXPECT_EQ(is_netaddr("[192.168.101.102%3]"), AF_UNSPEC);
    EXPECT_EQ(is_netaddr("192.168.101.102%4"), AF_UNSPEC);
    EXPECT_EQ(is_netaddr("[fe80::db8:4%5]", AF_INET), AF_UNSPEC);
    EXPECT_EQ(is_netaddr("[2001:db8::4%6]", AF_INET6), AF_INET6);
    EXPECT_EQ(is_netaddr("[::1%2]", AF_INET6), AF_INET6);
    EXPECT_EQ(is_netaddr("[::%2]"), AF_INET6);
}

TEST(NetaddrTestSuite, is_numport) {
    EXPECT_EQ(is_numport(""), -1);
    EXPECT_EQ(is_numport("X"), -1);
    EXPECT_EQ(is_numport("-1"), -1);
    EXPECT_EQ(is_numport("-0"), -1);
    EXPECT_EQ(is_numport("0"), 0);
    EXPECT_EQ(is_numport("65535"), 0);
    EXPECT_EQ(is_numport("65536"), 1);
    EXPECT_EQ(is_numport("6553X"), -1);
    EXPECT_EQ(is_numport("65535Y"), -1);
    EXPECT_EQ(is_numport("http"), -1);
}

} // namespace utest


int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include <utest/utest_main.inc>
    return gtest_return_code; // managed in gtest_main.inc
}
