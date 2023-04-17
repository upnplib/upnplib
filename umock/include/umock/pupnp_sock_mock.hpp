#ifndef UMOCK_PUPNP_SOCK_MOCK_HPP
#define UMOCK_PUPNP_SOCK_MOCK_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-04-17

#include <umock/pupnp_sock.hpp>
#include <gmock/gmock.h>

namespace umock {

class PupnpSockMock : public umock::PupnpSockInterface {
  public:
    virtual ~PupnpSockMock() override = default;
    MOCK_METHOD(int, sock_make_blocking, (SOCKET sock), (override));
    MOCK_METHOD(int, sock_make_no_blocking, (SOCKET sock), (override));
};

} // namespace umock

#endif // UMOCK_PUPNP_SOCK_MOCK_HPP
