#ifndef COMPA_UPNPUNISTD_HPP
#define COMPA_UPNPUNISTD_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-01-26
// Taken from authors who haven't made a note.
/*!
 * \file
 * \brief Do not \#include <unistd.h> on WIN32
 */

#ifdef _WIN32
/* Do not #include <unistd.h> on WIN32. */
#else               /* _WIN32 */
/// \cond
#include <unistd.h> /* for close() */
/// \endcond
#endif              /* _WIN32 */

#endif /* COMPA_UPNPUNISTD_HPP */
