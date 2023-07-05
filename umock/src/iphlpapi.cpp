// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-07-05

#include <umock/iphlpapi.inc>

namespace umock {

ULONG IphlpapiReal::GetAdaptersAddresses(ULONG Family, ULONG Flags,
                                         PVOID Reserved,
                                         PIP_ADAPTER_ADDRESSES AdapterAddresses,
                                         PULONG SizePointer) {
    return ::GetAdaptersAddresses(Family, Flags, Reserved, AdapterAddresses,
                                  SizePointer);
}

// This constructor is used to inject the pointer to the real function.
Iphlpapi::Iphlpapi(IphlpapiReal* a_ptr_realObj) {
    m_ptr_workerObj = (IphlpapiInterface*)a_ptr_realObj;
}

// This constructor is used to inject the pointer to the mocking function.
Iphlpapi::Iphlpapi(IphlpapiInterface* a_ptr_mockObj) {
    m_ptr_oldObj = m_ptr_workerObj;
    m_ptr_workerObj = a_ptr_mockObj;
}

// The destructor is ussed to restore the old pointer.
Iphlpapi::~Iphlpapi() { m_ptr_workerObj = m_ptr_oldObj; }

// Methods
ULONG Iphlpapi::GetAdaptersAddresses(ULONG Family, ULONG Flags, PVOID Reserved,
                                     PIP_ADAPTER_ADDRESSES AdapterAddresses,
                                     PULONG SizePointer) {
    return m_ptr_workerObj->GetAdaptersAddresses(Family, Flags, Reserved,
                                                 AdapterAddresses, SizePointer);
}

// On program start create an object and inject pointer to the real functions.
// This will exist until program end.
IphlpapiReal iphlpapi_realObj;
UPNPLIB_API Iphlpapi iphlpapi_h(&iphlpapi_realObj);

} // namespace umock
