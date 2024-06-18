// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-06-20

// There is no include guard '#ifndef ...' because this file shouln't be
// included more than two times as given.

/*!
 * \file
 * \brief Global used constants, variables, functions and macros.
 */

// Due to the global nature of this header file additional #include statements
// should be taken with great care. They are included in nearly all other
// compile units.
#include <upnplib/visibility.hpp>
/// \cond
#include <string>


// strndup() is a GNU extension.
// -----------------------------
#ifndef HAVE_STRNDUP
UPNPLIB_API char* strndup(const char* __string, size_t __n);
#endif

namespace upnplib {

// Global constants
// ================
// Default response timeout for UPnP messages as given by The UPnP™ Device
// Architecture 2.0, Document Revision Date: April 17, 2020.
inline constexpr int g_response_timeout{30};

// Info message about the library
inline constexpr std::string_view libinfo{
    "upnplib library version = under developement"};

} // namespace upnplib
/// \endcond
