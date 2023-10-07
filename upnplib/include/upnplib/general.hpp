#ifndef UPNPLIB_GENERAL_HPP
#define UPNPLIB_GENERAL_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-10-07

// Here we have general used functions and tools

#include "upnplib/visibility.hpp"
#include <string>


// strndup() is a GNU extension.
#ifndef HAVE_STRNDUP
UPNPLIB_API char* strndup(const char* __string, size_t __n);
#endif

namespace upnplib {
UPNPLIB_API std::string libinfo();
}

#endif // UPNPLIB_GENERAL_HPP
