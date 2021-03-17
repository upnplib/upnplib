// Testing the tools, of course ;-)
// Author: 2021-03-12 - Ingo Höft <Ingo@Hoeft-online.de>

#include "gtest/gtest.h"
#include "ifaddrs.cpp"

TEST(ToolsTestSuite, initializing_interface_addresses)
{
    struct ifaddrs* ifaddr = nullptr;
    struct sockaddr_in* ifa_addr_in = nullptr;
    struct sockaddr_in* ifa_netmask_in = nullptr;
    struct sockaddr_in* ifa_ifu_in = nullptr;

    Ifaddr4 ifaddr4;
    ifaddr = ifaddr4.get();

    EXPECT_EQ(ifaddr->ifa_next, nullptr);
    EXPECT_STREQ(ifaddr->ifa_name, "lo");
    EXPECT_EQ(ifaddr->ifa_flags, (const unsigned int)0 | IFF_LOOPBACK | IFF_UP);
    ifa_addr_in = (sockaddr_in*)ifaddr->ifa_addr;
    EXPECT_EQ(ifa_addr_in->sin_family, AF_INET);
    EXPECT_EQ(ifa_addr_in->sin_addr.s_addr, (const unsigned int)16777343);
    ifa_netmask_in = (sockaddr_in*)ifaddr->ifa_netmask;
    EXPECT_EQ(ifa_netmask_in->sin_family, AF_INET);
    EXPECT_EQ(ifa_netmask_in->sin_addr.s_addr, (const unsigned int)255);
    ifa_ifu_in = (sockaddr_in*)ifaddr->ifa_broadaddr;
    EXPECT_EQ(ifa_ifu_in->sin_family, AF_INET);
    EXPECT_EQ(ifa_ifu_in->sin_addr.s_addr, (const unsigned int)0);
    EXPECT_EQ(ifaddr->ifa_data, nullptr);

    EXPECT_TRUE(ifaddr4.set("if0v4", "192.168.168.168/20"));
    EXPECT_STREQ(ifaddr->ifa_name, "if0v4");
    EXPECT_EQ(ifa_addr_in->sin_addr.s_addr, (const unsigned int)2829625536);
    EXPECT_EQ(ifa_netmask_in->sin_addr.s_addr, (const unsigned int)15794175);
    EXPECT_EQ(ifaddr->ifa_flags, (const unsigned int)0 | IFF_UP | IFF_BROADCAST | IFF_MULTICAST);
    char broadcast_address[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ifa_ifu_in->sin_addr.s_addr, broadcast_address, INET_ADDRSTRLEN);
    EXPECT_STREQ(broadcast_address, "192.168.175.255");

    EXPECT_TRUE(ifaddr4.set("if1v4", "10.168.168.200"));
    EXPECT_EQ(ifa_addr_in->sin_addr.s_addr, (const unsigned int)3366496266);
    EXPECT_EQ(ifa_netmask_in->sin_addr.s_addr, (const unsigned int)4294967295);
    EXPECT_EQ(ifaddr->ifa_flags, (const unsigned int)0 | IFF_UP | IFF_BROADCAST | IFF_MULTICAST);
    inet_ntop(AF_INET, &ifa_ifu_in->sin_addr.s_addr, broadcast_address, INET_ADDRSTRLEN);
    EXPECT_STREQ(broadcast_address, "10.168.168.200");

    //EXPECT_TRUE(ifaddr4.set("if0v4", "10.168.168.200/"));
    //EXPECT_ANY_THROW(std::__cxx11::stoi(""));
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
