// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-07-26

#include <pupnp/upnpdebug.hpp>
#include <upnplib/port.hpp>

#include <stdexcept>
#include <string>

namespace pupnp {

CLogging::CLogging(Upnp_LogLevel a_loglevel) {
    TRACE2(this, " Construct CLogging()");
    UpnpSetLogLevel(a_loglevel);
    if (UpnpInitLog() != UPNP_E_SUCCESS) {
        throw std::runtime_error(
            std::string("UpnpInitLog(): failed to initialize pupnp logging."));
    }
}

CLogging::~CLogging() {
    TRACE2(this, " Destruct CLogging()");
    UpnpCloseLog();
}

} // namespace pupnp
