// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-09-26

// Yes, we have this global varibles in namespace ::upnplib, but they are only
// needed as long as the old C sources not re-engeneered to C++ objects. We need
// these globals to mock calls of system functions from the old C sources
// because there is no dependency injection possible. In the production
// environment these globels are never touched after instantiation. Its objects
// wrap the system calls and are only statically used to call them.
//
// For further information look at the header files in upnp/include/upnpmock/.

#include "upnpmock/winsock2_win32.hpp"

namespace upnplib {

Bwinsock2 winsock2Obj{};
Bwinsock2* winsock2_h = &winsock2Obj;

} // namespace upnplib
