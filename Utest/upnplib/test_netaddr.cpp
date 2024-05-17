// Copyright (C) 2024+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-05-18

#include <upnplib/global.hpp>
#include <upnplib/netaddr.hpp>
#include <gmock/gmock.h>

namespace utest {

using ::upnplib::CNetaddr;


// CNetaddr TestSuite
// ==================

TEST(NetaddrTestSuite, first_test) {
    CNetaddr netaddr;
    netaddr.set("[2001:db8::1]", "50001");
}

} // namespace utest


int main(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
#include <utest/utest_main.inc>
    return gtest_return_code; // managed in gtest_main.inc
}
