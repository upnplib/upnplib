#if defined(COMPA_HAVE_CTRLPT_SSDP) || defined(COMPA_HAVE_DEVICE_SSDP) ||      \
    defined(INCLUDE_CLIENT_APIS) || defined(INCLUDE_DEVICE_APIS)

#ifndef UMOCK_PUPNP_SSDP_MOCK_HPP
#define UMOCK_PUPNP_SSDP_MOCK_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2024-03-21

#include <umock/pupnp_ssdp.hpp>
#include <gmock/gmock.h>

namespace umock {

class PupnpSsdpMock : public umock::PupnpSsdpInterface {
  public:
    virtual ~PupnpSsdpMock() override = default;
    MOCK_METHOD(int, get_ssdp_sockets, (MiniServerSockArray * miniSock),
                (override));
};

} // namespace umock

#endif // UMOCK_PUPNP_SSDP_MOCK_HPP
#endif // defined(COMPA_HAVE_CTRLPT_SSDP) || defined(COMPA_HAVE_DEVICE_SSDP)
