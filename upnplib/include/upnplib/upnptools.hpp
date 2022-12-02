#ifndef UPNPLIB_UPNPTOOLS_HPP
#define UPNPLIB_UPNPTOOLS_HPP
// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-12-03

#include "upnplib/visibility.hpp" // for UPNPLIB_API
#include <string>

namespace upnplib {

UPNPLIB_API const std::string errStr(int error);
UPNPLIB_API const std::string errStrEx(const int error, const int success);

} // namespace upnplib

#endif // UPNPLIB_UPNPTOOLS_HPP
