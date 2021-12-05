// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-11-06

#ifndef UPNP_IFADDRS_UIX_HPP
#define UPNP_IFADDRS_UIX_HPP

#include "port.hpp"
#include <ifaddrs.h>
#include <netinet/in.h> // for sockaddr_in
#include <string>
#include <vector>

namespace upnp {

class UPNP_API CIfaddr4
// Tool to manage and fill a socket address structure. This is needed
// for mocked network interfaces.
{
  public:
    CIfaddr4();
    ifaddrs* get();
    bool set(std::string pIfname, std::string pIfaddress);
    void chain_next_addr(struct ifaddrs* ptrNextAddr);

  private:
    struct ifaddrs ifaddr;

    struct sockaddr_in ifa_addr;    // network address
    struct sockaddr_in ifa_netmask; // netmask
    struct sockaddr_in ifa_ifu; // broadcast addr or point-to-point dest addr

    std::string mIfname;    // interface name
    std::string mIfaddress; // interface ip address

    // clang-format off
    // the bitmask is the offset in the netmasks array.
    std::string netmasks[33] = {"0.0.0.0",
            "128.0.0.0", "192.0.0.0", "224.0.0.0", "240.0.0.0",
            "248.0.0.0", "252.0.0.0", "254.0.0.0", "255.0.0.0",
            "255.128.0.0", "255.192.0.0", "255.224.0.0", "255.240.0.0",
            "255.248.0.0", "255.252.0.0", "255.254.0.0", "255.255.0.0",
            "255.255.128.0", "255.255.192.0", "255.255.224.0", "255.255.240.0",
            "255.255.248.0", "255.255.252.0", "255.255.254.0", "255.255.255.0",
            "255.255.255.128", "255.255.255.192", "255.255.255.224", "255.255.255.240",
            "255.255.255.248", "255.255.255.252", "255.255.255.254", "255.255.255.255"};
    // clang-format on
};

//
class UPNP_API CIfaddr4Container {
    // This is a Container for multiple network interface structures that are
    // chained by ifaddr.ifa_next as given by the low level struct ifaddrs.
    //
    // It is IMPORTANT to know that the ifaddr.ifa_next address pointer chain
    // changes when adding an interface address object. You MUST get_ifaddr(..)
    // the new address pointer for ongoing work.
  public:
    bool add(std::string prmIfname, std::string prmIfaddress);
    ifaddrs* get_ifaddr(long unsigned int pIdx);

  private:
    std::vector<CIfaddr4> ifaddr4Container;
};

} // namespace upnp

#endif // UPNP_IFADDRS_UIX_HPP
