#ifndef UMOCK_IPHLPAPI_MOCK_HPP
#define UMOCK_IPHLPAPI_MOCK_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-08-09

#include <umock/iphlpapi.hpp>
#include <gmock/gmock.h>

namespace umock {

class IphlpapiMock : public umock::IphlpapiInterface {
  public:
    virtual ~IphlpapiMock() override = default;
    MOCK_METHOD(ULONG, GetAdaptersAddresses,
                (ULONG Family, ULONG Flags, PVOID Reserved,
                 PIP_ADAPTER_ADDRESSES AdapterAddresses, PULONG SizePointer),
                (override));
};

} // namespace umock

#endif // UMOCK_IPHLPAPI_MOCK_HPP
