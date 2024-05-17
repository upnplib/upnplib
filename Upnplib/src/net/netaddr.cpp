// Copyright (C) 2024+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-05-19
/*!
 * \file
 * \brief Definition of the Netaddr class.
 */

#include <upnplib/netaddr.hpp>

#include <upnplib/global.hpp>
#include <upnplib/addrinfo.hpp>
#include <upnplib/synclog.hpp>
#ifdef UPNPLIB_WITH_TRACE
#include <iostream>
#endif

namespace upnplib {

// Netaddr class
// =============
CNetaddr::CNetaddr(){
    TRACE2(this, " Construct default CNetaddr()") //
}

CNetaddr::~CNetaddr() {
    TRACE2(this, " Destruct CNetaddr()") //
}

// Set a netaddress
// ----------------
void CNetaddr::set(std::string_view a_node, std::string_view a_service) {
    CAddrinfo ai(static_cast<std::string>(a_node),
                 static_cast<std::string>(a_service));
    try {
        ai.init();
    } catch (const std::exception& e) {
        UPNPLIB_LOGCATCH "MSG1116: catched next line...\n" << e.what();
        return;
    }
    m_netaddrp = ai.netaddrp();
}

} // namespace upnplib
