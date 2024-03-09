#ifndef UMOCK_PUPNP_SOCK_HPP
#define UMOCK_PUPNP_SOCK_HPP
// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-03-10

// This mocks a function from the pupnp library. Due to conflicts with exported
// symbols we cannot handle it like an independent function as given by a
// system function from the Standard Template Library. We have to link the
// pupnp_sock.cpp source file direct with the pupnp library and cannot add it
// to the internal umock library. This is the reason why you do not find
// export/import decorator(UPNPLIB_API etc.).

#include <umock/pupnp_sock.inc>

namespace umock {

extern PupnpSock pupnp_sock;

} // namespace umock

#endif // UMOCK_PUPNP_SOCK_HPP
