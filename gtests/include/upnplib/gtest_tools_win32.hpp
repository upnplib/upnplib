// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-03-09

#ifndef UPNP_TOOLS_WIN_HPP
#define UPNP_TOOLS_WIN_HPP

#include "upnplib/port.hpp"
#include <winsock2.h>
#include <iphlpapi.h>
#include <iostream>

namespace upnplib {

class UPNPLIB_API CNetIf4
// Object to manage and fill a network adapter structure. This is needed for
// mocked network interfaces. References:
// [GetAdaptersAddresses_function_(iphlpapi.h)]
// (https://docs.microsoft.com/en-us/windows/win32/api/iphlpapi/nf-iphlpapi-getadaptersaddresses)
// [IP_ADAPTER_ADDRESSES_LH_structure_(iptypes.h)]
// (https://docs.microsoft.com/en-us/windows/win32/api/iptypes/ns-iptypes-ip_adapter_addresses_lh)
{
  public:
    CNetIf4();
    // With constructing the object you get a loopback device by default.
    // Properties are set to an ipv4 UP interface, supporting broadcast and
    // multicast.

    ::PIP_ADAPTER_ADDRESSES get();
    // Return the pointer to a network interface structure.
    //
    // Exception: no-fail guarantee.

    void set(std::wstring_view a_Ifname, std::string_view a_Ifaddress);
    // Set the interface name and the ipv4 address with bitmask.
    // An empty a_Ifname will do nothing. An empty a_Ifaddress will set an
    // adapter structure with a zero ip address and bitmask. An ip address
    // without a bitmask will set a host ip address with bitmask '/32'.
    //
    // Exception: Strong guarantee (no modifications)
    //    throws: [std::logic_error] <- std::invalid_argument
    // A wrong ip address will throw an exception.

    void set_ifindex(::IF_INDEX a_IfIndex);
    // Sets the interface index as shown by the operating system tools.
    //
    // Exception: No-fail guarantee

    void chain_next(::PIP_ADAPTER_ADDRESSES a_ptrNextIf);
    // Sets the pointer to the next network interface structure in the interface
    // chain.
    //
    // Exception: No-fail guarantee

  private:
    // Structures needed to form the interface structure.
    // https://docs.microsoft.com/en-us/windows/win32/winsock/sockaddr-2
    UPNPLIB_LOCAL ::sockaddr_in m_inaddr{};
    // https://docs.microsoft.com/en-us/windows/win32/api/ws2def/ns-ws2def-socket_address
    UPNPLIB_LOCAL ::SOCKET_ADDRESS m_saddr{};
    // https://docs.microsoft.com/en-us/windows/win32/api/iptypes/ns-iptypes-ip_adapter_unicast_address_lh?redirectedfrom=MSDN
    UPNPLIB_LOCAL ::IP_ADAPTER_UNICAST_ADDRESS m_uniaddr{};
    // https://docs.microsoft.com/en-us/windows/win32/api/iptypes/ns-iptypes-ip_adapter_addresses_lh#see-also
    UPNPLIB_LOCAL ::IP_ADAPTER_ADDRESSES m_adapts{};

    // On the adapter (net interface) structure we only have pointer to strings
    // so we need to save them here to be sure we do not get dangling pointer.
    UPNPLIB_LOCAL std::wstring m_Description{
        L"Mocked Adapter for Unit testing"};
    UPNPLIB_LOCAL std::wstring m_FriendlyName{L"Loopback Pseudo-Interface 1"};
};

} // namespace upnplib

#endif // UPNP_TOOLS_WIN_HPP
