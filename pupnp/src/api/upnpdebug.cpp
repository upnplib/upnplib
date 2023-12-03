// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-12-06

#include <pupnp/upnpdebug.hpp>

#include <upnplib/global.hpp>
#include <stdexcept>
#include <string>

namespace pupnp {

CLogging::CLogging() = default;

void CLogging::enable(Upnp_LogLevel a_loglevel) {
    UpnpSetLogLevel(a_loglevel);
    if (UpnpInitLog() != UPNP_E_SUCCESS) {
        throw std::runtime_error(
            UPNPLIB_LOGEXCEPT + "MSG1041: Failed to initialize pupnp logging.");
    }
}

void CLogging::disable() { UpnpCloseLog(); }

CLogging::~CLogging() { UpnpCloseLog(); }

} // namespace pupnp
