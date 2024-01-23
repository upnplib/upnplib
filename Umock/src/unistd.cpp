// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-12-26

#include <umock/unistd.hpp>
#include <upnplib/port.hpp>

namespace umock {

UnistdInterface::UnistdInterface() = default;
UnistdInterface::~UnistdInterface() = default;

UnistdReal::UnistdReal() = default;
UnistdReal::~UnistdReal() = default;
int UnistdReal::CLOSE_SOCKET_P(SOCKET fd) { return ::CLOSE_SOCKET_P(fd); }

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
int Unistd::CLOSE_SOCKET_P(SOCKET fd) {
    return m_ptr_workerObj->CLOSE_SOCKET_P(fd);
}

// On program start create an object and inject pointer to the real functions.
// This will exist until program end.
UnistdReal unistd_realObj;
SUPPRESS_MSVC_WARN_4273_NEXT_LINE
UPNPLIB_API Unistd unistd_h(&unistd_realObj);

} // namespace umock
