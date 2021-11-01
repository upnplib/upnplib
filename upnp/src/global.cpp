// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-11-01

// Yes, we have this global varibles, but they are only needed as long as the
// old C sources not re-engeneered to C++ objects. We need these globals to mock
// calls of system functions from the old C sources because there is no
// dependency injection possible. In the production environment these globels
// are never touched after instantiation. Its objects wrap the system calls and
// are only statically used to call them.
//
// For further information look at the header files in upnp/include/upnpmock/.

#include "upnpmock/pthread.hpp"
#include "upnpmock/stdio.hpp"
#include "upnpmock/stdlib.hpp"
#include "upnpmock/string.hpp"

namespace upnp {

Bpthread* pthread_h;
Bpthread pthreadObj;

Bstdio* stdio_h;
Bstdio stdioObj;

Bstdlib* stdlib_h;
Bstdlib stdlibObj;

Bstring* string_h;
Bstring stringObj;

} // namespace upnp
