// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-12-18
/*!
 * \file
 * \ingroup compa-Operating
 * \brief Provides global used constants and variables.
 */

#include <compa/globalvars.hpp>
#include <upnplib/port.hpp>

// Global variables
// ================
/*! \brief Global variable used to store the OpenSSL context object to be used
 * for all SSL/TLS connections.
 *
 * \todo Do not export this variable to be usable outside the library.
 */
#ifdef UPNP_ENABLE_OPEN_SSL
// Warning 4273: 'function' : inconsistent DLL linkage.
SUPPRESS_MSVC_WARN_4273_NEXT_LINE
UPNPLIB_API SSL_CTX* gSslCtx{nullptr};
#endif
