// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-03-30

#include "upnplib_gtest_tools.hpp"
#include "upnplib_gtest_tools_unix.hpp"
#include "gtest/gtest.h"

#include <net/if.h>
#include <arpa/inet.h>

namespace upnplib {

TEST(ToolsTestSuite, initialize_interface_addresses) {
    struct ifaddrs* ifaddr{};
    struct sockaddr_in* ifa_addr_in{};
    struct sockaddr_in* ifa_netmask_in{};
    struct sockaddr_in* ifa_ifu_in{};
    char addr4buf[INET_ADDRSTRLEN]{};

    CIfaddr4 ifaddr4Obj;
    ifaddr = ifaddr4Obj.get();
    ifa_addr_in = (sockaddr_in*)ifaddr->ifa_addr;
    ifa_netmask_in = (sockaddr_in*)ifaddr->ifa_netmask;
    ifa_ifu_in = (sockaddr_in*)ifaddr->ifa_broadaddr;

    // should be constructed with a loopback interface
    EXPECT_EQ(ifaddr->ifa_next, nullptr);
    EXPECT_STREQ(ifaddr->ifa_name, "lo");
    EXPECT_EQ(ifaddr->ifa_flags, (unsigned int)0 | IFF_LOOPBACK | IFF_UP);
    EXPECT_EQ(ifa_addr_in->sin_family, AF_INET);
    EXPECT_EQ(ifa_addr_in->sin_addr.s_addr, (unsigned int)16777343);
    EXPECT_EQ(ifa_netmask_in->sin_family, AF_INET);
    EXPECT_EQ(ifa_netmask_in->sin_addr.s_addr, (unsigned int)255);
    EXPECT_EQ(ifa_ifu_in->sin_family, AF_INET);
    EXPECT_EQ(ifa_ifu_in->sin_addr.s_addr, (unsigned int)0);
    EXPECT_EQ(ifaddr->ifa_data, nullptr);

    // This throws a segfault by C++ and does not need to be tested
    // EXPECT_ANY_THROW(ifaddr4Obj.set(NULL, "192.168.168.3/24"));
    // EXPECT_ANY_THROW(ifaddr4Obj.set("if0v4", NULL));
    EXPECT_FALSE(ifaddr4Obj.set("", "192.168.168.3/24"));
    EXPECT_FALSE(ifaddr4Obj.set("if0v4", ""));

    EXPECT_TRUE(ifaddr4Obj.set("if0v4", "192.168.168.168/20"));
    EXPECT_STREQ(ifaddr->ifa_name, "if0v4");
    inet_ntop(AF_INET, &ifa_addr_in->sin_addr.s_addr, addr4buf,
              INET_ADDRSTRLEN);
    EXPECT_STREQ(addr4buf, "192.168.168.168")
        << "    addr4buf contains the ip address";
    inet_ntop(AF_INET, &ifa_netmask_in->sin_addr.s_addr, addr4buf,
              INET_ADDRSTRLEN);
    EXPECT_STREQ(addr4buf, "255.255.240.0")
        << "    addr4buf contains the netmask";
    EXPECT_EQ(ifaddr->ifa_flags,
              (unsigned int)0 | IFF_UP | IFF_BROADCAST | IFF_MULTICAST);
    inet_ntop(AF_INET, &ifa_ifu_in->sin_addr.s_addr, addr4buf, INET_ADDRSTRLEN);
    EXPECT_STREQ(addr4buf, "192.168.175.255")
        << "    addr4buf contains the broadcast address";

    EXPECT_TRUE(ifaddr4Obj.set("if1v4", "10.168.168.200"));
    inet_ntop(AF_INET, &ifa_addr_in->sin_addr.s_addr, addr4buf,
              INET_ADDRSTRLEN);
    EXPECT_STREQ(addr4buf, "10.168.168.200")
        << "    addr4buf contains the ip address";
    inet_ntop(AF_INET, &ifa_netmask_in->sin_addr.s_addr, addr4buf,
              INET_ADDRSTRLEN);
    EXPECT_STREQ(addr4buf, "255.255.255.255")
        << "    addr4buf contains the netmask";
    EXPECT_EQ(ifaddr->ifa_flags,
              (unsigned int)0 | IFF_UP | IFF_BROADCAST | IFF_MULTICAST);
    inet_ntop(AF_INET, &ifa_ifu_in->sin_addr.s_addr, addr4buf, INET_ADDRSTRLEN);
    EXPECT_STREQ(addr4buf, "10.168.168.200")
        << "    addr4buf contains the broadcast address";

    EXPECT_ANY_THROW(ifaddr4Obj.set("if2v4", "10.168.168.47/"));
}

TEST(ToolsTestSuite, capture_output_with_pipe) {
    CCaptureStdOutErr captOutObj(STDOUT_FILENO);
    CCaptureStdOutErr captErrObj(STDERR_FILENO);

    captErrObj.start();
    captOutObj.start();
    std::cerr << "err: First output ";
    std::cout << "out: First output ";
    std::cerr << "to StdErr" << std::endl;
    std::cout << "to StdOut" << std::endl;
    std::string captured_err = captErrObj.get();
    std::string captured_out = captOutObj.get();

    EXPECT_STREQ(captured_err.c_str(), "err: First output to StdErr\n");
    EXPECT_STREQ(captured_out.c_str(), "out: First output to StdOut\n");
    // std::cout << "First capture " << captured_err;
    // std::cout << "First capture " << captured_out;

    captErrObj.start();
    captOutObj.start();
    std::cerr << "err: Second output to StdErr" << std::endl;
    std::cout << "out: Second output to StdOut" << std::endl;
    captured_err = captErrObj.get();
    captured_out = captOutObj.get();

    EXPECT_STREQ(captured_err.c_str(), "err: Second output to StdErr\n");
    EXPECT_STREQ(captured_out.c_str(), "out: Second output to StdOut\n");
    // std::cout << "Second start " << captured_err;
    // std::cout << "Second start " << captured_out;
}

TEST(ToolsTestSuite, throw_exception) {
    EXPECT_THROW(CCaptureStdOutErr capttureObj(-1), ::std::invalid_argument);
}

} // namespace upnplib

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
