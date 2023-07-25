#ifndef PUPNP_UPNPDEBUG_HPP
#define PUPNP_UPNPDEBUG_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-07-26

#include <upnpdebug.hpp>
#include <UpnpGlobal.hpp> // for EXPORT_SPEC

namespace pupnp {

// Helper class
// ============
class EXPORT_SPEC CLogging { /*
 * Use it for example with:
    class CLogging loggingObj; // Output only with build type DEBUG.
 * or
    class CLogging loggingObj(UPNP_ALL); // Output only with build type DEBUG.
 * or other loglevel.
 */
  public:
    CLogging(Upnp_LogLevel a_loglevel = UPNP_INFO);
    virtual ~CLogging();
};

} // namespace pupnp

#endif // PUPNP_UPNPDEBUG_HPP
