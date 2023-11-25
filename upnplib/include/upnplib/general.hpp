#ifndef UPNPLIB_GENERAL_HPP
#define UPNPLIB_GENERAL_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-12-01

#include <upnplib/general.inc>

namespace upnplib {

// Global switch to enable verbose (debug) output.
UPNPLIB_EXTERN bool g_dbug;

// Response timeout for UPnP messages as given by The UPnP™ Device
// Architecture 2.0, Document Revision Date: April 17, 2020.
inline constexpr int g_response_timeout{30};

} // namespace upnplib

#endif // UPNPLIB_GENERAL_HPP
