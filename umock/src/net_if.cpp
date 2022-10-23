// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-10-23

#include "umock/net_if.inc"

namespace umock {

unsigned int Net_ifReal::if_nametoindex(const char* ifname) {
    return ::if_nametoindex(ifname);
}

// This constructor is used to inject the pointer to the real function.
Net_if::Net_if(Net_ifReal* a_ptr_realObj) {
    m_ptr_workerObj = (Net_ifInterface*)a_ptr_realObj;
}

// This constructor is used to inject the pointer to the mocking function.
Net_if::Net_if(Net_ifInterface* a_ptr_mockObj) {
    m_ptr_oldObj = m_ptr_workerObj;
    m_ptr_workerObj = a_ptr_mockObj;
}

// The destructor is ussed to restore the old pointer.
Net_if::~Net_if() { m_ptr_workerObj = m_ptr_oldObj; }

// Methods
unsigned int Net_if::if_nametoindex(const char* ifname) {
    return m_ptr_workerObj->if_nametoindex(ifname);
}

// On program start create an object and inject pointer to the real functions.
// This will exist until program end.
Net_ifReal net_if_realObj;
UPNPLIB_API Net_if net_if_h(&net_if_realObj);

} // namespace umock
