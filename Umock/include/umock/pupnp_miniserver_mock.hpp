#ifndef UMOCK_PUPNP_MINISERVER_MOCK_HPP
#define UMOCK_PUPNP_MINISERVER_MOCK_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-03-19

#include <umock/pupnp_miniserver.hpp>
#include <gmock/gmock.h>

namespace umock {

class PupnpMiniServerMock : public umock::PupnpMiniServerInterface {
  public:
    virtual ~PupnpMiniServerMock() override = default;
    MOCK_METHOD(void, RunMiniServer, (MiniServerSockArray * miniSock),
                (override));
};

} // namespace umock

#endif // UMOCK_PUPNP_MINISERVER_MOCK_HPP
