/*******************************************************************************
 *
 * Copyright (c) 2006 Rémi Turboult <r3mi@users.sourceforge.net>
 * Copyright (c) 2021 Ingo Höft     <Ingo@Hoeft-online.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * * Neither name of Intel Corporation nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

#ifndef UPNP_CONFIG_H
#define UPNP_CONFIG_H

/***************************************************************************
 * Library version
 ***************************************************************************/

// TODO: Check the version handling
/** The library version (string) e.g. "1.3.0" */
#cmakedefine UPNP_VERSION_STRING "${UPNP_VERSION_STRING}"
/** Major version of the library */
#cmakedefine UPNP_VERSION_MAJOR ${UPNP_VERSION_MAJOR}
/** Minor version of the library */
#define UPNP_VERSION_MINOR 0
/** Patch version of the library */
#define UPNP_VERSION_PATCH 0
/** The library version (numeric) e.g. 10300 means version 1.3.0 */
#define UPNP_VERSION \
((UPNP_VERSION_MAJOR * 100 + UPNP_VERSION_MINOR) * 100 + \
UPNP_VERSION_PATCH)

/***************************************************************************
 * Large file support
 ***************************************************************************/
/* whether the system defaults to 32bit off_t but can do 64bit when requested
 * warning libupnp requires largefile mode - use AC_SYS_LARGEFILE */
#define UPNP_LARGEFILE_SENSITIVE 1

/***************************************************************************
 * Library optional features
 ***************************************************************************/
/*
 * The following defines can be tested in order to know which
 * optional features have been included in the installed library.
 */

// TODO: optimize following defines in ./upnp/src/inc/config.h
/** Defined to 1 if the library has been compiled with client API enabled */
//#cmakedefine UPNP_HAVE_CLIENT 1

/** Defined to 1 if the library has been compiled with device API enabled */
//#cmakedefine UPNP_HAVE_DEVICE 1

/** Defined to 1 if the library has been compiled with integrated web server */
//#cmakedefine UPNP_HAVE_WEBSERVER 1

/** Defined to 1 if the library has been compiled with the SSDP part enabled */
//#cmakedefine UPNP_HAVE_SSDP 1

/** Defined to 1 if the library has been compiled with optional SSDP headers
 *  support */
//#cmakedefine UPNP_HAVE_OPTSSDP 1

/** Defined to 1 if the library has been compiled with the SOAP part enabled */
//#cmakedefine UPNP_HAVE_SOAP 1

/** Defined to 1 if the library has been compiled with the GENA part enabled */
//#cmakedefine UPNP_HAVE_GENA 1

/** Defined to 1 if the library has been compiled with ipv6 support */
//#cmakedefine UPNP_ENABLE_IPV6 1

/** Defined to 1 if the library has been compiled with unspecified SERVER
 * header */
//#cmakedefine UPNP_ENABLE_UNSPECIFIED_SERVER 1

/** Defined to 1 if the library has been compiled with OpenSSL support */
//#cmakedefine UPNP_ENABLE_OPEN_SSL 1

/** Defined to 1 if the library has been compiled to use blocking TCP socket
 * calls */
//#cmakedefine UPNP_ENABLE_BLOCKING_TCP_CONNECTIONS 1

/* Defined to 1 if the library has been compiled with debug information */
//#cmakedefine UPNP_HAVE_DEBUG 1

/* Defined to 1 if the library has IXML script support enabled */
//#cmakedefine IXML_HAVE_SCRIPTSUPPORT 1

/* Defined to ON if the library will be created statically linkable */
#cmakedefine UPNP_STATIC_LIB ${UPNP_STATIC_LIB}

/* Defined to ON if the library will use the static pthreads4W library */
#cmakedefine PTW32_STATIC_LIB ${PTW32_STATIC_LIB}

/* Defines if strnlen is available on your system */
#cmakedefine HAVE_STRNLEN ${HAVE_STRNLEN}

/* Defines if strndup is available on your system */
#cmakedefine HAVE_STRNDUP ${HAVE_STRNDUP}

/* Use pthread_rwlock_t */
//#cmakedefine UPNP_USE_RWLOCK 1

#endif /* UPNP_CONFIG_H */
