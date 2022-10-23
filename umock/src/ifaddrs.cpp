// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-10-23

#include "umock/ifaddrs.inc"

namespace umock {

int IfaddrsReal::getifaddrs(struct ifaddrs** ifap) {
    return ::getifaddrs(ifap);
}

void IfaddrsReal::freeifaddrs(struct ifaddrs* ifa) {
    return ::freeifaddrs(ifa);
}

// This constructor is used to inject the pointer to the real function.
Ifaddrs::Ifaddrs(IfaddrsReal* a_ptr_mockObj) {
    m_ptr_workerObj = (IfaddrsInterface*)a_ptr_mockObj;
}

// This constructor is used to inject the pointer to the mocking function.
Ifaddrs::Ifaddrs(IfaddrsInterface* a_ptr_mockObj) {
    m_ptr_oldObj = m_ptr_workerObj;
    m_ptr_workerObj = (IfaddrsInterface*)a_ptr_mockObj;
}

// The destructor is ussed to restore the old pointer.
Ifaddrs::~Ifaddrs() { m_ptr_workerObj = m_ptr_oldObj; }

// Methods
int Ifaddrs::getifaddrs(struct ifaddrs** ifap) {
    return m_ptr_workerObj->getifaddrs(ifap);
}

void Ifaddrs::freeifaddrs(struct ifaddrs* ifa) {
    return m_ptr_workerObj->freeifaddrs(ifa);
}

// On program start create an object and inject pointer to the real function.
// This will exist until program end.
IfaddrsReal ifaddrs_realObj;
UPNPLIB_API Ifaddrs ifaddrs_h(&ifaddrs_realObj);

} // namespace umock
