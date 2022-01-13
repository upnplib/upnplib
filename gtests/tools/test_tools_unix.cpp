// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-12-06

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
    EXPECT_EQ(ifaddr->ifa_flags, (const unsigned int)0 | IFF_LOOPBACK | IFF_UP);
    EXPECT_EQ(ifa_addr_in->sin_family, AF_INET);
    EXPECT_EQ(ifa_addr_in->sin_addr.s_addr, (const unsigned int)16777343);
    EXPECT_EQ(ifa_netmask_in->sin_family, AF_INET);
    EXPECT_EQ(ifa_netmask_in->sin_addr.s_addr, (const unsigned int)255);
    EXPECT_EQ(ifa_ifu_in->sin_family, AF_INET);
    EXPECT_EQ(ifa_ifu_in->sin_addr.s_addr, (const unsigned int)0);
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
              (const unsigned int)0 | IFF_UP | IFF_BROADCAST | IFF_MULTICAST);
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
              (const unsigned int)0 | IFF_UP | IFF_BROADCAST | IFF_MULTICAST);
    inet_ntop(AF_INET, &ifa_ifu_in->sin_addr.s_addr, addr4buf, INET_ADDRSTRLEN);
    EXPECT_STREQ(addr4buf, "10.168.168.200")
        << "    addr4buf contains the broadcast address";

    EXPECT_ANY_THROW(ifaddr4Obj.set("if2v4", "10.168.168.47/"));
}

TEST(ToolsTestSuite, initialize_interface_address_container) {
    CIfaddr4Container ifaddr4Container;

    struct ifaddrs* ifaddr = nullptr;
    struct sockaddr_in* ifa_addr_in = nullptr;
    struct sockaddr_in* ifa_netmask_in = nullptr;
    struct sockaddr_in* ifa_ifu_in = nullptr;
    char addr4buf[INET_ADDRSTRLEN] = {};

    ifaddr = ifaddr4Container.get_ifaddr(0);
    EXPECT_TRUE(ifaddr == nullptr);

    EXPECT_TRUE(ifaddr4Container.add("if1v4", "192.168.0.168/20"));
    ifaddr = ifaddr4Container.get_ifaddr(1);
    ifa_addr_in = (sockaddr_in*)ifaddr->ifa_addr;
    ifa_netmask_in = (sockaddr_in*)ifaddr->ifa_netmask;
    ifa_ifu_in = (sockaddr_in*)ifaddr->ifa_broadaddr;

    EXPECT_STREQ(ifaddr->ifa_name, "if1v4");
    inet_ntop(AF_INET, &ifa_addr_in->sin_addr.s_addr, addr4buf,
              INET_ADDRSTRLEN);
    EXPECT_STREQ(addr4buf, "192.168.0.168")
        << "    addr4buf contains the ip address";
    inet_ntop(AF_INET, &ifa_netmask_in->sin_addr.s_addr, addr4buf,
              INET_ADDRSTRLEN);
    EXPECT_STREQ(addr4buf, "255.255.240.0")
        << "    addr4buf contains the netmask";
    inet_ntop(AF_INET, &ifa_ifu_in->sin_addr.s_addr, addr4buf, INET_ADDRSTRLEN);
    EXPECT_STREQ(addr4buf, "192.168.15.255")
        << "    addr4buf contains the broadcast address";

    // This throws a segfault by C++ and does not need to be tested
    // EXPECT_ANY_THROW(ifaddr4Container.add(NULL, "10.0.0.1/8"));
    // EXPECT_ANY_THROW(ifaddr4Container.add("if3v4", NULL));
    EXPECT_ANY_THROW(ifaddr4Container.get_ifaddr(-1));
    EXPECT_ANY_THROW(ifaddr4Container.get_ifaddr(2));
    EXPECT_FALSE(ifaddr4Container.add("", "10.0.0.1/8"));
    EXPECT_FALSE(ifaddr4Container.add("if3v4", ""));
}

TEST(ToolsTestSuite, chain_ifaddr_in_interface_address_container) {
    CIfaddr4Container ifaddr4Container;

    struct ifaddrs* ifaddr1 = nullptr;
    struct ifaddrs* ifaddr2 = nullptr;
    struct ifaddrs* ifaddr3 = nullptr;
    struct sockaddr_in* ifa_addr_in = nullptr;
    struct sockaddr_in* ifa_netmask_in = nullptr;
    struct sockaddr_in* ifa_ifu_in = nullptr;
    char addr4buf[INET_ADDRSTRLEN] = {};

    EXPECT_FALSE(ifaddr4Container.add("lo", "127.0.0.2/8"));
    EXPECT_TRUE(ifaddr4Container.add("lo", "127.0.0.1/8"));
    ifaddr1 = ifaddr4Container.get_ifaddr(1);
    ifa_addr_in = (sockaddr_in*)ifaddr1->ifa_addr;

    EXPECT_STREQ(ifaddr1->ifa_name, "lo");
    inet_ntop(AF_INET, &ifa_addr_in->sin_addr.s_addr, addr4buf,
              INET_ADDRSTRLEN);
    EXPECT_STREQ(addr4buf, "127.0.0.1")
        << "    addr4buf contains the ip address";

    EXPECT_TRUE(ifaddr4Container.add("lo", "127.0.0.1/8"));
    ifaddr2 = ifaddr4Container.get_ifaddr(2);
    EXPECT_STREQ(ifaddr2->ifa_name, "lo");

    EXPECT_TRUE(ifaddr4Container.add("if3v4", "192.168.1.2/24"));

    // Here we get the chain.
    // It is important to get the current pointer because the chain changed
    // with every addition of an interface address.
    ifaddr1 = ifaddr4Container.get_ifaddr(1);
    ifaddr2 = ifaddr4Container.get_ifaddr(2);
    ifaddr3 = ifaddr4Container.get_ifaddr(3);
    EXPECT_EQ(ifaddr1->ifa_next, ifaddr2);
    EXPECT_EQ(ifaddr1->ifa_next->ifa_next, ifaddr3);
    EXPECT_EQ(ifaddr1->ifa_next->ifa_next->ifa_next, nullptr);

    ifa_addr_in = (sockaddr_in*)ifaddr3->ifa_addr;
    ifa_netmask_in = (sockaddr_in*)ifaddr3->ifa_netmask;
    ifa_ifu_in = (sockaddr_in*)ifaddr3->ifa_broadaddr;
    EXPECT_STREQ(ifaddr3->ifa_name, "if3v4");
    inet_ntop(AF_INET, &ifa_addr_in->sin_addr.s_addr, addr4buf,
              INET_ADDRSTRLEN);
    EXPECT_STREQ(addr4buf, "192.168.1.2")
        << "    addr4buf contains the ip address";
    inet_ntop(AF_INET, &ifa_netmask_in->sin_addr.s_addr, addr4buf,
              INET_ADDRSTRLEN);
    EXPECT_STREQ(addr4buf, "255.255.255.0")
        << "    addr4buf contains the netmask";
    inet_ntop(AF_INET, &ifa_ifu_in->sin_addr.s_addr, addr4buf, INET_ADDRSTRLEN);
    EXPECT_STREQ(addr4buf, "192.168.1.255")
        << "    addr4buf contains the broadcast address";
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
