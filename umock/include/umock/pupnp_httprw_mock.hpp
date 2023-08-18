#ifndef UMOCK_PUPNP_HTTPRW_MOCK_HPP
#define UMOCK_PUPNP_HTTPRW_MOCK_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-08-20

#include <umock/pupnp_httprw.hpp>
#include <gmock/gmock.h>

namespace umock {

class PupnpHttpRwMock : public umock::PupnpHttpRwInterface {
  public:
    virtual ~PupnpHttpRwMock() override = default;
    MOCK_METHOD(int, private_connect,
                (SOCKET sockfd, const struct sockaddr* serv_addr,
                 socklen_t addrlen),
                (override));
};

} // namespace umock

#endif // UMOCK_PUPNP_HTTPRW_MOCK_HPP
