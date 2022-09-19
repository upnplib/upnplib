// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-09-19

#include "mockwrap/ifaddrs.hpp"

#include <iostream> // DEBUG!

namespace mockwrap {

int IfaddrsReal::getifaddrs(struct ifaddrs** ifap) {
    return ::getifaddrs(ifap);
}

void IfaddrsReal::freeifaddrs(struct ifaddrs* ifa) {
    return ::freeifaddrs(ifa);
}

// This constructor is used to inject the pointer to the real function.
Ifaddrs::Ifaddrs(IfaddrsReal* a_ptr_mockObj) {
    std::cout << "DEBUG! Constructor for real function pointer called.\n";
    std::cout << "DEBUG! m_ptr_workerObj = " << m_ptr_workerObj << "\n";
    m_ptr_workerObj = (IfaddrsInterface*)a_ptr_mockObj;
    std::cout << "DEBUG! m_ptr_workerObj = " << m_ptr_workerObj << "\n";
}

// This constructor is used to inject the pointer to the mocking function.
Ifaddrs::Ifaddrs(IfaddrsInterface* a_ptr_mockObj) {
    std::cout << "DEBUG! Constructor for mocked function pointer called.\n";
    std::cout << "DEBUG! m_ptr_workerObj = " << m_ptr_workerObj << "\n";
    m_ptr_oldObj = m_ptr_workerObj;
    m_ptr_workerObj = (IfaddrsInterface*)a_ptr_mockObj;
    std::cout << "DEBUG! m_ptr_workerObj = " << m_ptr_workerObj << "\n";
}

// The destructor is ussed to restore the old pointer.
Ifaddrs::~Ifaddrs() {
    std::cout << "DEBUG! Destructor is called.\n";
    std::cout << "DEBUG! m_ptr_workerObj current  = " << m_ptr_workerObj
              << "\n";
    m_ptr_workerObj = m_ptr_oldObj;
    std::cout << "DEBUG! m_ptr_workerObj restored = " << m_ptr_workerObj
              << "\n";
}

int Ifaddrs::getifaddrs(struct ifaddrs** ifap) {
    std::cout << "DEBUG! Method getifaddrs() is called.\n";
    std::cout << "DEBUG! m_ptr_workerObj = " << m_ptr_workerObj << "\n";
    return m_ptr_workerObj->getifaddrs(ifap);
}

void Ifaddrs::freeifaddrs(struct ifaddrs* ifa) {
    std::cout << "DEBUG! Method freeifaddrs() is called.\n";
    std::cout << "DEBUG! m_ptr_workerObj = " << m_ptr_workerObj << "\n";
    return m_ptr_workerObj->freeifaddrs(ifa);
}

// On program start create an object and inject pointer to the real function.
// This will exist until program end.
IfaddrsReal ifaddrs_realObj;
Ifaddrs ifaddrs_h(&ifaddrs_realObj);

} // namespace mockwrap
