// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-12-18

// Here we have global used constants and variables.

#include <compa/globalvars.hpp>
#include <upnplib/port.hpp>

// Global variables
// ================
/*! Global variable used to store the OpenSSL context object to be used for all
 * SSL/TLS connections.
 */
#ifdef UPNP_ENABLE_OPEN_SSL
DISABLE_MSVC_WARN_4273
// Warning 4273: 'function' : inconsistent DLL linkage.
// This is expected on propagate global variables with included header file
// (with __declspec(dllimport)) in its source file (with __declspec(dllexport)).

UPNPLIB_API SSL_CTX* gSslCtx{nullptr};
ENABLE_MSVC_WARN
#endif
