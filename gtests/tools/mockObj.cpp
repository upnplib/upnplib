// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-09-20

// Yes, we have this global varibles in namespace ::upnplib. We need them to
// mock calls independent from the call stack of a mocked Unit. In the
// production environment these globels are never touched after instantiation.
// Its objects wrap the gmock calls only under test.
//
// For further information look at the header files in upnp/include/upnpmock/.

#include "upnpmock/pthread.hpp"
#include "upnpmock/stdio.hpp"
#include "upnpmock/stdlib.hpp"
#include "upnpmock/string.hpp"
#include "upnpmock/sys_socket.hpp"
#include "upnpmock/unistd.hpp"

namespace upnplib {

class Bpthread pthreadObj {};
Bpthread* pthread_h = &pthreadObj;

class Bstdio stdioObj {};
Bstdio* stdio_h = &stdioObj;

class Bstdlib stdlibObj {};
Bstdlib* stdlib_h = &stdlibObj;

class Bstring stringObj {};
Bstring* string_h = &stringObj;

class Bsys_socket sys_socketObj {};
Bsys_socket* sys_socket_h = &sys_socketObj;

class Bunistd unistdObj {};
Bunistd* unistd_h = &unistdObj;

} // namespace upnplib
