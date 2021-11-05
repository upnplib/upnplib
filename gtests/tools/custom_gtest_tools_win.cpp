// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-11-06

// Implementation of the ifaddrs classes
// =====================================

#include "custom_gtest_tools_win.hpp"

namespace upnp {

CIfaddr4::CIfaddr4() {
    // With constructing the object you get a loopback device by default.
    std::cout << "Constructor executed.\n";
}

PIP_ADAPTER_ADDRESSES CIfaddr4::get() {
    // Return the pointer to the ifaddr structure
    return nullptr;
}

bool CIfaddr4::set(std::string pIfname, std::string pIfaddress) {
    // Set the interface name and the ipv4 address with bitmask. Properties are
    // set to an ipv4 UP interface, supporting broadcast and multicast.
    // Returns true if successful.
    return true;
}

void CIfaddr4::chain_next_addr(PIP_ADAPTER_ADDRESSES ptrNextAddr) {}

} // namespace upnp
