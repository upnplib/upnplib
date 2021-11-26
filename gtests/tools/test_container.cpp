// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-11-27

// Compiled this with:
// cd gtests\build/MinSizeRel
// cl /nologo /EHsc /std:c++17
// /I..\..\../build\_deps\googletest-src\googletest\include
// ..\..\..\build\lib\MinSizeRel\gtest.lib ..\..\tools\test_container.cpp
// ws2_32.lib iphlpapi.lib

#include "gtest/gtest.h"
#include <winsock2.h>
#include <iphlpapi.h>
#include <iostream>
#include <ws2tcpip.h>

namespace upnp {

class CObjSimple {
  public:
    CObjSimple() { m_adapts.IfIndex = 1; }

    ::PIP_ADAPTER_ADDRESSES get() const {
        return (::PIP_ADAPTER_ADDRESSES)&m_adapts;
    }

    void set(int a_ifindex) { m_adapts.IfIndex = a_ifindex; }

    void chain_next_addr(::PIP_ADAPTER_ADDRESSES a_ptrNextAddr) {
        m_adapts.Next = a_ptrNextAddr;
        // std::cout << "DEBUG: m_adapts.IfIndex = " << m_adapts.IfIndex
        //           << ", m_adapts.Next = " << m_adapts.Next << std::endl;
    }

  private:
    ::IP_ADAPTER_ADDRESSES m_adapts{};
};

class CObj {
  public:
    CObj() {
        // set ip4 unicast address structures
        // std::cout << "DEBUG: constructor executed." << std::endl;

        m_inaddr.sin_family = AF_INET;
        m_inaddr.sin_port = 0;
        m_inaddr.sin_addr.s_addr = 16777343; // "127.0.0.1"

        m_saddr.lpSockaddr = (::LPSOCKADDR)&m_inaddr;
        m_saddr.iSockaddrLength = sizeof(::SOCKET_ADDRESS);

        m_uniaddr.Length = sizeof(::IP_ADAPTER_UNICAST_ADDRESS);
        m_uniaddr.Flags = 0;
        m_uniaddr.Address = m_saddr;
        m_uniaddr.PrefixOrigin = (::IP_PREFIX_ORIGIN)0;
        m_uniaddr.SuffixOrigin = (::IP_SUFFIX_ORIGIN)0;
        m_uniaddr.DadState = (::IP_DAD_STATE)0;
        m_uniaddr.ValidLifetime = 0;
        m_uniaddr.PreferredLifetime = 0;
        m_uniaddr.LeaseLifetime = 0;
        m_uniaddr.OnLinkPrefixLength = 8; // subnet bit mask

        m_adapts.Length = sizeof(::IP_ADAPTER_ADDRESSES);
        m_adapts.IfIndex = 1;
        m_adapts.Next = nullptr;
        m_adapts.AdapterName =
            (::PCHAR) "{441447DD-ABA7-11EB-892C-806E6F6E6963}";
        m_adapts.FirstUnicastAddress = &m_uniaddr;
        m_adapts.FirstAnycastAddress = nullptr;
        m_adapts.FirstMulticastAddress = nullptr;
        m_adapts.FirstDnsServerAddress = nullptr;
        m_adapts.DnsSuffix = (::PWCHAR) "";
        m_adapts.Description = (::PWCHAR)m_Description.c_str();
        m_adapts.FriendlyName = (::PWCHAR)m_FriendlyName.c_str();
        // m_adapts.PhysicalAddress is initialized to {0}
        // m_adapts.PhysicalAddressLength is initialized to 0
        m_adapts.Flags = IP_ADAPTER_IPV4_ENABLED;
        m_adapts.Mtu = -1;
        m_adapts.IfType = IF_TYPE_SOFTWARE_LOOPBACK;
        m_adapts.OperStatus = IfOperStatusUp;
        m_adapts.Ipv6IfIndex = 0;
        // Remaining settings are initialized to 0. Look at the structure if
        // some more settings are needed in the future or for special cases.
    }

    ::PIP_ADAPTER_ADDRESSES get() const {
        return (::PIP_ADAPTER_ADDRESSES)&m_adapts;
    }

    void set(std::wstring_view a_ifname, std::string_view a_ifaddress) {
        if (a_ifname == L"") {
            return;
        }

        // Split address and bit mask from ip address string
        std::size_t slashpos = a_ifaddress.find_last_of("/");
        std::string address;
        std::string bitmask;
        if (slashpos != std::string_view::npos) {
            address = a_ifaddress.substr(0, slashpos);
            bitmask = a_ifaddress.substr(slashpos + 1);
        } else {
            address = a_ifaddress;
            bitmask = address.empty() ? bitmask = "0" : bitmask = "32";
        }

        // Convert ip address string to address in network order
        int nipaddr{};
        if (address != "") {
            int rc = ::inet_pton(AF_INET, address.c_str(), &nipaddr);
            if (rc != 1) {
                throw std::invalid_argument(std::string(
                    "Invalid ip address " + (std::string)a_ifaddress));
            }
        }

        // Convert bitmask string to number
        std::size_t pos{};
        int nbitmsk = std::stoi(bitmask, &pos);
        // try {
        //     nbitmsk = std::stoi(bitmask);
        // } catch (std::out_of_range const& e) {
        // std::cerr << "DEBUG: Catched exception - " << e.what() << '\n';
        //     return;
        // } catch (std::invalid_argument const& e) {
        // std::cerr << "DEBUG: Catched exception - " << e.what() << ",
        // bitmask=\"" << bitmask << "\"\n";
        //     return;
        // }
        if (pos < bitmask.size()) {
            throw std::invalid_argument(
                std::string("Not a valid bitmask: " + bitmask));
        }
        if (nbitmsk < 0 || nbitmsk > 32) {
            throw std::invalid_argument(
                std::string("Bitmask not in range 0..32"));
        }

        // No errors so far, modify the interface structure
        m_FriendlyName = a_ifname;
        m_inaddr.sin_addr.s_addr = nipaddr;
        m_uniaddr.OnLinkPrefixLength = nbitmsk;
        m_adapts.Mtu = 1500;
        m_adapts.IfType = IF_TYPE_ETHERNET_CSMACD;
    }

    void chain_next_addr(::PIP_ADAPTER_ADDRESSES a_ptrNextAddr) {
        m_adapts.Next = a_ptrNextAddr;
    }

  private:
    ::sockaddr_in m_inaddr{};
    ::SOCKET_ADDRESS m_saddr{};
    ::IP_ADAPTER_UNICAST_ADDRESS m_uniaddr{};
    ::IP_ADAPTER_ADDRESSES m_adapts{};

    // On the adapter structure we only have pointer to strings so we need to
    // save them here to be sure we do not get dangling pointer.
    std::wstring m_Description{L"Mocked Adapter for Unit testing"};
    std::wstring m_FriendlyName{L"Loopback Pseudo-Interface 1"};
};

class CObjVect {
  public:
    void add(std::wstring_view a_ifname, std::string_view a_ifaddress) {
        CObj obj{};

        if (a_ifaddress != "127.0.0.1/8") {
            obj.set(a_ifname, a_ifaddress);
        }

        m_objvec.push_back(std::move(obj));

        // Because the low level ifaddr address pointer chain changes when
        // adding an interface object (don't know why), the address chain must
        // be rebuild.
        for (int i = 1; i < m_objvec.size(); i++) {
            ::PIP_ADAPTER_ADDRESSES ifaddr4New = m_objvec[i].get();
            m_objvec[i - 1].chain_next_addr(ifaddr4New);
        }
    }

    PIP_ADAPTER_ADDRESSES get_ifaddr(int a_idx) {
        if (a_idx == 0)
            return nullptr;

        PIP_ADAPTER_ADDRESSES adapts = m_objvec.at(a_idx - 1).get();
        return adapts;
    }

  private:
    std::vector<CObj> m_objvec{};
};

//
// Testsuites
// ----------
TEST(ObjectTestSuite, simple_test) {
    CObj obj{};
    ::PIP_ADAPTER_ADDRESSES adapts = obj.get();

    EXPECT_STREQ(adapts->FriendlyName, L"Loopback Pseudo-Interface 1");
    obj.set(L"if1v4", "192.168.10.4/24");
    EXPECT_STREQ(adapts->FriendlyName, L"if1v4");
}

TEST(ContainerTestSuite, object_vector_test) {
    CObjVect objvect{};

    objvect.add(L"if1v4", "127.0.0.1/8");
    ::PIP_ADAPTER_ADDRESSES adapts = objvect.get_ifaddr(1);
    EXPECT_STREQ(adapts->FriendlyName, L"Loopback Pseudo-Interface 1");

    objvect.add(L"if2v4", "192.168.192.168/20");
    adapts = objvect.get_ifaddr(2);
    EXPECT_STREQ(adapts->FriendlyName, L"if2v4");

    objvect.add(L"if3v4", "127.0.0.1/8");
    adapts = objvect.get_ifaddr(3);
    EXPECT_STREQ(adapts->FriendlyName, L"Loopback Pseudo-Interface 1");

    adapts = objvect.get_ifaddr(1);
    EXPECT_STREQ(adapts->FriendlyName, L"Loopback Pseudo-Interface 1");
    EXPECT_STREQ(adapts->Next->FriendlyName, L"if2v4");
    EXPECT_STREQ(adapts->Next->Next->FriendlyName,
                 L"Loopback Pseudo-Interface 1");
#if false
    objvect.add(20);
    adapts = objvect.get_ifaddr(2);
    EXPECT_EQ(adapts->IfIndex, 20);

    objvect.add(30);
    adapts = objvect.get_ifaddr(3);
    EXPECT_EQ(adapts->IfIndex, 30);

    adapts = objvect.get_ifaddr(1);
    EXPECT_EQ(adapts->IfIndex, 10);
    EXPECT_EQ(adapts->Next->IfIndex, 20);
    EXPECT_EQ(adapts->Next->Next->IfIndex, 30);
#endif
}

} // namespace upnp

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
