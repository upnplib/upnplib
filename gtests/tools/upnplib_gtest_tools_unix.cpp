// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-12-05

// Tools and helper classes to manage gtests
// =========================================

#include "upnplib_gtest_tools_unix.hpp"

#include <arpa/inet.h>
#include <net/if.h>

namespace upnplib {

//
// CIfaddr4
// --------

CIfaddr4::CIfaddr4()
// With constructing the object you get a loopback device by default.
{
    // loopback interface
    //-------------------
    // set network address
    m_ifa_addr.sin_family = AF_INET;
    // m_ifa_addr.sin_port = htons(MYPORT);
    inet_aton("127.0.0.1", &(m_ifa_addr.sin_addr));

    // set netmask
    m_ifa_netmask.sin_family = AF_INET;
    // m_ifa_netmask.sin_port = htons(MYPORT);
    inet_aton("255.0.0.0", &(m_ifa_netmask.sin_addr));

    // set broadcast address or Point-to-point destination address
    m_ifa_ifu.sin_family = AF_INET;
    // m_ifa_ifu.sin_port = htons(MYPORT);
    inet_aton("0.0.0.0", &(m_ifa_ifu.sin_addr));

    m_ifaddr.ifa_next = nullptr; // pointer to next ifaddrs structure
    m_ifaddr.ifa_name = (char*)"lo";
    // v-- Flags from SIOCGIFFLAGS, man 7 netdevice
    m_ifaddr.ifa_flags = 0 | IFF_LOOPBACK | IFF_UP;
    m_ifaddr.ifa_addr = (struct sockaddr*)&m_ifa_addr;
    m_ifaddr.ifa_netmask = (struct sockaddr*)&m_ifa_netmask;
    m_ifaddr.ifa_broadaddr = (struct sockaddr*)&m_ifa_ifu;
    m_ifaddr.ifa_data = nullptr;
}

ifaddrs* CIfaddr4::get()
// Return the pointer to the m_ifaddr structure
{
    return &m_ifaddr;
}

bool CIfaddr4::set(std::string_view a_Ifname, std::string_view a_Ifaddress)
// Set the interface name and the ipv4 address with bitmask. Properties are
// set to an ipv4 UP interface, supporting broadcast and multicast.
// Returns true if successful.
{
    if (a_Ifname == "" or a_Ifaddress == "")
        return false;

    // to be thread save we will have the strings here
    m_Ifname = a_Ifname;
    m_Ifaddress = a_Ifaddress;
    m_ifaddr.ifa_name = (char*)m_Ifname.c_str();

    // get the netmask from the bitmask
    // the bitmask is the offset in the netmasks array.
    std::size_t slashpos = m_Ifaddress.find_first_of("/");
    std::string address = m_Ifaddress;
    std::string bitmask = "32";
    if (slashpos != std::string::npos) {
        address = m_Ifaddress.substr(0, slashpos);
        bitmask = m_Ifaddress.substr(slashpos + 1);
    }
    // std::cout << "DEBUG! set ifa_name: " << m_ifaddr.ifa_name << ",
    // address: '" << address << "', bitmask: '" << bitmask << "', netmask: " <<
    // netmasks[std::stoi(bitmask)] << ", slashpos: " << slashpos << "\n";

    // convert address strings to numbers and store them
    inet_aton(address.c_str(), &(m_ifa_addr.sin_addr));
    std::string netmask = netmasks[std::stoi(bitmask)];
    inet_aton(netmask.c_str(), &(m_ifa_netmask.sin_addr));
    m_ifaddr.ifa_flags = 0 | IFF_UP | IFF_BROADCAST | IFF_MULTICAST;

    // calculate broadcast address as follows: broadcast = ip | ( ~ subnet )
    // broadcast = ip-addr ored the inverted subnet-mask
    m_ifa_ifu.sin_addr.s_addr =
        m_ifa_addr.sin_addr.s_addr | ~m_ifa_netmask.sin_addr.s_addr;

    m_ifaddr.ifa_addr = (struct sockaddr*)&m_ifa_addr;
    m_ifaddr.ifa_netmask = (struct sockaddr*)&m_ifa_netmask;
    m_ifaddr.ifa_broadaddr = (struct sockaddr*)&m_ifa_ifu;
    return true;
}

void CIfaddr4::chain_next_addr(struct ifaddrs* a_ptrNextAddr) {
    m_ifaddr.ifa_next = a_ptrNextAddr;
}

//
// CIfaddr4Container
// -----------------

bool CIfaddr4Container::add(std::string_view a_Ifname,
                            std::string_view a_Ifaddress) {
    ifaddrs* ifaddr4New;

    if (a_Ifname == "lo" and a_Ifaddress != "127.0.0.1/8")
        return false;

    if (m_ifaddr4Container.empty()) {
        // On an empty container just push an interface object to it
        m_ifaddr4Container.push_back(CIfaddr4());
        if (a_Ifname == "lo")
            return true;
        return m_ifaddr4Container[0].set(a_Ifname, a_Ifaddress);
    }

    // If the container is not empty, get the position to the last
    // interface object, append a new interface object and chain it to the
    // low level structure ifaddrs, managed by previous interface object.
    // pos is zero based, so the size points direct to a new entry.
    int pos = m_ifaddr4Container.size();
    m_ifaddr4Container.push_back(CIfaddr4());

    // Because the ifaddr.ifa_next address pointer chain changes when adding
    // an interface object, the chain must be rebuild
    for (int i = 1; i <= pos; i++) {
        ifaddr4New = m_ifaddr4Container.at(i).get();
        m_ifaddr4Container.at(i - 1).chain_next_addr(ifaddr4New);
    }

    if (a_Ifname == "lo")
        return true;
    return m_ifaddr4Container.at(pos).set(a_Ifname, a_Ifaddress);
}

ifaddrs* CIfaddr4Container::get_ifaddr(long unsigned int pIdx) {
    if (pIdx == 0)
        return nullptr;

    // This throws an exception if vector.size() is violated.
    // Don't return direct, return the variable. Otherwise I found that the
    // structure isn't correct returned on macOS.
    // return m_ifaddr4Container.at(pIdx - 1).get();
    ifaddrs* ifaddr4 = m_ifaddr4Container.at(pIdx - 1).get();
    return ifaddr4;
}

} // namespace upnplib
