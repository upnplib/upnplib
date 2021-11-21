// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-11-20

#include "custom_gtest_tools_win.hpp"
#include <ws2tcpip.h>
#include "gmock/gmock.h"

using testing::ElementsAre;

namespace upnp {

TEST(ToolsTestSuite, show_real_loopback_interface) {
    // For the structure look at
    // https://docs.microsoft.com/en-us/windows/win32/api/iptypes/ns-iptypes-ip_adapter_addresses_lh

    ::PIP_ADAPTER_ADDRESSES adapts{};
    ULONG adapts_sz{};

#if false // Does not return correct buffer size with MS Windows Server 2019 on
          // Github Actions CI Tests. Don't know why. So we will use a fixed
          // buffer size here.
    // Get Adapters addresses required size. Check with nullptr to adapts
    // structure will fail with ERROR_BUFFER_OVERFLOW but return required size.
    EXPECT_EQ(::GetAdaptersAddresses(
                  AF_INET, GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_DNS_SERVER,
                  NULL, adapts, &adapts_sz),
              ERROR_BUFFER_OVERFLOW);
#else
    adapts_sz = 16383;
#endif
    // Allocate needed memory.
    adapts = (::PIP_ADAPTER_ADDRESSES)::malloc(adapts_sz);
    ASSERT_NE(adapts, nullptr);
    ::memset(adapts, 0, adapts_sz);
    // Do the call that will actually return the info.
    ULONG rc = ::GetAdaptersAddresses(
        AF_UNSPEC, GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_DNS_SERVER, NULL,
        adapts, &adapts_sz);
    std::cout << "adapts_sz             : " << adapts_sz << '\n';
    EXPECT_EQ(rc, ERROR_SUCCESS);

    // Look for the loopback interface with IfIndex 1
    for (::PIP_ADAPTER_ADDRESSES adapts_item = adapts; adapts_item != nullptr;
         adapts_item = adapts_item->Next) {
        if (adapts_item->IfIndex == 1) {
            // std::wcout << "---------------------------------\n";
            std::wcout << "IfIndex               : " << adapts_item->IfIndex
                       << "\n";
            std::wcout << "FriendlyName          : "
                       << adapts_item->FriendlyName << '\n';
            std::wcout << "Description           : " << adapts_item->Description
                       << "\n";
            std::wcout << "AdapterName           : " << adapts_item->AdapterName
                       << "\n";
            printf("PhysicalAddress       : %02X-%02X-%02X-%02X-%02X-%02X\n",
                   adapts_item->PhysicalAddress[0],
                   adapts_item->PhysicalAddress[1],
                   adapts_item->PhysicalAddress[2],
                   adapts_item->PhysicalAddress[3],
                   adapts_item->PhysicalAddress[4],
                   adapts_item->PhysicalAddress[5]);
            std::wcout << "PhysicalAddressLength : "
                       << adapts_item->PhysicalAddressLength << '\n';
            EXPECT_EQ(adapts_item->PhysicalAddressLength, 0);
            std::wcout << "Mtu                   : " << adapts_item->Mtu
                       << " (signed -1)\n";
            std::wcout << "IfType                : " << adapts_item->IfType
                       << "\n";
            std::wcout << "OperStatus            : " << adapts_item->OperStatus
                       << "\n";

            PIP_ADAPTER_UNICAST_ADDRESS uni_addr =
                adapts_item->FirstUnicastAddress;
            while (uni_addr) {
                SOCKADDR* ip_addr = uni_addr->Address.lpSockaddr;
                // ip_addr->sa_family = 1;
                std::string af_str = std::to_string(ip_addr->sa_family);
                switch (ip_addr->sa_family) {
                case 2:
                    af_str = "AF_INET";
                    in_addr sin_addr = ((::sockaddr_in*)ip_addr)->sin_addr;
                    std::cout << "sin_family = " << af_str
                              << ",  ip address = " << ::inet_ntoa(sin_addr)
                              << "/";
                    std::wcout << uni_addr->OnLinkPrefixLength << '\n';
                    // std::cout << sin_addr.s_addr << "\n";
                    break;
                case 23:
                    af_str = "AF_INET6";
                    char ipstr[INET6_ADDRSTRLEN];
                    in6_addr sin6_addr = ((::sockaddr_in6*)ip_addr)->sin6_addr;
                    EXPECT_NE(
                        ::inet_ntop(AF_INET6, &sin6_addr, ipstr, sizeof(ipstr)),
                        nullptr);
                    std::cout << "sin_family = " << af_str
                              << ", ip address = " << ipstr << "/";
                    std::wcout << uni_addr->OnLinkPrefixLength << '\n';
                    break;
                case 0:
                    af_str = "AF_UNSPEC(0)";
                    // no break, continue with default
                default:
                    GTEST_FAIL() << "ip_addr->sin_family = " << af_str
                                 << "\n   ERROR     Undefined sin_family. Only "
                                    "AF_INET(2) or AF_INET6(23) valid.";
                    break;
                }

                // Next address on the interface.
                uni_addr = uni_addr->Next;
            }

            // Exit loop looking for interfaces, we will not find more
            // interfaces with the same IfIndex.
            break;
        }
    }
    ::free(adapts);
}

TEST(ToolsTestSuite, get_fake_loopback_interface) {
    CIfaddr4 ifaddr4Obj;
    ::IP_ADAPTER_ADDRESSES const* adapts = ifaddr4Obj.get();

    EXPECT_NE(adapts, nullptr);
    EXPECT_EQ(adapts->IfIndex, 1);
    EXPECT_EQ(adapts->Next, nullptr);
    EXPECT_STREQ(adapts->AdapterName, "{441447DD-ABA7-11EB-892C-806E6F6E6963}");
    // Check subnet bit mask
    EXPECT_EQ(adapts->FirstUnicastAddress->OnLinkPrefixLength, 8);
    // Check ip address
    ::sockaddr_in* ip_addr =
        (::sockaddr_in*)adapts->FirstUnicastAddress->Address.lpSockaddr;
    EXPECT_EQ(ip_addr->sin_family, AF_INET);
    EXPECT_EQ(ip_addr->sin_port, 0);
    EXPECT_STREQ(::inet_ntoa(ip_addr->sin_addr), "127.0.0.1");
    // There should not be other first addresses
    EXPECT_EQ(adapts->FirstAnycastAddress, nullptr);
    EXPECT_EQ(adapts->FirstMulticastAddress, nullptr);
    EXPECT_EQ(adapts->FirstDnsServerAddress, nullptr);
    EXPECT_STREQ(adapts->DnsSuffix, L"");

    EXPECT_STREQ(adapts->Description, L"Mocked Adapter for Unit testing");
    EXPECT_STREQ(adapts->FriendlyName, L"Loopback Pseudo-Interface 1");
    EXPECT_THAT(adapts->PhysicalAddress, ElementsAre(0, 0, 0, 0, 0, 0, 0, 0));
    EXPECT_EQ(adapts->PhysicalAddressLength, 0);
    EXPECT_EQ(adapts->Flags, IP_ADAPTER_IPV4_ENABLED);
    EXPECT_EQ(adapts->Mtu, 4294967295); // That is signed -1
    EXPECT_EQ(adapts->IfType, IF_TYPE_SOFTWARE_LOOPBACK);
    EXPECT_EQ(adapts->OperStatus, ::IfOperStatusUp);
    // Only AF_INET (IPv4) available here
    EXPECT_EQ(adapts->Ipv6IfIndex, 0);
}

TEST(ToolsTestSuite, set_regular_fake_network_interface) {
    CIfaddr4 ifaddr4Obj;
    ::IP_ADAPTER_ADDRESSES const* adapts = ifaddr4Obj.get();

    // Set interface with ip address and bit mask
    ifaddr4Obj.set(L"Ethernet 1", "192.168.24.28/24");
    // Check settings
    EXPECT_STREQ(adapts->FriendlyName, L"Ethernet 1");
    // Check ip address
    ::sockaddr_in const* ip_addr =
        (::sockaddr_in*)adapts->FirstUnicastAddress->Address.lpSockaddr;
    EXPECT_EQ(ip_addr->sin_family, AF_INET);
    EXPECT_EQ(ip_addr->sin_port, 0);
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ip_addr->sin_addr, ip_str, INET_ADDRSTRLEN);
    EXPECT_STREQ(ip_str, "192.168.24.28");
    // Check subnet bit mask
    EXPECT_EQ(adapts->FirstUnicastAddress->OnLinkPrefixLength, 24);
    // Check other settings
    EXPECT_EQ(adapts->Mtu, 1500);
    EXPECT_EQ(adapts->IfType, IF_TYPE_ETHERNET_CSMACD);

    // Set interface with ip address but no bit mask
    ifaddr4Obj.set(L"Ethernet 2", "192.168.26.30");
    EXPECT_STREQ(adapts->FriendlyName, L"Ethernet 2");
    inet_ntop(AF_INET, &ip_addr->sin_addr, ip_str, INET_ADDRSTRLEN);
    EXPECT_STREQ(ip_str, "192.168.26.30");
    // Check subnet bit mask
    EXPECT_EQ(adapts->FirstUnicastAddress->OnLinkPrefixLength, 32);
    // Check other settings
    EXPECT_EQ(adapts->Mtu, 1500);
    EXPECT_EQ(adapts->IfType, IF_TYPE_ETHERNET_CSMACD);
}

TEST(ToolsTestSuite, set_fake_network_interface_with_failures) {
    CIfaddr4 ifaddr4Obj;
    ::IP_ADAPTER_ADDRESSES const* adapts = ifaddr4Obj.get();

    // Check ip address, no changes with empty interface name
    ifaddr4Obj.set(L"", "");
    ::sockaddr_in* ip_addr =
        (::sockaddr_in*)adapts->FirstUnicastAddress->Address.lpSockaddr;
    EXPECT_EQ(ip_addr->sin_family, AF_INET);
    EXPECT_EQ(ip_addr->sin_port, 0);
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ip_addr->sin_addr, ip_str, INET_ADDRSTRLEN);
    EXPECT_STREQ(ip_str, "127.0.0.1");
    EXPECT_EQ(adapts->FirstUnicastAddress->OnLinkPrefixLength, 8);
    EXPECT_EQ(adapts->Mtu, -1);
    EXPECT_EQ(adapts->IfType, IF_TYPE_SOFTWARE_LOOPBACK);

    // No changes with empty interface name
    ifaddr4Obj.set(L"", "192.168.24.77");
    EXPECT_EQ(ip_addr->sin_family, AF_INET);
    EXPECT_EQ(ip_addr->sin_port, 0);
    inet_ntop(AF_INET, &ip_addr->sin_addr, ip_str, INET_ADDRSTRLEN);
    EXPECT_STREQ(ip_str, "127.0.0.1");
    EXPECT_EQ(adapts->FirstUnicastAddress->OnLinkPrefixLength, 8);
    EXPECT_EQ(adapts->Mtu, -1);
    EXPECT_EQ(adapts->IfType, IF_TYPE_SOFTWARE_LOOPBACK);

    // Interface but with zero ip address
    ifaddr4Obj.set(L"Ethernet 1", "");
    EXPECT_STREQ(adapts->FriendlyName, L"Ethernet 1");
    EXPECT_EQ(ip_addr->sin_family, AF_INET);
    EXPECT_EQ(ip_addr->sin_port, 0);
    EXPECT_EQ(ip_addr->sin_addr.s_addr, 0);
    EXPECT_EQ(adapts->FirstUnicastAddress->OnLinkPrefixLength, 0);
    EXPECT_EQ(adapts->Mtu, 1500);
    EXPECT_EQ(adapts->IfType, IF_TYPE_ETHERNET_CSMACD);

    // Interface but with wrong ip address should do nothing
    ifaddr4Obj.set(L"Ethernet wrong IP", "192.168.24.129/");
    EXPECT_STREQ(adapts->FriendlyName, L"Ethernet 1");
    EXPECT_EQ(ip_addr->sin_family, AF_INET);
    EXPECT_EQ(ip_addr->sin_port, 0);
    EXPECT_EQ(ip_addr->sin_addr.s_addr, 0);
    EXPECT_EQ(adapts->FirstUnicastAddress->OnLinkPrefixLength, 0);
    EXPECT_EQ(adapts->Mtu, 1500);
    EXPECT_EQ(adapts->IfType, IF_TYPE_ETHERNET_CSMACD);

    ifaddr4Obj.set(L"Ethernet wrong IP", "192.168.129.129:24");
    EXPECT_STREQ(adapts->FriendlyName, L"Ethernet 1");
    EXPECT_EQ(ip_addr->sin_family, AF_INET);
    EXPECT_EQ(ip_addr->sin_port, 0);
    EXPECT_EQ(ip_addr->sin_addr.s_addr, 0);
    EXPECT_EQ(adapts->FirstUnicastAddress->OnLinkPrefixLength, 0);
    EXPECT_EQ(adapts->Mtu, 1500);
    EXPECT_EQ(adapts->IfType, IF_TYPE_ETHERNET_CSMACD);

    ifaddr4Obj.set(L"Ethernet wrong IP", "/");
    EXPECT_STREQ(adapts->FriendlyName, L"Ethernet 1");
    EXPECT_EQ(ip_addr->sin_family, AF_INET);
    EXPECT_EQ(ip_addr->sin_port, 0);
    EXPECT_EQ(ip_addr->sin_addr.s_addr, 0);
    EXPECT_EQ(adapts->FirstUnicastAddress->OnLinkPrefixLength, 0);
    EXPECT_EQ(adapts->Mtu, 1500);
    EXPECT_EQ(adapts->IfType, IF_TYPE_ETHERNET_CSMACD);

    ifaddr4Obj.set(L"Ethernet wrong IP", "192.168.168.168/-1");
    EXPECT_STREQ(adapts->FriendlyName, L"Ethernet 1");
    EXPECT_EQ(ip_addr->sin_family, AF_INET);
    EXPECT_EQ(ip_addr->sin_port, 0);
    EXPECT_EQ(ip_addr->sin_addr.s_addr, 0);
    EXPECT_EQ(adapts->FirstUnicastAddress->OnLinkPrefixLength, 0);
    EXPECT_EQ(adapts->Mtu, 1500);
    EXPECT_EQ(adapts->IfType, IF_TYPE_ETHERNET_CSMACD);

    ifaddr4Obj.set(L"Ethernet wrong IP", "192.168.168.168/33");
    EXPECT_STREQ(adapts->FriendlyName, L"Ethernet 1");
    EXPECT_EQ(ip_addr->sin_family, AF_INET);
    EXPECT_EQ(ip_addr->sin_port, 0);
    EXPECT_EQ(ip_addr->sin_addr.s_addr, 0);
    EXPECT_EQ(adapts->FirstUnicastAddress->OnLinkPrefixLength, 0);
    EXPECT_EQ(adapts->Mtu, 1500);
    EXPECT_EQ(adapts->IfType, IF_TYPE_ETHERNET_CSMACD);
}

} // namespace upnp

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
