// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-10-10

// Header file for portable definitions
// ====================================
// This header should be includable into any source file to have portable
// definitions available. So it should never have other includes.

#ifndef UPNP_INCLUDE_PORT_H
#define UPNP_INCLUDE_PORT_H

// Check Debug settings. Exlusive NDEBUG or DEBUG must be set.
#if defined(NDEBUG) && defined(DEBUG)
#error "NDBUG and DEBUG are defined. Only one is possible."
#endif
#if !defined(NDEBUG) && !defined(DEBUG)
#error "Neither NDBUG nor DEBUG is definded."
#endif

// On MS Windows <unistd.h> isn't availabe. We can use <io.h> instead for most
// functions but it's not 100% compatible.
#if _WIN32
#define PORT_UNISTD_H <io.h>
#else
#define PORT_UNISTD_H <unistd.h>
#endif

#endif // UPNP_INCLUDE_PORT_H
