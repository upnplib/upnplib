// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-12-02

// Yes, we have this global varibles, but they are only needed as long as the
// old C sources not re-engeneered to C++ objects. We need these globals to mock
// calls of system functions from the old C sources because there is no
// dependency injection possible. In the production environment these globels
// are never touched after instantiation. Its objects wrap the system calls and
// are only statically used to call them.
//
// For further information look at the header files in upnp/include/upnpmock/.

#include "upnpmock/ifaddrs.hpp"
#include "upnpmock/net_if.hpp"
#include "upnpmock/sys_socket.hpp"
#include "upnpmock/sys_select.hpp"

namespace upnp {

Bifaddrs ifaddrsObj{};
Bifaddrs* ifaddrs_h = &ifaddrsObj;

Bnet_if net_ifObj{};
Bnet_if* net_if_h = &net_ifObj;

Bsys_socket sys_socketObj{};
Bsys_socket* sys_socket_h = &sys_socketObj;

Bsys_select sys_selectObj{};
Bsys_select* sys_select_h = &sys_selectObj;

} // namespace upnp
