// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-11-09

#ifndef UPNP_IFADDRS_WIN_HPP
#define UPNP_IFADDRS_WIN_HPP

#include "port.hpp"
#include <winsock2.h>
#include <iphlpapi.h>
#include <iostream>

namespace upnp {

class UPNP_API CIfaddr4 {
    // Tool to manage and fill a network adapter structure. This is needed
    // for mocked network interfaces.
    // References:
    // [GetAdaptersAddresses function (iphlpapi.h)]
    // (https://docs.microsoft.com/en-us/windows/win32/api/iphlpapi/nf-iphlpapi-getadaptersaddresses)
    // [IP_ADAPTER_ADDRESSES_LH structure (iptypes.h)]
    // (https://docs.microsoft.com/en-us/windows/win32/api/iptypes/ns-iptypes-ip_adapter_addresses_lh)

  public:
    CIfaddr4();
    PIP_ADAPTER_ADDRESSES get();
    bool set(std::string t_Ifname, std::string t_Ifaddress);
    void chain_next_addr(PIP_ADAPTER_ADDRESSES ptrNextAddr);
};

// class CIfaddr4Container;

} // namespace upnp

#endif // UPNP_IFADDRS_WIN_HPP
