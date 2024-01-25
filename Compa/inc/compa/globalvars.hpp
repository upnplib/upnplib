#ifndef COMPA_GLOBALVARS_HPP
#define COMPA_GLOBALVARS_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-01-27
/*!
 * \file
 * \brief Provides global variables.
 */

#if defined(UPNP_ENABLE_OPEN_SSL) || defined(DOXYGEN_RUN)
#include <upnplib/visibility.hpp>
#include <openssl/ssl.h>

/*! \brief Pointer to the global available SSL Context object if OpenSSL is
 * enabled.
 *
 * If OpenSSL is available and configured to use then an SSL_CTX object is
 * initialized and the pointer to it is global available so it can be used for
 * SSL connections. */
UPNPLIB_EXTERN SSL_CTX* gSslCtx;
#endif

#endif // COMPA_GLOBALVARS_HPP
