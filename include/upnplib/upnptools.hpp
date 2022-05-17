// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-05-17

#ifndef UPNPLIB_UPNPTOOLS_HPP
#define UPNPLIB_UPNPTOOLS_HPP

#include "upnplib/port.hpp" // for UPNPLIB_API
#include <string>

namespace upnplib {

const char* err_c_str(int rc);
UPNPLIB_API const std::string errStr(int error);
UPNPLIB_API const std::string errStrEx(const int error, const int success);

} // namespace upnplib

#endif // UPNPLIB_UPNPTOOLS_HPP
