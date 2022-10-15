// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-10-16

#include "upnplib/mocking/unistd.inc"

namespace upnplib::mocking {

int UnistdReal::UPNPLIB_CLOSE_SOCKET(UPNPLIB_SOCKET_TYPE fd) {
    return ::UPNPLIB_CLOSE_SOCKET(fd);
}

// This constructor is used to inject the pointer to the real function.
Unistd::Unistd(UnistdReal* a_ptr_realObj) {
    m_ptr_workerObj = (UnistdInterface*)a_ptr_realObj;
}

// This constructor is used to inject the pointer to the mocking function.
Unistd::Unistd(UnistdInterface* a_ptr_mockObj) {
    m_ptr_oldObj = m_ptr_workerObj;
    m_ptr_workerObj = a_ptr_mockObj;
}

// The destructor is ussed to restore the old pointer.
Unistd::~Unistd() { m_ptr_workerObj = m_ptr_oldObj; }

// Methods
int Unistd::UPNPLIB_CLOSE_SOCKET(UPNPLIB_SOCKET_TYPE fd) {
    return m_ptr_workerObj->UPNPLIB_CLOSE_SOCKET(fd);
}

// On program start create an object and inject pointer to the real functions.
// This will exist until program end.
UnistdReal unistd_realObj;
UPNPLIB_API Unistd unistd_h(&unistd_realObj);

} // namespace upnplib::mocking
