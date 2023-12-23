// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-12-24

#include <umock/netdb.hpp>
#include <upnplib/port.hpp>

namespace umock {

NetdbInterface::NetdbInterface() = default;
NetdbInterface::~NetdbInterface() = default;

NetdbReal::NetdbReal() = default;
NetdbReal::~NetdbReal() = default;
int NetdbReal::getaddrinfo(const char* node, const char* service,
                           const struct addrinfo* hints,
                           struct addrinfo** res) {
    return ::getaddrinfo(node, service, hints, res);
}
void NetdbReal::freeaddrinfo(struct addrinfo* res) {
    return ::freeaddrinfo(res);
}

// This constructor is used to inject the pointer to the real function.
Netdb::Netdb(NetdbReal* a_ptr_realObj) {
    m_ptr_workerObj = (NetdbInterface*)a_ptr_realObj;
}

// This constructor is used to inject the pointer to the mocking function.
Netdb::Netdb(NetdbInterface* a_ptr_mockObj) {
    m_ptr_oldObj = m_ptr_workerObj;
    m_ptr_workerObj = a_ptr_mockObj;
}

// The destructor is ussed to restore the old pointer.
Netdb::~Netdb() { m_ptr_workerObj = m_ptr_oldObj; }

// Methods
int Netdb::getaddrinfo(const char* node, const char* service,
                       const struct addrinfo* hints, struct addrinfo** res) {
    return m_ptr_workerObj->getaddrinfo(node, service, hints, res);
}

void Netdb::freeaddrinfo(struct addrinfo* res) {
    return m_ptr_workerObj->freeaddrinfo(res);
}

// On program start create an object and inject pointer to the real function.
// This will exist until program end.
NetdbReal netdb_realObj;
SUPPRESS_MSVC_WARN_4273_NEXT_LINE
UPNPLIB_API Netdb netdb_h(&netdb_realObj);

} // namespace umock
