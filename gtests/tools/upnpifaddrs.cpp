// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-10-10

// Tools and helper classes to manage gtests
// =========================================

#include "upnpifaddrs.hpp"

#include <arpa/inet.h>
#include <net/if.h>
#include <vector>

namespace upnp {

CIfaddr4::CIfaddr4()
// With constructing the object you get a loopback device by default.
{
    // loopback interface
    //-------------------
    // set network address
    ifa_addr.sin_family = AF_INET;
    // ifa_addr.sin_port = htons(MYPORT);
    inet_aton("127.0.0.1", &(ifa_addr.sin_addr));

    // set netmask
    ifa_netmask.sin_family = AF_INET;
    // ifa_netmask.sin_port = htons(MYPORT);
    inet_aton("255.0.0.0", &(ifa_netmask.sin_addr));

    // set broadcast address or Point-to-point destination address
    ifa_ifu.sin_family = AF_INET;
    // ifa_ifu.sin_port = htons(MYPORT);
    inet_aton("0.0.0.0", &(ifa_ifu.sin_addr));

    this->ifaddr.ifa_next = nullptr; // pointer to next ifaddrs structure
    this->ifaddr.ifa_name = (char*)"lo";
    // v-- Flags from SIOCGIFFLAGS, man 7 netdevice
    this->ifaddr.ifa_flags = 0 | IFF_LOOPBACK | IFF_UP;
    this->ifaddr.ifa_addr = (struct sockaddr*)&ifa_addr;
    this->ifaddr.ifa_netmask = (struct sockaddr*)&ifa_netmask;
    this->ifaddr.ifa_broadaddr = (struct sockaddr*)&ifa_ifu;
    this->ifaddr.ifa_data = nullptr;
}

ifaddrs* CIfaddr4::get()
// Return the pointer to the ifaddr structure
{
    return &this->ifaddr;
}

bool CIfaddr4::set(std::string pIfname, std::string pIfaddress)
// Set the interface name and the ipv4 address with bitmask. Properties are
// set to an ipv4 UP interface, supporting broadcast and multicast.
// Returns true if successful.
{
    if (pIfname == "" or pIfaddress == "")
        return false;

    // to be thread save we will have the strings here
    this->mIfname = pIfname;
    this->mIfaddress = pIfaddress;
    this->ifaddr.ifa_name = (char*)this->mIfname.c_str();

    // get the netmask from the bitmask
    // the bitmask is the offset in the netmasks array.
    std::size_t slashpos = this->mIfaddress.find_first_of("/");
    std::string address = this->mIfaddress;
    std::string bitmask = "32";
    if (slashpos != std::string::npos) {
        address = this->mIfaddress.substr(0, slashpos);
        bitmask = this->mIfaddress.substr(slashpos + 1);
    }
    // std::cout << "DEBUG! set ifa_name: " << this->ifaddr.ifa_name << ",
    // address: '" << address << "', bitmask: '" << bitmask << "', netmask: " <<
    // netmasks[std::stoi(bitmask)] << ", slashpos: " << slashpos << "\n";

    // convert address strings to numbers and store them
    inet_aton(address.c_str(), &(this->ifa_addr.sin_addr));
    std::string netmask = netmasks[std::stoi(bitmask)];
    inet_aton(netmask.c_str(), &(this->ifa_netmask.sin_addr));
    this->ifaddr.ifa_flags = 0 | IFF_UP | IFF_BROADCAST | IFF_MULTICAST;

    // calculate broadcast address as follows: broadcast = ip | ( ~ subnet )
    // broadcast = ip-addr ored the inverted subnet-mask
    this->ifa_ifu.sin_addr.s_addr =
        this->ifa_addr.sin_addr.s_addr | ~this->ifa_netmask.sin_addr.s_addr;

    this->ifaddr.ifa_addr = (struct sockaddr*)&this->ifa_addr;
    this->ifaddr.ifa_netmask = (struct sockaddr*)&this->ifa_netmask;
    this->ifaddr.ifa_broadaddr = (struct sockaddr*)&this->ifa_ifu;
    return true;
}

void CIfaddr4::chain_next_addr(struct ifaddrs* ptrNextAddr) {
    this->ifaddr.ifa_next = ptrNextAddr;
}

class CIfaddr4Container
// This is a Container for multiple network interface structures that are
// chained by ifaddr.ifa_next as given by the low level struct ifaddrs.
//
// It is IMPORTANT to know that the ifaddr.ifa_next address pointer chain
// changes when adding an interface address object. You MUST get_ifaddr(..)
// the new address pointer for ongoing work.
{
    std::vector<CIfaddr4> ifaddr4Container;

  public:
    bool add(std::string prmIfname, std::string prmIfaddress) {
        ifaddrs* ifaddr4New;

        if (prmIfname == "lo" and prmIfaddress != "127.0.0.1/8")
            return false;

        if (this->ifaddr4Container.empty()) {
            // On an empty container just push an interface object to it
            this->ifaddr4Container.push_back(CIfaddr4());
            if (prmIfname == "lo")
                return true;
            return this->ifaddr4Container[0].set(prmIfname, prmIfaddress);
        }

        // If the container is not empty, get the position to the last
        // interface object, append a new interface object and chain it to the
        // low level structure ifaddrs, managed by previous interface object.
        // pos is zero based, so the size points direct to a new entry.
        int pos = this->ifaddr4Container.size();
        this->ifaddr4Container.push_back(CIfaddr4());

        // Because the ifaddr.ifa_next address pointer chain changes when adding
        // an interface object, the chain must be rebuild
        for (int i = 1; i <= pos; i++) {
            ifaddr4New = this->ifaddr4Container.at(i).get();
            this->ifaddr4Container.at(i - 1).chain_next_addr(ifaddr4New);
        }

        if (prmIfname == "lo")
            return true;
        return this->ifaddr4Container.at(pos).set(prmIfname, prmIfaddress);
    }

    ifaddrs* get_ifaddr(long unsigned int pIdx) {
        if (pIdx == 0)
            return nullptr;

        // this throws an exception if vector.size() is violated
        ifaddrs* ifaddr4 = this->ifaddr4Container.at(pIdx - 1).get();
        return ifaddr4;
    }
};

} // namespace upnp
