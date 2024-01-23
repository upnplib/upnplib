#ifndef UMOCK_IPHLPAPI_MOCK_HPP
#define UMOCK_IPHLPAPI_MOCK_HPP
// Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-12-24

#include <umock/iphlpapi.hpp>
#include <upnplib/port.hpp>
#include <gmock/gmock.h>

namespace umock {

class UPNPLIB_API IphlpapiMock : public umock::IphlpapiInterface {
  public:
    IphlpapiMock();
    virtual ~IphlpapiMock() override;
    SUPPRESS_MSVC_WARN_4251_NEXT_LINE
    MOCK_METHOD(ULONG, GetAdaptersAddresses,
                (ULONG Family, ULONG Flags, PVOID Reserved,
                 PIP_ADAPTER_ADDRESSES AdapterAddresses, PULONG SizePointer),
                (override));
};

} // namespace umock

#endif // UMOCK_IPHLPAPI_MOCK_HPP
