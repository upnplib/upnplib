#ifndef COMPA_UPNPGLOBAL_HPP
#define COMPA_UPNPGLOBAL_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-08-05
// Taken from authors who haven't made a note.

// Include file for backward compatibility with old ixml code.

#include <upnplib/visibility.hpp>

// With C++ the inline statement belongs to the standard. There are no
// differences on different platforms anymore.
#define UPNP_INLINE inline

// Switch old pupnp definition to use new visibility support.
#define EXPORT_SPEC UPNPLIB_API
#define EXPORT_SPEC_LOCAL UPNPLIB_LOCAL
#define EXPORT_SPEC_EXTERN UPNPLIB_EXTERN

#endif /* COMPA_UPNPGLOBAL_HPP */
