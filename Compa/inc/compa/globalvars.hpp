#ifndef COMPA_GLOBALVARS_HPP
#define COMPA_GLOBALVARS_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-01-24
/*!
 * \file
 * \brief Provides global variables.
 */

#ifdef UPNP_ENABLE_OPEN_SSL
#include <upnplib/visibility.hpp>
#include <openssl/ssl.h>

UPNPLIB_EXTERN SSL_CTX* gSslCtx;
#endif

#endif // COMPA_GLOBALVARS_HPP
