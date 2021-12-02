// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-11-30

// Implementation of the NetIf classes
// ===================================

#include "custom_gtest_tools_win.hpp"
#include <ws2tcpip.h>
#include <string>

namespace upnp {

CNetIf4::CNetIf4() {
    // set ip4 unicast address structures

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
    m_adapts.AdapterName = (::PCHAR) "{441447DD-ABA7-11EB-892C-806E6F6E6963}";
    m_adapts.FirstUnicastAddress = &m_uniaddr;
    m_adapts.FirstAnycastAddress = nullptr;
    m_adapts.FirstMulticastAddress = nullptr;
    m_adapts.FirstDnsServerAddress = nullptr;
    m_adapts.DnsSuffix = (::PWCHAR)L"";
    m_adapts.Description = (::PWCHAR)m_Description.c_str();
    m_adapts.FriendlyName = (::PWCHAR)m_FriendlyName.c_str();
    // m_adapts.PhysicalAddress is initialized to {0}
    // m_adapts.PhysicalAddressLength is initialized to 0
    m_adapts.Flags = IP_ADAPTER_IPV4_ENABLED;
    m_adapts.Mtu = -1;
    m_adapts.IfType = IF_TYPE_SOFTWARE_LOOPBACK;
    m_adapts.OperStatus = IfOperStatusUp;
    m_adapts.Ipv6IfIndex = 0;
    // Remaining settings are initialized to 0. Look at the structure if some
    // more settings are needed in the future or for special cases.
}

::PIP_ADAPTER_ADDRESSES CNetIf4::get() const {
    return (::PIP_ADAPTER_ADDRESSES)&m_adapts;
}

void CNetIf4::set(std::wstring_view a_ifname, std::string_view a_ifaddress) {
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

    // Convert ip address string to address in network byte order
    int nipaddr{};
    if (address != "") {
        int rc = ::inet_pton(AF_INET, address.c_str(), &nipaddr);
        if (rc != 1) {
            throw std::invalid_argument(
                std::string("Invalid ip address " + (std::string)a_ifaddress));
        }
    }

    // Convert bitmask string to number
    std::size_t pos{};
    int nbitmsk = std::stoi(bitmask, &pos);
    if (pos < bitmask.size()) {
        throw std::invalid_argument(
            std::string("Not a valid bitmask: " + bitmask));
    }
    if (nbitmsk < 0 || nbitmsk > 32) {
        throw std::invalid_argument(std::string("Bitmask not in range 0..32"));
    }

    // No errors so far, modify the interface structure
    m_FriendlyName = a_ifname;
    m_inaddr.sin_addr.s_addr = nipaddr;
    m_uniaddr.OnLinkPrefixLength = nbitmsk;
    m_adapts.Mtu = 1500;
    m_adapts.IfType = IF_TYPE_ETHERNET_CSMACD;
}

void CNetIf4::set_ifindex(IF_INDEX a_IfIndex) { m_adapts.IfIndex = a_IfIndex; }

void CNetIf4::chain_next(::PIP_ADAPTER_ADDRESSES a_ptrNextAddr) {
    m_adapts.Next = a_ptrNextAddr;
}

} // namespace upnp
