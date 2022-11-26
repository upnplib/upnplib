#ifdef UPNPLIB_WITH_NATIVE_PUPNP
// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-11-26
// Taken from authors who haven't made a note.

#ifndef PUPNP_UPNPSTDINT_HPP
#define PUPNP_UPNPSTDINT_HPP

/* Sized integer types. */
#include <stdint.h>

#if !defined(UPNP_USE_BCBPP)

#ifdef UPNP_USE_MSVCPP
/* no ssize_t defined for VC */
#ifdef _WIN64
typedef int64_t ssize_t;
#else
typedef int32_t ssize_t;
#endif
#endif

#endif /* !defined(UPNP_USE_BCBPP) */

#endif /* PUPNP_UPNPSTDINT_HPP */
#endif // UPNPLIB_WITH_NATIVE_PUPNP
