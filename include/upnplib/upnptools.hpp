// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-03-07

#ifndef UPNPLIB_UPNPTOOLS_HPP
#define UPNPLIB_UPNPTOOLS_HPP

#include "UpnpGlobal.hpp" // for EXPORT_SPEC
#include <string>

EXPORT_SPEC const char* UpnpGetErrorMessage(int rc);

namespace upnplib {

const char* errCstr(int rc);
const std::string errStrEx(int rc);

} // namespace upnplib

#endif // UPNPLIB_UPNPTOOLS_HPP
