#ifndef COMPA_UPNPDEBUG_HPP
#define COMPA_UPNPDEBUG_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-02-25

#include <upnpdebug.hpp>

namespace compa {

// Helper class
// ============
class CLogging { /*
 * Use it for example with:
    class CLogging loggingObj; // Output only with build type DEBUG.
 * or
    class CLogging loggingObj(UPNP_ALL); // Output only with build type DEBUG.
 * or other loglevel.
 */
  public:
    CLogging(Upnp_LogLevel a_loglevel = UPNP_ALL);
    virtual ~CLogging();
};

} // namespace compa

#endif // COMPA_UPNPDEBUG_HPP
