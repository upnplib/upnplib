#ifndef UMOCK_IPHLPAPI_HPP
#define UMOCK_IPHLPAPI_HPP
// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-12-23

// iphlpapi.h is a Microsoft Windows library.

#include <upnplib/visibility.hpp>
#include <winsock2.h>
#include <iphlpapi.h>

namespace umock {

class UPNPLIB_API IphlpapiInterface {
  public:
    IphlpapiInterface();
    virtual ~IphlpapiInterface();
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
    IphlpapiReal();
    virtual ~IphlpapiReal() override;
    ULONG GetAdaptersAddresses(ULONG Family, ULONG Flags, PVOID Reserved,
                               PIP_ADAPTER_ADDRESSES AdapterAddresses,
                               PULONG SizePointer) override;
};

//
// This is the caller or injector class that injects the class (worker) to be
// used, real or mocked functions.
/* Example:
    IfaddrsReal ifaddrs_realObj; // already done below
    Ifaddrs(&ifaddrs_realObj);   // already done below
    { // Other scope, e.g. within a gtest
        class IfaddrsMock : public IfaddrsInterface { ...; MOCK_METHOD(...) };
        IfaddrsMock ifaddrs_mockObj;
        Ifaddrs ifaddrs_injectObj(&ifaddrs_mockObj); // obj. name doesn't matter
        EXPECT_CALL(ifaddrs_mockObj, ...);
    } // End scope, mock objects are destructed, worker restored to default.
*/ //------------------------------------------------------------------------
class UPNPLIB_API Iphlpapi {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    Iphlpapi(IphlpapiReal* a_ptr_realObj);

    // This constructor is used to inject the pointer to the mocking function.
    Iphlpapi(IphlpapiInterface* a_ptr_mockObj);

    // The destructor is ussed to restore the old pointer.
    virtual ~Iphlpapi();

    // Methods
    virtual ULONG GetAdaptersAddresses(ULONG Family, ULONG Flags,
                                       PVOID Reserved,
                                       PIP_ADAPTER_ADDRESSES AdapterAddresses,
                                       PULONG SizePointer);

  private:
    // Next variable must be static. Please note that a static member variable
    // belongs to the class, but not to the instantiated object. This is
    // important here for mocking because the pointer is also valid on all
    // objects of this class. With inline we do not need an extra definition
    // line outside the class. I also make the symbol hidden so the variable
    // cannot be accessed globaly with Iphlpapi::m_ptr_workerObj.
    UPNPLIB_LOCAL static inline IphlpapiInterface* m_ptr_workerObj;
    IphlpapiInterface* m_ptr_oldObj{};
};


UPNPLIB_EXTERN Iphlpapi iphlpapi_h;

} // namespace umock

#endif // UMOCK_IPHLPAPI_HPP
