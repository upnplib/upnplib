// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-08-31

// Yes, we have this global varibles in namespace ::upnplib. We need them to
// mock calls independent from the call stack of a mocked Unit. In the
// production environment these globels are never touched after instantiation.
// Its objects wrap the gmock calls only under test.
//
// For further information look at the header files in include/upnpmock/.

#include "upnpmock/sys_socket.hpp"

namespace upnplib {

class Bsys_socket sys_socketObj {};
Bsys_socket* sys_socket_h = &sys_socketObj;

} // namespace upnplib
