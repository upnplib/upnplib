// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-09-19

#include "upnpmock/sys_select.hpp"

#include <iostream> // DEBUG!

namespace upnplib {

int Sys_selectReal::select(int nfds, fd_set* readfds, fd_set* writefds,
                           fd_set* exceptfds, struct timeval* timeout) {
    // Call real standard library function
    return ::select(nfds, readfds, writefds, exceptfds, timeout);
}

// This constructor is used to inject the pointer to the real function.
Sys_select::Sys_select(Sys_selectReal* a_ptr_mockObj) {
    std::cout << "DEBUG! Constructor for real function pointer called.\n";
    std::cout << "DEBUG! m_ptr_workerObj = " << m_ptr_workerObj << "\n";
    m_ptr_workerObj = (Sys_selectInterface*)a_ptr_mockObj;
    std::cout << "DEBUG! m_ptr_workerObj = " << m_ptr_workerObj << "\n";
}

// This constructor is used to inject the pointer to the mocking function.
Sys_select::Sys_select(Sys_selectInterface* a_ptr_mockObj) {
    std::cout << "DEBUG! Constructor for mocked function pointer called.\n";
    std::cout << "DEBUG! m_ptr_workerObj = " << m_ptr_workerObj << "\n";
    m_ptr_oldObj = m_ptr_workerObj;
    m_ptr_workerObj = (Sys_selectInterface*)a_ptr_mockObj;
    std::cout << "DEBUG! m_ptr_workerObj = " << m_ptr_workerObj << "\n";
}

// The destructor is ussed to restore the old pointer.
Sys_select::~Sys_select() {
    std::cout << "DEBUG! Destructor is called.\n";
    std::cout << "DEBUG! m_ptr_workerObj current  = " << m_ptr_workerObj
              << "\n";
    m_ptr_workerObj = m_ptr_oldObj;
    std::cout << "DEBUG! m_ptr_workerObj restored = " << m_ptr_workerObj
              << "\n";
}

int Sys_select::select(int nfds, fd_set* readfds, fd_set* writefds,
                       fd_set* exceptfds, struct timeval* timeout) {
    std::cout << "DEBUG! Method select() is called.\n";
    std::cout << "DEBUG! m_ptr_workerObj = " << m_ptr_workerObj << "\n";
    return m_ptr_workerObj->select(nfds, readfds, writefds, exceptfds, timeout);
}

// On program start inject pointer to the real function.
Sys_selectReal sys_select_realObj;
Sys_select sys_select_h(&sys_select_realObj);

} // namespace upnplib
