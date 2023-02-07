// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-02-06

#include "umock/sysinfo.inc"

namespace umock {

time_t SysinfoReal::time(time_t* tloc) { return ::time(tloc); }
#ifndef _WIN32
int SysinfoReal::uname(utsname* buf) { return ::uname(buf); }
#endif

// This constructor is used to inject the pointer to the real function.
Sysinfo::Sysinfo(SysinfoReal* a_ptr_realObj) {
    m_ptr_workerObj = (SysinfoInterface*)a_ptr_realObj;
}

// This constructor is used to inject the pointer to the mocking function.
Sysinfo::Sysinfo(SysinfoInterface* a_ptr_mockObj) {
    m_ptr_oldObj = m_ptr_workerObj;
    m_ptr_workerObj = a_ptr_mockObj;
}

// The destructor is ussed to restore the old pointer.
Sysinfo::~Sysinfo() { m_ptr_workerObj = m_ptr_oldObj; }

// Methods
time_t Sysinfo::time(time_t* tloc) { return m_ptr_workerObj->time(tloc); }
#ifndef _WIN32
int Sysinfo::uname(utsname* buf) { return m_ptr_workerObj->uname(buf); }
#endif

// On program start create an object and inject pointer to the real functions.
// This will exist until program end.
SysinfoReal sysinfo_realObj;
UPNPLIB_API Sysinfo sysinfo(&sysinfo_realObj);

} // namespace umock
