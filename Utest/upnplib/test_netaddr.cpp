// Copyright (C) 2024+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-05-23

#include <upnplib/netaddr.hpp>

#include <upnplib/global.hpp>
#include <iostream>
#include <utest/utest.hpp>

namespace utest {

using ::testing::EndsWith;

using ::upnplib::Netaddr;


// Netaddr TestSuite
// =================

TEST(NetaddrTestSuite, netaddr_successful) {
    Netaddr napObj;

    napObj = "[2001:db8::1]:61234";
    CaptureStdOutErr captureObj(STDOUT_FILENO); // or STDERR_FILENO
    captureObj.start();

    // Test Unit output stream
    std::cout << napObj << "\n";
    EXPECT_THAT(captureObj.str(), EndsWith("[2001:db8::1]:61234\n"));

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


class NetaddrAssignTest
    : public ::testing::TestWithParam<
          std::tuple<const std::string, const std::string>> {};

TEST_P(NetaddrAssignTest, netaddress_assign) {
    // Get parameter
    std::tuple params = GetParam();

    // Test Unit
    Netaddr napObj;
    napObj = std::get<0>(params);
    EXPECT_EQ(napObj.str(), std::get<1>(params));
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(
    NetaddrAssign, NetaddrAssignTest,
    ::testing::Values(
        std::make_tuple("", ""),
        std::make_tuple("[", ""),
        std::make_tuple("]", ""),
        std::make_tuple("[]", ""),
        std::make_tuple(":", ""),
        std::make_tuple(":0", ""),
        std::make_tuple(":50987", ":50987"),
        std::make_tuple(".", ""),
        std::make_tuple(".:", ""),
        std::make_tuple(":.", ""),
        std::make_tuple("::", ""),
        std::make_tuple(":::", ""),
        std::make_tuple("[::", ""),
        std::make_tuple("::]", ""),
        std::make_tuple("[::]", "[::]:0"),
        std::make_tuple("[::]:", "[::]:0"),
        std::make_tuple("[::]:0", "[::]:0"),
        std::make_tuple("[::1", ""),
        std::make_tuple("::1]", ""),
        std::make_tuple("[::1]", "[::1]:0"),
        std::make_tuple("[::1]:0", "[::1]:0"),
        std::make_tuple("[::1].4", ""), // may be any alphanum
        std::make_tuple("0.0.0.0", "0.0.0.0:0"),
        std::make_tuple("0.0.0.0:", "0.0.0.0:0"),
        std::make_tuple("0.0.0.0:0", "0.0.0.0:0"),
        std::make_tuple("127.0.0.1", "127.0.0.1:0"),
        std::make_tuple("127.0.0.1:", "127.0.0.1:0"),
        std::make_tuple("127.0.0.1:0", "127.0.0.1:0"),
        std::make_tuple("127.0.0.1.4", ""), // dot for colon
        std::make_tuple("[2001:db8::43]:", "[2001:db8::43]:0"),
        std::make_tuple("2001:db8::41:59897", ":59897"),
        std::make_tuple("[2001:db8::fg]", ""),
        std::make_tuple("[2001:db8::fg]:59877", ":59877"),
        std::make_tuple("[2001:db8::42]:65535", "[2001:db8::42]:65535"),
        std::make_tuple("[2001:db8::51]:65536", "[2001:db8::51]:0"),
        std::make_tuple("[2001:db8::52::53]", ""), // double double colon
        std::make_tuple("[2001:db8::52::53]:65535", ":65535"), // double double
        std::make_tuple("192.168.88.99:59876", "192.168.88.99:59876"),
        std::make_tuple("192.168.88.99:65537", "192.168.88.99:0"),
        std::make_tuple("192.168.88.256:59866", ":59866"),
        std::make_tuple("192.168.88.91", "192.168.88.91:0"),
        std::make_tuple("garbage", ""),
        std::make_tuple("[garbage]:49494", ":49494")
    ));
// clang-forat on

} // namespace utest


int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include <utest/utest_main.inc>
    return gtest_return_code; // managed in gtest_main.inc
}
