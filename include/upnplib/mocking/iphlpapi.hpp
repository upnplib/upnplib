#ifndef UPNPLIB_MOCKING_IPHLPAPI_HPP
#define UPNPLIB_MOCKING_IPHLPAPI_HPP
// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-09-25

// iphlpapi.h is a Microsoft Windows library.

#include "upnplib/visibility.hpp"
#include <winsock2.h>
#include <iphlpapi.h>

namespace upnplib {
namespace mocking {

class IphlpapiInterface {
  public:
    virtual ~IphlpapiInterface() = default;
    virtual ULONG GetAdaptersAddresses(ULONG Family, ULONG Flags,
                                       PVOID Reserved,
                                       PIP_ADAPTER_ADDRESSES AdapterAddresses,
                                       PULONG SizePointer) = 0;
};

//
// This is the wrapper class for the real (library?) function
// ----------------------------------------------------------
class IphlpapiReal : public IphlpapiInterface {
  public:
    virtual ~IphlpapiReal() override = default;
    ULONG GetAdaptersAddresses(ULONG Family, ULONG Flags, PVOID Reserved,
                               PIP_ADAPTER_ADDRESSES AdapterAddresses,
                               PULONG SizePointer) override;
};

//
// This is the caller or injector class that injects the class (worker) to be
// used, real or mocked functions.
/* Example:
    IphlpapiReal iphlpapi_realObj;
    Iphlpapi(&iphlpapi_realObj;
    { // Other scope, e.g. within a gtest
        class IphlpapiMock { ...; MOCK_METHOD(...) };
        IphlpapiMock iphlpapi_mockObj;
        Iphlpapi(&iphlpapi_mockObj);
    } // End scope, mock objects are destructed, worker restored to default.
*///------------------------------------------------------------------------
class UPNPLIB_API Iphlpapi {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    Iphlpapi(IphlpapiReal* a_ptr_realObj);

    // This constructor is used to inject the pointer to the mocking function.
    Iphlpapi(IphlpapiInterface* a_ptr_mockObj);

    // The destructor is ussed to restore the default pointer.
    virtual ~Iphlpapi();

    virtual ULONG GetAdaptersAddresses(ULONG Family, ULONG Flags,
                                       PVOID Reserved,
                                       PIP_ADAPTER_ADDRESSES AdapterAddresses,
                                       PULONG SizePointer);

  private:
    // Must be static to be persistent also available on a new constructed
    // object. With inline we do not need an extra definition line outside the
    // class. I also make the symbol hidden so the variable cannot be accessed
    // globaly with Iphlpapi::m_ptr_workerObj.
    UPNPLIB_LOCAL static inline IphlpapiInterface* m_ptr_workerObj;
    IphlpapiInterface* m_ptr_oldObj{};
};

extern Iphlpapi UPNPLIB_API iphlpapi_h;

} // namespace mocking
} // namespace upnplib

#endif // UPNPLIB_MOCKING_IPHLPAPI_HPP
