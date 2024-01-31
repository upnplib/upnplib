#ifndef PUPNP_UPNPDEBUG_HPP
#define PUPNP_UPNPDEBUG_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-01-30

#include <upnpdebug.hpp>
#include <UpnpGlobal.hpp> // for EXPORT_SPEC

namespace pupnp {

// Helper class
// ============
/// \brief Helper class for debug log messages in compatible code.
class EXPORT_SPEC CLogging { /*
 * Use it for example with:
    CLogging loggingObj; // Output only with build type DEBUG.
    loggingObj.enable(UPNP_ALL); // or other loglevel, e.g. UPNP_INFO.
    loggingObj.disable(); // optional
 */
  public:
    CLogging();
    virtual ~CLogging();
    /// -brief Enable debug logging messages.
    void enable(Upnp_LogLevel a_loglevel);
    /// -brief Disable debug logging messages.
    void disable();
};

} // namespace pupnp

#endif // PUPNP_UPNPDEBUG_HPP
